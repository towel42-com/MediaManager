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
#include "TitleInfo.h"
#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/AutoWaitCursor.h"

#include <QDebug>
#include <QUrl>
#include <QInputDialog>
#include <QTextStream>
#include <QCollator>
#include <QDir>
#include <QLocale>
#include <QFileIconProvider>
#include <QTimer>
#include <QTreeView>

#include <set>
#include <list>

CDirModel::CDirModel( QObject *parent /*= 0*/ ) :
    QStandardItemModel( parent )
{
    fIconProvider = new QFileIconProvider();
    fTimer = new QTimer( this );
    fTimer->setInterval( 50 );
    fTimer->setSingleShot( true );
    connect( fTimer, &QTimer::timeout, this, &CDirModel::slotLoadRootDirectory );

    fPatternTimer = new QTimer( this );
    fPatternTimer->setInterval( 50 );
    fPatternTimer->setSingleShot( true );
    connect( fPatternTimer, &QTimer::timeout, this, &CDirModel::slotPatternChanged );

    fExcludedDirNames = { "subs", "#recycle", "#recycler", "extras" };
}

CDirModel::~CDirModel()
{
    delete fIconProvider;
}

void CDirModel::setRootPath( const QString & rootPath, QTreeView * view )
{
    fRootPath = rootPath;
    reloadModel( view );
}

void CDirModel::setNameFilters( const QStringList &filters, QTreeView * view )
{
    fNameFilter = filters;
    reloadModel( view );
}

void CDirModel::reloadModel( QTreeView *view )
{
    fTreeView = view;
    fTimer->stop();
    fTimer->start();
    fPatternTimer->stop(); // if its runinng when this timer stops its realoaded anyway
}

void CDirModel::slotLoadRootDirectory()
{
    clear();
    setHorizontalHeaderLabels( QStringList() << "Name" << "Size" << "Type" << "Date Modified" << "Transformed Name" );
    QFileInfo rootFI( fRootPath );
    loadFileInfo( rootFI.absoluteFilePath(), nullptr );
    if ( fTreeView )
    {
        fTreeView->resizeColumnToContents( EColumns::eFSName );
        fTreeView->resizeColumnToContents( EColumns::eTransformName );
    }
}

void CDirModel::loadFileInfo( const QFileInfo & fileInfo, QStandardItem * parent )
{
    if ( !fileInfo.exists() || !fileInfo.isReadable() )
        return;

    //qDebug() << "Loading: " << fileInfo.absoluteFilePath();
    auto row = getItemRow( fileInfo );
    if ( parent )
        parent->appendRow( row );
    else
        appendRow( row );

    if ( fileInfo.isDir() )
    {
        QDir dir( fileInfo.absoluteFilePath() );
        dir.setFilter( QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable );
        dir.setSorting( QDir::Name | QDir::DirsFirst | QDir::IgnoreCase );
        dir.setNameFilters( fNameFilter );
        auto fileInfos = dir.entryInfoList();
        for ( auto &&ii : fileInfos )
        {
            if ( ii.isDir() && excludedDirName( ii ) )
                continue;
            loadFileInfo( ii, row.front() );
        }
    }
    if ( fTreeView )
    {
        fTreeView->setExpanded( row.front()->index(), true );
    }
}


bool CDirModel::excludedDirName( const QFileInfo &ii ) const
{
    auto fn = ii.fileName().toLower();
    return fExcludedDirNames.find( fn ) != fExcludedDirNames.end();
}

QList< QStandardItem * > CDirModel::getItemRow( const QFileInfo & fileInfo ) const
{
    QLocale locale;

    QList< QStandardItem * > retVal;
    auto nameItem = new QStandardItem( fileInfo.fileName() );
    nameItem->setIcon( fIconProvider->icon( fileInfo ) );
    nameItem->setData( fileInfo.absoluteFilePath(), ECustomRoles::eFullPathRole );
    retVal.push_back( nameItem );
    retVal.push_back( new QStandardItem( fileInfo.isFile() ? locale.toString( fileInfo.size() ) : QString() ) );
    retVal.back()->setTextAlignment( Qt::AlignRight );
    retVal.push_back( new QStandardItem( fIconProvider->type( fileInfo ) ) );
    retVal.push_back( new QStandardItem( fileInfo.lastModified().toString( "MM/dd/yyyy hh:mm:ss.zzz") ) );
    auto transformInfo = transformItem( fileInfo );
    auto transformedItem = new QStandardItem( transformInfo.second );
    retVal.push_back( transformedItem );

    updatePattern( nameItem, transformedItem );
    return retVal;
}

