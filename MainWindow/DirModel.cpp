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
CDirModel::CDirModel( QObject * parent /*= 0*/ ) : 
    QFileSystemModel( parent )
{

}

CDirModel::~CDirModel()
{

}

QVariant CDirModel::data( const QModelIndex& index, int role /*= Qt::DisplayRole */ ) const
{
    if ( !index.isValid() )
        return QVariant();
    if ( ( role == Qt::DisplayRole ) && ( index.column() == 4 ) )
    {
        if ( isDir( index ) )
            return QString();
        return transformFile( index ).second;
    }
    return QFileSystemModel::data( index, role );
}

QVariant CDirModel::headerData( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */ ) const
{
    if ( ( section == 4 ) && ( orientation == Qt::Orientation::Horizontal ) && ( role == Qt::DisplayRole ) )
        return tr( "New File Name" );
    return QFileSystemModel::headerData( section, orientation, role );
}

Qt::ItemFlags CDirModel::flags( const QModelIndex& idx ) const
{
    return QFileSystemModel::flags( idx );}

int CDirModel::columnCount( const QModelIndex& parent ) const
{
    auto retVal = QFileSystemModel::columnCount( parent );
    return retVal ? retVal + 1 : 0;
}

void CDirModel::slotInputPatternChanged( const QString& inPattern )
{
    fInPattern = inPattern;

    //QRegularExpression regEx( "([\\^\\$\\.\\*\\+\\?\\|\\(\\)\\[\\]\\{\\}\\\\])" );
    //fInPattern.replace( regEx, "\\\\1" );

    fInPattern.replace( "%P", "(?<program>.+)" );
    fInPattern.replace( "%S", "(?<season>\\d+)" );
    fInPattern.replace( "%E", "(?<episode>\\d+)" );
    fInPattern.replace( "%T", "(?<title>.+)" );

    fInPatternRegExp.setPattern( fInPattern );
    //Q_ASSERT( fInPatternRegExp.isValid() );
    patternChanged();
}

void CDirModel::slotOutputPatternChanged( const QString& outPattern )
{
    fOutPattern = outPattern;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

std::pair< bool, QString > CDirModel::transformFile( const QModelIndex& idx ) const
{
    auto fnIdx = this->index( idx.row(), 0, idx.parent() );
    auto fileName = fnIdx.data().toString();

    if ( !fInPatternRegExp.isValid() )
        return std::make_pair( false, tr( "<INVALID INPUT REGEX>" ) );

    auto pos = fFileMapping.find( fileName );
    std::pair< bool, QString > retVal = std::make_pair( false, QString() );
    if ( pos == fFileMapping.end() )
    {
        auto lastDotPos = fileName.lastIndexOf( QLatin1Char( '.' ) );
        QString fn = fileName;
        QString ext = QString();
        if ( lastDotPos != -1 )
        {
            fn = fileName.left( lastDotPos );
            ext = fileName.right( fileName.length() - lastDotPos - 1 );
        }

        auto match = fInPatternRegExp.match( fn );
        if ( !match.hasMatch() )
            retVal.second = "<NOMATCH>";
        else
        {
            auto program = match.captured( "program" ).replace( '.', ' ' ).trimmed();
            auto season = match.captured( "season" ).trimmed();
            auto episode = match.captured( "episode" ).trimmed();
            auto title = match.captured( "title" ).replace( '.', ' ' ).trimmed();

            retVal.second = fOutPattern;
            retVal.second.replace( "%P", program );
            retVal.second.replace( "%S", season );
            retVal.second.replace( "%E", episode );
            retVal.second.replace( "%T", title );

            retVal.second += "." + ext;
            retVal.first = true;
        }
        fFileMapping[ fileName ] = retVal;
    }
    else
        retVal = (*pos).second;

    return retVal;
}

std::pair< bool, QStringList > CDirModel::transform( const QModelIndex & idx, bool displayOnly )
{
    if ( !idx.isValid() )
        return std::make_pair( false, QStringList() );

    QStringList retVal;
    auto newFile = transformFile( idx );
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
                retVal[ retVal.length() - 1 ] = QString( "ERROR: '%1' - No Longer Exists" ).arg( oldName );;
            }
            else
            {
                auto dir = fi.absoluteDir();
                auto newName = dir.absoluteFilePath( newFile.second );
                aOK = QFile::rename( oldName, newName );
                if ( !aOK )
                {
                    retVal[ retVal.length() - 1 ] = "ERROR: " + retVal[ retVal.length() - 1 ] + ": FAILED TO RENAME";
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

void CDirModel::patternChanged( const QModelIndex & idx )
{
    if ( !idx.isValid() )
        return;

    auto numRows = rowCount( idx );
    for( int ii = 0; ii < numRows; ++ii )
    {
        auto childIndex = index( ii, 4, idx );
        if ( childIndex.isValid() )
        {
            emit dataChanged( childIndex, childIndex );
            patternChanged( childIndex );
        }
    }
}
