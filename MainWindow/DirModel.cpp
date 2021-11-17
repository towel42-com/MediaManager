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
#include "SABUtils/FileUtils.h"

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
#include <QApplication>

#include <set>
#include <list>
#include "SearchTMDBInfo.h"

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

    fExcludedDirNames = { "#recycle", "#recycler", "extras" };
    fIgnoredNames = { "sub", "subs" };
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

QString CDirModel::getSearchName( const QModelIndex &idx ) const
{
    auto nm = index( idx.row(), CDirModel::EColumns::eTransformName, idx.parent() ).data().toString();
    if ( nm == "<NOMATCH>" || nm.isEmpty() )
    {
        nm = index( idx.row(), CDirModel::EColumns::eFSName, idx.parent() ).data( CDirModel::ECustomRoles::eFullPathRole ).toString();
        nm = nm.isEmpty() ? QString() : ( QFileInfo( nm ).isDir() ? QFileInfo( nm ).fileName() : QFileInfo( nm ).completeBaseName() );
    }
    return nm;
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
    setHorizontalHeaderLabels( QStringList() << "Name" << "Size" << "Type" << "Date Modified" << "Is TV Show?" <<"Transformed Name" );

    QFileInfo rootFI( fRootPath );

    TParentTree parentTree;
    loadFileInfo( rootFI.absoluteFilePath(), parentTree );

    if ( fTreeView )
    {
        fTreeView->resizeColumnToContents( EColumns::eFSName );
        fTreeView->resizeColumnToContents( EColumns::eFSSize );
        fTreeView->resizeColumnToContents( EColumns::eFSType );
        fTreeView->resizeColumnToContents( EColumns::eFSModDate );
        fTreeView->resizeColumnToContents( EColumns::eIsTVShow );
        fTreeView->resizeColumnToContents( EColumns::eTransformName );
    }

    emit sigDirReloaded();
}

bool CDirModel::setData( const QModelIndex & idx, const QVariant & value, int role )
{
    if ( role == Qt::CheckStateRole && idx.column() == EColumns::eIsTVShow )
    {
        auto isTVShow = value.toInt() == Qt::Checked;
        auto baseItem = getItemFromindex( idx );
        if ( baseItem )
            baseItem->setData( isTVShow, eIsTVShowRole );
        auto item = itemFromIndex( idx );
        item->setText( isTVShow ? "Yes" : "No" );
        updatePattern( item );
    }
    return QStandardItemModel::setData( idx, value, role );
}

void CDirModel::loadFileInfo( const QFileInfo & fileInfo, TParentTree & siblings )
{
    if ( !fileInfo.exists() )
        return;

    //for( auto && ii : parentTree )
    //{
    //    qDebug() << ii.first.front()->text() << ": Is Loaded? " << ii.second;
    //}
    //qDebug() << "Loading: " << fileInfo.absoluteFilePath();
    auto row = getItemRow( fileInfo );
    siblings.push_back( row );

    if ( fileInfo.isDir() )
    {
        QDir dir( fileInfo.absoluteFilePath() );
        dir.setFilter( QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable );
        dir.setSorting( QDir::Name | QDir::DirsFirst | QDir::IgnoreCase );
        dir.setNameFilters( fNameFilter );
        auto fileInfos = dir.entryInfoList();
        for ( auto &&ii : fileInfos )
        {
            if ( ii.isDir() && isExcludedDirName( ii ) )
                continue;
            
            loadFileInfo( ii, siblings );
        }
        qApp->processEvents();
    }
    else
    {
        attachTreeNodes( siblings );
    }

    if ( !siblings.back().second )
    {
        for ( auto &&ii : siblings.back().first )
            delete ii;
    }
    siblings.pop_back();
}