QString CDirModel::patternToRegExp( const QString & captureName, const QString & inPattern, const QString &value, bool removeOptional ) const
{
    if ( captureName.isEmpty() || inPattern.isEmpty() )
        return inPattern;

    // see if the capture name exists in the return pattern
    auto capRegEx = QString( "\\\\\\((?<optname>.*)\\\\\\)(\\\\)?\\:\\<%1\\>" ).arg( captureName );
    auto regExp = QRegularExpression( capRegEx );
    auto retVal = inPattern;
    retVal = retVal.replace( regExp, removeOptional ? "\\1" : "(\\1)?" );

    capRegEx = QString( "\\<%1\\>" ).arg( captureName );
    regExp = QRegularExpression( capRegEx );
    retVal = retVal.replace( regExp, value );

    return retVal;
}

QString CDirModel::patternToRegExp( const QString & pattern, bool removeOptional ) const
{
    QString retVal = pattern;
    retVal.replace( "(", "\\(" );
    retVal.replace( ")", "\\)" );
    retVal.replace( ":", "\\:" );

    retVal = patternToRegExp( "title", retVal, ".*", removeOptional );
    retVal = patternToRegExp( "year", retVal, "\\d{2,4}", removeOptional );
    retVal = patternToRegExp( "tmdbid", retVal, "\\d+", removeOptional );
    retVal = patternToRegExp( "season", retVal, "\\d+", removeOptional );
    retVal = patternToRegExp( "episode", retVal, "\\d+", removeOptional );
    retVal = patternToRegExp( "episode_title", retVal, ".*", removeOptional );
    retVal = patternToRegExp( "extra_info", retVal, ".*", removeOptional );
    return retVal;
}

bool CDirModel::isValidName( const QFileInfo & fi ) const
{
    if ( isValidName( fi.fileName(), fi.isDir() ) )
        return true;
    if ( fi.isDir() )
    {
        if ( fTreatAsMovie )
        {
            QDir dir( fi.absoluteFilePath() );
            auto children = dir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
            return !children.empty();
        }
        else
            return true;
    }
    return false;
}

bool CDirModel::isValidName( const QString &name, bool isDir ) const
{
    if ( name.isEmpty() )
        return false;
    QStringList patterns = { fInPattern };
    if ( isDir )
    {
        patterns
            << patternToRegExp( fOutDirPattern, false )
            << "(.*)\\s\\((\\d{2,4}\\))\\s(-\\s(.*)\\s)?\\[(tmdbid=\\d+)|(imdbid=tt.*)\\]"
            ;
    }
    else
    {
        patterns << patternToRegExp( fOutFilePattern, true )
            ;
    }
    for ( auto &&ii : patterns )
    {
        QRegularExpression regExp( ii );
        if ( !ii.isEmpty() && regExp.match( name ).hasMatch() )
            return true;
    }
    return false;
}

QFileInfo CDirModel::fileInfo( const QStandardItem * item ) const
{
    if ( !item )
        return QFileInfo();
    return QFileInfo( item->data( ECustomRoles::eFullPathRole ).toString() );
}

QStandardItem *CDirModel::getItemFromindex( QModelIndex idx ) const
{
    if ( idx.column() != EColumns::eFSName )
    {
        idx = idx.model()->index( idx.row(), EColumns::eFSName, idx.parent() );
    }
    return itemFromIndex( idx );
}

bool CDirModel::isDir( const QStandardItem *item ) const
{
    return fileInfo( item ).isDir();
}

QString CDirModel::filePath( const QStandardItem *item ) const
{
    return fileInfo( item ).absoluteFilePath();
}

QString CDirModel::filePath( const QModelIndex &idx ) const
{
    auto item = getItemFromindex( idx );
    return filePath( item );
}

QFileInfo CDirModel::fileInfo( const QModelIndex & idx ) const
{
    auto item = getItemFromindex( idx );
    return fileInfo( item );
}

