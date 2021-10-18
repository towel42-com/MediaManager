// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "DirModel.h"
#include "SABUtils/StringUtils.h"
#include <QDebug>
#include <QUrl>
#include <QInputDialog>
#include <QTextStream>
#include <QCollator>
#include <set>
#include <list>

CDirModel::CDirModel( QObject *parent /*= 0*/ ) :
    QFileSystemModel( parent )
{

}

CDirModel::~CDirModel()
{

}

bool CDirModel::isValidDirName( const QString &name ) const
{
    QRegularExpression regExp( "(.*)\\s\\(\\d{2,4}\\)\\s\\[(tmdbid=\\d+)|(imdbid=tt.*)\\]" );
    return regExp.match( name ).hasMatch();
}

QVariant CDirModel::data( const QModelIndex &index, int role /*= Qt::DisplayRole */ ) const
{
    if ( !index.isValid() )
        return QVariant();
    if ( ( role == Qt::DisplayRole ) && ( index.column() == 4 ) )
        return transformItem( index ).second;
    else if ( role == Qt::BackgroundRole && ( index.column() == 4 ) && isDir( index ) )
    {
        if ( !isValidDirName( transformItem( index ).second ) )
            return QColor( Qt::red );
    }
    return QFileSystemModel::data( index, role );
}

QVariant CDirModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */ ) const
{
    if ( ( section == 4 ) && ( orientation == Qt::Orientation::Horizontal ) && ( role == Qt::DisplayRole ) )
        return tr( "New File Name" );
    return QFileSystemModel::headerData( section, orientation, role );
}

Qt::ItemFlags CDirModel::flags( const QModelIndex &idx ) const
{
    return QFileSystemModel::flags( idx );
}

int CDirModel::columnCount( const QModelIndex &parent ) const
{
    auto retVal = QFileSystemModel::columnCount( parent );
    return retVal ? retVal + 1 : 0;
}

int CDirModel::rowCount( const QModelIndex &parent ) const
{
    if ( parent.data() == "#recycle" )
    {
        return 0;
    }
    return QFileSystemModel::rowCount( parent );
}

void CDirModel::slotInputPatternChanged( const QString &inPattern )
{
    fInPattern = inPattern;
    fInPatternRegExp.setPattern( fInPattern );
    //Q_ASSERT( fInPatternRegExp.isValid() );
    patternChanged();
}