void CDirModel::attachTreeNodes( TParentTree &parentTree )
{
    QStandardItem *prevParent = nullptr;
    for( auto ii = parentTree.begin(); ii != parentTree.end(); ++ii )
    {
        auto nextParent = ( *ii ).first.front();
        if ( !(*ii).second ) // already been laoded
        {
            if ( prevParent )
            {
                bool isTVShow = nextParent->data( eIsTVShowRole ).toBool();
                bool isParentTVShow = prevParent->data( eIsTVShowRole ).toBool();

                if ( !isTVShow && ( isTVShow != isParentTVShow ) )
                {
                    setIsTVShow( ( *ii ).first[EColumns::eIsTVShow], isParentTVShow );
                    nextParent->setData( isParentTVShow, eIsTVShowRole );
                }

                prevParent->appendRow( ( *ii ).first );
            }
            else
            {
                appendRow( (*ii).first );
                nextParent->setData( true, ECustomRoles::eIsRoot );
            }
            ( *ii ).second = true;
        }
        prevParent = nextParent;
        fPathMapping[nextParent->data( ECustomRoles::eFullPathRole ).toString()] = nextParent;

        if ( fTreeView && prevParent )
            fTreeView->setExpanded( prevParent->index(), true );
    }
}

bool CDirModel::isExcludedDirName( const QFileInfo &ii ) const
{
    auto fn = ii.fileName().toLower();
    return fExcludedDirNames.find( fn ) != fExcludedDirNames.end();
}

bool CDirModel::isIgnoredPathName( const QFileInfo &ii ) const
{
    auto fn = ii.fileName().toLower();
    return fIgnoredNames.find( fn ) != fIgnoredNames.end();
}

TTreeNode CDirModel::getItemRow( const QFileInfo & fileInfo ) const
{
    QLocale locale;

    QList< QStandardItem * > retVal;
    auto nameItem = new QStandardItem( fileInfo.fileName() );
    nameItem->setIcon( fIconProvider->icon( fileInfo ) );
    nameItem->setData( fileInfo.absoluteFilePath(), ECustomRoles::eFullPathRole );
    nameItem->setData( fileInfo.isDir(), ECustomRoles::eIsDir );
    retVal.push_back( nameItem );
    retVal.push_back( new QStandardItem( fileInfo.isFile() ? locale.toString( fileInfo.size() ) : QString() ) );
    if ( fileInfo.isFile() )
    {
        retVal.back()->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );
    }
    retVal.push_back( new QStandardItem( fIconProvider->type( fileInfo ) ) );
    retVal.push_back( new QStandardItem( fileInfo.lastModified().toString( "MM/dd/yyyy hh:mm:ss.zzz") ) );
    bool isTVShow = SSearchTMDBInfo::looksLikeTVShow( fileInfo.fileName() ).first;
    auto isTVShowItem = new QStandardItem( QString() );
    isTVShowItem->setCheckable( true );
    setIsTVShow( isTVShowItem, isTVShow );

    nameItem->setData( isTVShow, ECustomRoles::eIsTVShowRole );
    retVal.push_back( isTVShowItem );
    auto transformInfo = transformItem( fileInfo );
    auto transformedItem = new QStandardItem( transformInfo.second );
    retVal.push_back( transformedItem );

    updatePattern( nameItem, transformedItem );
    return std::make_pair( retVal, false );
}

void CDirModel::setIsTVShow( QStandardItem * item, bool isTVShow ) const
{
    item->setText( isTVShow ? "Yes" : "No" );
    item->setCheckState( isTVShow ? Qt::Checked : Qt::Unchecked );
}

QString patternToRegExp( const QString & captureName, const QString & inPattern, const QString &value, bool removeOptional )
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

QString patternToRegExp( const QString & pattern, bool removeOptional )
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

QStandardItem * CDirModel::getItemFromPath( const QFileInfo & fi ) const
{
    auto path = fi.absoluteFilePath();
    auto pos = fPathMapping.find( path );
    if ( pos != fPathMapping.end() )
        return ( *pos ).second;
    return nullptr;
}

bool CDirModel::isValidName( const QFileInfo &fi ) const
{
    return isValidName( fi.absoluteFilePath(), fi.isDir() );
}

bool CDirModel::isValidName( const QString & path, bool isDir ) const
{
    bool asTVShow = treatAsTVShow( path, !isTVShow( path ) );
    auto fn = QFileInfo( path ).fileName();
    if (   (!asTVShow && fMoviePatterns.isValidName( fn, isDir ) )
         || (asTVShow && fTVPatterns.isValidName( fn, isDir ) ) )
        return true;

    if ( isDir )
    {
        QDir dir( path );
        auto children = dir.entryList( QDir::AllDirs | QDir::NoDotAndDotDot );
        return !children.empty();
    }
    return false;

}