bool CDirModel::isDir( const QModelIndex & idx ) const
{
    auto item = getItemFromindex( idx );
    return isDir( item );
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

void CDirModel::cleanFileName( QString & inFile ) const
{
    QString text = "\\s*\\:\\s*";
    inFile.replace( QRegularExpression( text ), " - " );
    text = "[\\<\\>\\\"\\/\\\\\\|\\?\\*]";
    inFile.replace( QRegularExpression( text ), "" );
}

std::pair< bool, QString > CDirModel::transformItem( const QFileInfo &fileInfo ) const
{
    auto filePath = fileInfo.absoluteFilePath();

    if ( !fInPatternRegExp.isValid() )
        return std::make_pair( false, tr( "<INVALID INPUT REGEX>" ) );

    auto pos = fileInfo.isDir() ? fDirMapping.find( filePath ) : fFileMapping.find( filePath );
    auto retVal = std::make_pair( false, QString() );

    if ( pos == ( fileInfo.isDir() ? fDirMapping.end() : fFileMapping.end() ) )
    {
        QString fn = fileInfo.fileName();
        QString ext = QString();
        if ( !fileInfo.isDir() )
        {
            fn = fileInfo.fileName();
            ext = fileInfo.suffix();
        }

        auto pos = fTitleInfoMapping.find( filePath );
        auto match = fInPatternRegExp.match( fn );
        if ( pos == fTitleInfoMapping.end() )
        {
            if ( isValidName( fileInfo ) )
                retVal.second = QString();
            else if ( !match.hasMatch() )
            {
                retVal.second = "<NOMATCH>";
                qDebug() << "Poorly formed filename" << filePath;
            }
        }
        else 
        {
            auto title = NStringUtils::transformTitle( match.captured( "title" ) );
            auto year = match.captured( "year" ).trimmed();
            auto tmdbid = match.captured( "tmdbid" ).trimmed();
            auto season = match.captured( "season" ).trimmed();
            auto episode = match.captured( "episode" ).trimmed();
            auto episodeTitle = NStringUtils::transformTitle( match.captured( "episode_title" ) );

            QString extraInfo;
            if ( pos != fTitleInfoMapping.end() )
            {
                title = (*pos).second->getTitle();
                year = ( *pos ).second->getYear();
                tmdbid = ( *pos ).second->fTMDBID;
                season = ( *pos ).second->fSeason;
                episode = ( *pos ).second->fEpisode;
                extraInfo = ( *pos ).second->fExtraInfo;
                episodeTitle = ( *pos ).second->fEpisodeTitle;
            }

            retVal.second = fileInfo.isDir() ? fOutDirPattern : fOutFilePattern;
            retVal.second = replaceCapture( "title", retVal.second, title );
            retVal.second = replaceCapture( "year", retVal.second, year );
            retVal.second = replaceCapture( "tmdbid", retVal.second, tmdbid );
            retVal.second = replaceCapture( "season", retVal.second, QString( "%1" ).arg( season, 2, QChar( '0' ) ) );
            retVal.second = replaceCapture( "episode", retVal.second, QString( "%1" ).arg( episode, 2, QChar( '0' ) ) );
            retVal.second = replaceCapture( "episode_title", retVal.second, episodeTitle );
            retVal.second = replaceCapture( "extra_info", retVal.second, extraInfo );

            if ( !fileInfo.isDir() )
                retVal.second += "." + ext;
            cleanFileName( retVal.second );
            retVal.first = true;
        }

        if ( fileInfo.isDir() )
            fDirMapping[filePath] = retVal;
        else
            fFileMapping[filePath] = retVal;
    }
    else
        retVal = ( *pos ).second;

    return retVal;
}

void CDirModel::saveM3U( QWidget *parent ) const
{
    auto rootIndex = invisibleRootItem();

    auto baseName = QInputDialog::getText( parent, tr( "Series Name" ), tr( "Name:" ) );
    if ( baseName.isEmpty() )
        return;

    saveM3U( rootIndex, baseName );
}

QString CDirModel::saveM3U( const QStandardItem  * parent, const QString &baseName ) const
{
    if ( isDir( parent ) )
    {
        auto parentDir = fileInfo( parent ).absoluteDir();
        std::list< QFileInfo > myMedia;
        auto numRows = parent->rowCount();
        for ( int ii = 0; ii < numRows; ++ii )
        {
            auto child = parent->child( ii );
            if ( !child )
                continue;

            if ( isDir( child ) )
            {
                auto childPlayList = saveM3U( child, baseName );
                if ( childPlayList.isEmpty() ) // no media files found
                    continue;
                myMedia.push_back( QFileInfo( childPlayList ) );;
            }
            else
            {
                auto suffix = fileInfo( child ).suffix();
                std::set< QString > media = { "mkv", "mp4", "avi" };
                if ( media.find( suffix ) == media.end() )
                    continue;
                myMedia.push_back( fileInfo( child ) );
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
            auto fi = fileInfo( parent );
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

void CDirModel::setTitleInfo( const QModelIndex &idx, std::shared_ptr< STitleInfo > titleInfo )
{
    if ( !idx.isValid() )
        return;

    auto fi = fileInfo( idx );
    fTitleInfoMapping[fi.absoluteFilePath()] = titleInfo;
    if ( isDir( idx ) )
        fDirMapping.erase( fi.absoluteFilePath() );
    else 
        fFileMapping.erase( fi.absoluteFilePath() );
    updatePattern( getItemFromindex( idx ) );
}

std::shared_ptr< STitleInfo > CDirModel::getTitleInfo( const QModelIndex &idx ) const
{
    if ( !idx.isValid() )
        return {};

    auto fi = fileInfo( idx );
    auto pos = fTitleInfoMapping.find( fi.absoluteFilePath() );
    if ( pos == fTitleInfoMapping.end() )
        return {};
    return ( *pos ).second;
}

std::pair< bool, QStringList > CDirModel::transform( const QStandardItem * parent, bool displayOnly ) const
{
    if ( !parent)
        return std::make_pair( false, QStringList() );

    QStringList retVal;
    auto numRows = parent->rowCount();
    for ( int ii = 0; ii < numRows; ++ii )
    {
        auto child = parent->child( ii );
        if ( !child )
            continue;

        auto sub = transform( child, displayOnly );
        //if ( sub.first )
            retVal << sub.second;
        //else
        //    return sub;
    }

    auto idx = indexFromItem( parent );
    auto transformItem = itemFromIndex( index( idx.row(), CDirModel::EColumns::eTransformName, idx.parent() ) );
    auto transformName = transformItem ? transformItem->text() : QString();
    
    if ( !transformName.isEmpty() && ( transformName != "<NOMATCH>" ) )
    {
    //if ( !parent->text( EColumns::eTransformName ).isEmpty() && parent->text( EColumns::eTransformName ) != "<NOMATCH>" )
    //{
    //auto newFile = transformItem( fileInfo( parent ) );
    //if ( newFile.first )
    //{
        retVal << QString( "'%1' => '%2'" ).arg( parent->text() ).arg( transformName );
        if ( !displayOnly )
        {
            auto oldName = this->filePath( parent );
            QFileInfo fi( oldName );
            bool aOK = false;
            if ( !fi.exists() )
            {
                retVal[retVal.length() - 1] = QString( "ERROR: '%1' - No Longer Exists" ).arg( oldName );;
            }
            else 
            {
                auto dir = fi.absoluteDir();
                auto newName = dir.absoluteFilePath( transformName );
                if ( oldName != newName )
                {
                    aOK = QFile::rename( oldName, newName );
                    if ( !aOK )
                    {
                        retVal[retVal.length() - 1] = "ERROR: " + retVal[retVal.length() - 1] + ": FAILED TO RENAME";
                    }
                }
            }
            return std::make_pair( aOK, retVal );
        }
    }

    return std::make_pair( true, retVal );
}

std::pair< bool, QStringList > CDirModel::transform( bool displayOnly ) const
{
    CAutoWaitCursor awc;
    return transform( invisibleRootItem(), displayOnly );
}

void CDirModel::patternChanged()
{
    fPatternTimer->stop();
    fPatternTimer->start();
}

void CDirModel::slotPatternChanged()
{
    fFileMapping.clear();
    fDirMapping.clear();
    patternChanged( invisibleRootItem() );
}

void CDirModel::patternChanged( const QStandardItem * item )
{
    if ( !item )
        return;

    updatePattern( item );

    auto numRows = item->rowCount();
    for ( int ii = 0; ii < numRows; ++ii )
    {
        auto child = item->child( ii );
        if ( child )
        {
            //emit dataChanged( child, child );
            patternChanged( child );
        }
    }
}

void CDirModel::updatePattern( const QStandardItem *item )const
{
    if ( item != invisibleRootItem() )
    {
        auto idx = item->index();
        auto transformedIndex = idx.model()->index( idx.row(), EColumns::eTransformName, idx.parent() );
        auto transformedItem = this->itemFromIndex( transformedIndex );
        updatePattern( item, transformedItem );
    }
}

void CDirModel::updatePattern( const QStandardItem * item, QStandardItem * transformedItem ) const
{
    if ( !item || !transformedItem )
        return;

    auto path = item->data( ECustomRoles::eFullPathRole ).toString();
    auto fileInfo = QFileInfo( path );
    auto transformInfo = transformItem( fileInfo );
    if ( transformInfo.second == "<NOMATCH>" || !isValidName( transformInfo.second, fileInfo.isDir() ) && !isValidName( fileInfo ) )
        transformedItem->setBackground( Qt::red );
    else
    {
        if ( transformedItem->text() != transformInfo.second )
            transformedItem->setText( transformInfo.second );
        transformedItem->setBackground( item->background() );
    }
}