void CDirModel::slotOutputFilePatternChanged( const QString &outPattern )
{
    fOutFilePattern = outPattern;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

void CDirModel::slotOutputDirPatternChanged( const QString &outPattern )
{
    fOutDirPattern = outPattern;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

void CDirModel::slotTreatAsMovieChanged( bool treatAsMovie )
{
    fTreatAsMovie = treatAsMovie;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

// do not include <> in the capture name
QString CDirModel::replaceCapture( const QString &captureName, const QString &returnPattern, const QString &value ) const
{
    if ( captureName.isEmpty() )
        return returnPattern;

    // see if the capture name exists in the return pattern
    auto capRegEx = QString( "\\<%1\\>" ).arg( captureName );
    auto regExp = QRegularExpression( capRegEx );

    int start = -1;
    int replLength = -1;

    auto match = regExp.match( returnPattern );
    if ( !match.hasMatch() )
        return returnPattern;
    else
    {
        start = match.capturedStart( 0 );
        replLength = match.capturedLength( 0 );
    }

    // its in there..now lets see if its optional
    auto optRegExStr = QString( "\\((?<replText>[^()]+)\\)\\:%1" ).arg( capRegEx );
    regExp = QRegularExpression( optRegExStr );
    match = regExp.match( returnPattern );
    bool optional = match.hasMatch();
    QString replText = value;
    if ( optional )
    {
        start = match.capturedStart( 0 );
        replLength = match.capturedLength( 0 );

        replText = match.captured( "replText" );
        if ( value.isEmpty() )
            replText.clear();
        else
        {
            replText = replaceCapture( captureName, replText, value );;
        }
    }
    auto retVal = returnPattern;
    retVal.replace( start, replLength, replText );
    return retVal;
}

std::pair< bool, QString > CDirModel::transformItem( const QModelIndex &idx ) const
{
    auto fnIdx = this->index( idx.row(), 0, idx.parent() );
    auto fileName = fnIdx.data().toString();

    if ( !fInPatternRegExp.isValid() )
        return std::make_pair( false, tr( "<INVALID INPUT REGEX>" ) );

    auto pos = isDir( idx ) ? fDirMapping.find( fileName ) : fFileMapping.find( fileName );
    auto retVal = std::make_pair( false, QString() );

    if ( pos == ( isDir( idx ) ? fDirMapping.end() : fFileMapping.end() ) )
    {
        QString fn = fileName;
        QString ext = QString();
        if ( isDir( idx ) )
            fn = fileName;
        else
        {
            auto lastDotPos = fileName.lastIndexOf( QLatin1Char( '.' ) );
            if ( lastDotPos != -1 )
            {
                fn = fileName.left( lastDotPos );
                ext = fileName.right( fileName.length() - lastDotPos - 1 );
            }
        }

        auto match = fInPatternRegExp.match( fn );
        if ( !match.hasMatch() )
            retVal.second = "<NOMATCH>";
        else
        {
            auto program = NStringUtils::transformTitle( match.captured( "program" ) );
            auto year = match.captured( "year" ).trimmed();
            auto tmdbid = match.captured( "tmdbid" ).trimmed();
            auto season = match.captured( "season" ).trimmed();
            auto episode = match.captured( "episode" ).trimmed();
            auto title = NStringUtils::transformTitle( match.captured( "title" ) );

            retVal.second = isDir( idx ) ? fOutDirPattern : fOutFilePattern;
            retVal.second = replaceCapture( "program", retVal.second, program );
            retVal.second = replaceCapture( "year", retVal.second, year );
            retVal.second = replaceCapture( "tmdbid", retVal.second, tmdbid );
            retVal.second = replaceCapture( "season", retVal.second, season );
            retVal.second = replaceCapture( "episode", retVal.second, episode );
            retVal.second = replaceCapture( "title", retVal.second, title );

            if ( !isDir( idx ) )
                retVal.second += "." + ext;
            retVal.first = true;
        }

        if ( isDir( idx ) )
            fDirMapping[fileName] = retVal;
        else
            fFileMapping[fileName] = retVal;
    }
    else
        retVal = ( *pos ).second;

    return retVal;
}

void CDirModel::saveM3U( QWidget *parent ) const
{
    auto rootIndex = this->rootIndex();

    auto baseName = QInputDialog::getText( parent, tr( "Series Name" ), tr( "Name:" ) );
    if ( baseName.isEmpty() )
        return;

    saveM3U( rootIndex, baseName );
}

QString CDirModel::saveM3U( const QModelIndex &parentIndex, const QString &baseName ) const
{
    if ( isDir( parentIndex ) )
    {
        auto parentDir = fileInfo( parentIndex ).absoluteDir();
        std::list< QFileInfo > myMedia;
        auto numRows = rowCount( parentIndex );
        for ( int ii = 0; ii < numRows; ++ii )
        {
            auto childIndex = index( ii, 0, parentIndex );
            if ( !childIndex.isValid() )
                continue;

            if ( isDir( childIndex ) )
            {
                auto childPlayList = saveM3U( childIndex, baseName );
                if ( childPlayList.isEmpty() ) // no media files found
                    continue;
                myMedia.push_back( QFileInfo( childPlayList ) );;
            }
            else
            {
                auto suffix = fileInfo( childIndex ).suffix();
                std::set< QString > media = { "mkv", "mp4", "avi" };
                if ( media.find( suffix ) == media.end() )
                    continue;
                myMedia.push_back( fileInfo( childIndex ) );
            }
        }
        if ( !myMedia.empty() )
        {
            qDebug() << myMedia;
            myMedia.sort( []( const QFileInfo &lhs, const QFileInfo &rhs )
                          {
                              qDebug() << lhs << "vs" << rhs;
                              if ( lhs.isDir() == rhs.isDir() )
                              {
                                  QCollator coll;
                                  coll.setNumericMode( true );
                                  coll.setIgnorePunctuation( true );
                                  coll.setCaseSensitivity( Qt::CaseSensitivity::CaseInsensitive );
                                  if ( lhs.absoluteDir() == rhs.absoluteDir() )
                                      return coll.compare( lhs.absoluteFilePath(), rhs.absoluteFilePath() ) < 0;
                                  else
                                      return coll.compare( lhs.absolutePath(), rhs.absolutePath() ) < 0;

                              }
                              if ( lhs.isDir() )
                                  return false;
                              return true;
                          } );
            qDebug() << myMedia;
            auto fi = fileInfo( parentIndex );
            auto fn = QString( "%1 - %2.m3u" ).arg( baseName ).arg( fi.baseName() );
            if ( baseName == fi.baseName() )
                fn = QString( "%1.m3u" ).arg( baseName );
            auto m3uPath = QDir( fi.absoluteFilePath() ).absoluteFilePath( fn );
            QFile file( m3uPath );
            if ( !file.open( QFile::WriteOnly | QFile::Text ) )
                return QString();

            QTextStream ts( &file );

            for ( auto &&ii : myMedia )
            {
                auto myPath = ii.absoluteFilePath();
                auto myRelPath = QDir( fi.absoluteFilePath() ).relativeFilePath( myPath );
                ts << myRelPath << "\n";
            }
            return m3uPath;
        }
    }

    return QString();
}

std::pair< bool, QStringList > CDirModel::transform( const QModelIndex &idx, bool displayOnly ) const
{
    if ( !idx.isValid() )
        return std::make_pair( false, QStringList() );

    QStringList retVal;
    auto newFile = transformItem( idx );
    if ( !isDir( idx ) && newFile.first )
    {
        retVal << QString( "'%1' => '%2'" ).arg( idx.data().toString() ).arg( newFile.second );
        if ( !displayOnly )
        {
            auto oldName = this->filePath( idx );
            QFileInfo fi( oldName );
            bool aOK = false;
            if ( !fi.exists() )
            {
                retVal[retVal.length() - 1] = QString( "ERROR: '%1' - No Longer Exists" ).arg( oldName );;
            }
            else
            {
                auto dir = fi.absoluteDir();
                auto newName = dir.absoluteFilePath( newFile.second );
                aOK = QFile::rename( oldName, newName );
                if ( !aOK )
                {
                    retVal[retVal.length() - 1] = "ERROR: " + retVal[retVal.length() - 1] + ": FAILED TO RENAME";
                }
            }
            return std::make_pair( aOK, retVal );
        }
    }

    auto numRows = rowCount( idx );
    for ( int ii = 0; ii < numRows; ++ii )
    {
        auto childIndex = index( ii, 0, idx );
        if ( childIndex.isValid() )
        {
            auto sub = transform( childIndex, displayOnly );
            if ( sub.first )
                retVal << sub.second;
            else
                return sub;
        }
    }
    return std::make_pair( true, retVal );
}

QModelIndex CDirModel::rootIndex() const
{
    auto rootPath = this->rootPath();
    auto root = this->index( rootPath );
    return root;
}

void CDirModel::patternChanged()
{
    fFileMapping.clear();
    patternChanged( rootIndex() );
}

void CDirModel::patternChanged( const QModelIndex &idx )
{
    if ( !idx.isValid() )
        return;

    auto numRows = rowCount( idx );
    for ( int ii = 0; ii < numRows; ++ii )
    {
        auto childIndex = index( ii, 4, idx );
        if ( childIndex.isValid() )
        {
            emit dataChanged( childIndex, childIndex );
            patternChanged( childIndex );
        }
    }
}

CDirFilterModel::CDirFilterModel( QObject *parent /*= nullptr */ ) :
    QSortFilterProxyModel( parent )
{

}

bool CDirFilterModel::filterAcceptsRow( int sourceRow, const QModelIndex &parent ) const
{
    auto index = sourceModel()->index( sourceRow, 0, parent );
    auto data = index.data().toString();
    return data != "#recycle";
}