bool SPatternInfo::isValidName( const QFileInfo & fi ) const
{
    return isValidName( fi.fileName(), fi.isDir() );
}

bool SPatternInfo::isValidName( const QString &name, bool isDir ) const
{
    if ( name.isEmpty() )
        return false;
    QStringList patterns;
    if ( isDir )
    {
        patterns
            << patternToRegExp( fOutDirPattern, false )
            << "(.*)\\s\\((\\d{2,4}\\))\\s(-\\s(.*)\\s)?\\[(tmdbid=\\d+)|(imdbid=tt.*)\\]"
            ;
    }
    else
    {
        patterns << patternToRegExp( fOutFilePattern, false )
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

void CDirModel::slotTVOutputFilePatternChanged( const QString &outPattern )
{
    fTVPatterns.fOutFilePattern = outPattern;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

void CDirModel::slotTVOutputDirPatternChanged( const QString &outPattern )
{
    fTVPatterns.fOutDirPattern = outPattern;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

void CDirModel::slotMovieOutputFilePatternChanged( const QString &outPattern )
{
    fMoviePatterns.fOutFilePattern = outPattern;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

void CDirModel::slotMovieOutputDirPatternChanged( const QString &outPattern )
{
    fMoviePatterns.fOutDirPattern = outPattern;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

void CDirModel::slotTreatAsTVByDefaultChanged( bool treatAsTVShowByDefault )
{
    fTreatAsTVShowByDefault = treatAsTVShowByDefault;
    // need to emit datachanged on column 4 for all known indexes
    patternChanged();
}

// do not include <> in the capture name
QString replaceCapture( const QString &captureName, const QString &returnPattern, const QString &value )
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

void cleanFileName( QString & inFile )
{
    QString text = "\\s*\\:\\s*";
    inFile.replace( QRegularExpression( text ), " - " );
    text = "[\\<\\>\\\"\\/\\\\\\|\\?\\*]";
    inFile.replace( QRegularExpression( text ), "" );
}

std::pair< bool, QString > CDirModel::transformItem( const QFileInfo &fileInfo ) const
{
    if ( treatAsTVShow( fileInfo, isTVShow( fileInfo ) ) )
        return transformItem( fileInfo, fTVPatterns );
    else
        return transformItem( fileInfo, fMoviePatterns );
}

std::pair< bool, QString > CDirModel::transformItem( const QFileInfo &fileInfo, const SPatternInfo & info ) const
{
    auto filePath = fileInfo.absoluteFilePath();

    //if ( !info.fInPatternRegExp.isValid() )
    //    return std::make_pair( false, QObject::tr( "<INVALID INPUT REGEX>" ) );

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
        if ( pos == fTitleInfoMapping.end() )
        {
            if ( isValidName( fileInfo ) || isIgnoredPathName( fileInfo ) )
                retVal.second = QString();
        }
        else 
        {
            auto title = (*pos).second->getTitle();
            auto year = ( *pos ).second->getYear();
            auto tmdbid = ( *pos ).second->fTMDBID;
            auto season = ( *pos ).second->fSeason;
            auto episode = ( *pos ).second->fEpisode;
            auto extraInfo = ( *pos ).second->fExtraInfo;
            auto episodeTitle = ( *pos ).second->fEpisodeTitle;

            retVal.second = fileInfo.isDir() ? info.fOutDirPattern : info.fOutFilePattern;
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

bool CDirModel::treatAsTVShow( const QFileInfo & fileInfo, bool defaultValue ) const
{
    bool asTVShow = defaultValue;
    auto pos = fTitleInfoMapping.find( fileInfo.absoluteFilePath() );
    if ( pos != fTitleInfoMapping.end() )
        asTVShow = ( *pos ).second->isTVShow();
    return asTVShow;
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

void CDirModel::setTitleInfo( QStandardItem * item, std::shared_ptr< STitleInfo > titleInfo, bool applyToChildren )
{
    auto idx = indexFromItem( item );
    setTitleInfo( idx, titleInfo, applyToChildren );
}

void CDirModel::setTitleInfo( const QModelIndex &idx, std::shared_ptr< STitleInfo > titleInfo, bool applyToChildren )
{
    if ( !idx.isValid() )
        return;

    if ( titleInfo && titleInfo->getTitle().isEmpty() )
        titleInfo.reset();

    auto fi = fileInfo( idx );
    if ( !titleInfo )
        fTitleInfoMapping.erase( fi.absoluteFilePath() );
    else
        fTitleInfoMapping[fi.absoluteFilePath()] = titleInfo;
    if ( isDir( idx ) )
        fDirMapping.erase( fi.absoluteFilePath() );
    else 
        fFileMapping.erase( fi.absoluteFilePath() );

    updatePattern( getItemFromindex( idx ) );

    if ( applyToChildren )
    {
        auto childCount = rowCount( idx );
        for ( int ii = 0; ii < childCount; ++ii )
        {
            auto childIdx = index( ii, CDirModel::EColumns::eFSName, idx );

            // exception for "SRT" files that are of the form X_XXXX.ext dont transform
            //auto txt = childIdx.data( CDirModel::ECustomRoles::eFullPathRole ).toString();
            if ( !isLanguageFile( childIdx ) )
            {
                auto childInfo = getTitleInfo( childIdx );
                if ( !childInfo )
                {
                    setTitleInfo( childIdx, titleInfo, applyToChildren );
                }
            }
        }
    }
}

bool CDirModel::isLanguageFile( const QFileInfo & fi ) const
{
    auto suffix = fi.suffix();
    static std::unordered_set< QString > extensions = { "srt", "sub", "idx" };
    if ( extensions.find( suffix ) != extensions.end() )
        return true;

    auto fn = fi.completeBaseName();
    auto regExp = QRegularExpression( "\\d+_\\S+" );
    if ( regExp.match( fn ).hasMatch() )
        return true;

    auto parentDir = fi.absoluteDir().dirName();
    if ( parentDir.toLower() == "subs" )
        return true;
    return false;
}

bool CDirModel::isLanguageFile( const QModelIndex &idx ) const
{
    auto path = idx.data( CDirModel::ECustomRoles::eFullPathRole ).toString();
    if ( path.isEmpty() )
        return false;

    return isLanguageFile( QFileInfo( path ) );
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

QString CDirModel::computeTransformPath( const QStandardItem * item, bool parentsOnly ) const
{
    if ( !item || ( item == invisibleRootItem() ) )
        return QString();
    if ( item->data( ECustomRoles::eIsRoot ).toBool() )
        return item->data( ECustomRoles::eFullPathRole ).toString();
    
    auto parentDir = computeTransformPath( item->parent(), false );

    auto transformItem = parentsOnly ? nullptr : getTransformItem( item );
    auto myName = transformItem ? transformItem->text() : QString();
    if ( myName.isEmpty() || ( myName == "<NOMATCH>" ) )
    {
        myName = item->text();
    }

    if ( myName.isEmpty() || parentDir.isEmpty() )
        return QString();

    auto retVal = QDir( parentDir ).absoluteFilePath( myName );
    return retVal;
}

QString CDirModel::getDispName( const QString & absPath ) const
{
    auto item = invisibleRootItem();
    if ( !invisibleRootItem() )
        return QString();
    auto root = invisibleRootItem()->child( 0, 0 );
    if ( !root->data( ECustomRoles::eIsRoot ).toBool() )
        return QString();

    auto rootDir = QDir( root->data( ECustomRoles::eFullPathRole ).toString() );
    return rootDir.relativeFilePath( absPath );
}

bool CDirModel::transform( const QStandardItem * item, bool displayOnly, QStandardItemModel * resultModel, QStandardItem * parentItem  ) const
{
    if ( !item )
        return false;

    bool aOK = true;
    QStandardItem * returnItem = nullptr;
    if ( item != invisibleRootItem() )
    {
        auto oldName = computeTransformPath( item, true );
        auto newName = computeTransformPath( item, false );

        if ( oldName != newName )
        {
            returnItem = new QStandardItem( QString( "'%1' => '%2'" ).arg( getDispName( oldName ) ).arg( getDispName( newName ) ) );

            returnItem->setData( oldName, Qt::UserRole + 1 );
            returnItem->setData( newName, Qt::UserRole + 2 );
            if ( parentItem )
                parentItem->appendRow( returnItem );
            else
                resultModel->appendRow( returnItem );
            if ( !displayOnly )
            {
                QFileInfo fi( oldName );
                if ( !fi.exists() )
                {
                    auto errorItem = new QStandardItem( QString( "ERROR: '%1' - No Longer Exists" ).arg( oldName ) );
                    returnItem->appendRow( errorItem );

                    QIcon icon;
                    icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                    errorItem->setIcon( icon );
                }
                else
                {
                    auto timeStamps = NFileUtils::timeStamps( oldName );
                    aOK = QFile::rename( oldName, newName );
                    if ( !aOK )
                    {
                        auto errorItem = new QStandardItem( QString( "ERROR: '%1' => '%2' : FAILED TO RENAME" ).arg( oldName ).arg( newName ) );
                        returnItem->appendRow( errorItem );

                        QIcon icon;
                        icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                        errorItem->setIcon( icon );
                    }
                    else
                    {
                        QString msg;
                        auto aOK = NFileUtils::setTimeStamps( newName, timeStamps );
                        if ( !aOK )
                        {
                            auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MODIFY TIMESTAMP: %2" ).arg( newName ).arg( msg ) );
                            returnItem->appendRow( errorItem );

                            QIcon icon;
                            icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                            errorItem->setIcon( icon );
                        }
                    }

                    QIcon icon;
                    icon.addFile( aOK ? QString::fromUtf8( ":/resources/ok.png" ) : QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                    returnItem->setIcon( icon );
                }
            }
        }
    }

    auto numRows = item->rowCount();
    for ( int ii = 0; ii < numRows; ++ii )
    {
        auto child = item->child( ii );
        if ( !child )
            continue;

        aOK = transform( child, displayOnly, resultModel, returnItem ) && aOK;
    }
    return aOK;
}

std::pair< bool, QStandardItemModel * > CDirModel::transform( bool displayOnly ) const
{
    CAutoWaitCursor awc;
    auto model = new QStandardItemModel;
    return std::make_pair( transform( invisibleRootItem(), displayOnly, model, nullptr ), model );
}


QStandardItem * CDirModel::getTransformItem( const QStandardItem * parent ) const
{
    auto idx = indexFromItem( parent );
    auto transformItem = itemFromIndex( index( idx.row(), CDirModel::EColumns::eTransformName, idx.parent() ) );
    return transformItem;
}

bool CDirModel::isTVShow( const QFileInfo &fileInfo ) const
{
    return isTVShow( fileInfo.absoluteFilePath() );
}

bool CDirModel::isTVShow( const QString &path ) const
{
    auto item = getItemFromPath( path );
    bool isTVShow = item ? item->data( eIsTVShowRole ).toBool() : false;
    return isTVShow;
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

        auto baseIdx = ( idx.column() == EColumns::eFSName ) ? idx : idx.model()->index( idx.row(), EColumns::eFSName, idx.parent() );
        auto baseItem = this->itemFromIndex( baseIdx );

        auto transformedIndex = idx.model()->index( idx.row(), EColumns::eTransformName, idx.parent() );
        auto transformedItem = this->itemFromIndex( transformedIndex );
        updatePattern( baseItem, transformedItem );
    }
}

void CDirModel::updatePattern( const QStandardItem * item, QStandardItem * transformedItem ) const
{
    if ( !item || !transformedItem )
        return;

    auto path = item->data( ECustomRoles::eFullPathRole ).toString();
    auto fileInfo = QFileInfo( path );
    auto transformInfo = transformItem( fileInfo );
    transformedItem->setBackground( Qt::white );
    if ( transformedItem->text() != transformInfo.second )
        transformedItem->setText( transformInfo.second );

    if ( transformInfo.second == "<NOMATCH>" || !isValidName( transformInfo.second, fileInfo.isDir() ) && !isValidName( fileInfo ) && !isIgnoredPathName( fileInfo ) && !isLanguageFile( fileInfo ) )
        transformedItem->setBackground( Qt::red );
}

