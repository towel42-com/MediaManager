// The MIT License( MIT )
//
// Copyright( c ) 2020-2021 Scott Aron Bloom
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

#include "TransformModel.h"
#include "Core/TransformResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/BackupFile.h"
#include "SABUtils/FileCompare.h"

#include "SABUtils/DoubleProgressDlg.h"

#include <QDir>
#include <QTimer>
#include <QTreeView>
#include <QApplication>
#include <QVariant>

#include <QDirIterator>

namespace NMediaManager
{
    namespace NModels
    {
        CTransformModel::CTransformModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
            fPatternTimer = new QTimer( this );
            fPatternTimer->setInterval( 50 );
            fPatternTimer->setSingleShot( true );
            connect( fPatternTimer, &QTimer::timeout, this, &CTransformModel::slotPatternChanged );
        }

        CTransformModel::~CTransformModel()
        {
        }

        QStringList CTransformModel::dirModelFilter() const
        {
            auto retVal = NPreferences::NCore::CPreferences::instance()->getVideoEncoderExtensions() << NPreferences::NCore::CPreferences::instance()->getSubtitleEncoderExtensions();
            retVal << NPreferences::NCore::CPreferences::instance()->getExtensionsToDelete();
            return retVal;
        }

        bool CTransformModel::setData( const QModelIndex &idx, const QVariant &value, int role )
        {
            if ( ( role == Qt::CheckStateRole ) && ( idx.column() == EColumns::eIsTVShow ) )
            {
                auto isTVShow = value.toInt() == Qt::Checked;
                auto baseItem = getPathItemFromIndex( idx );
                if ( baseItem && idx.column() == EColumns::eIsTVShow )
                    baseItem->setData( isTVShow, eIsTVShowRole );
                auto item = itemFromIndex( idx );
                updateTransformPattern( item );
            }
            return CDirModel::setData( idx, value, role );
        }

        bool CTransformModel::isValidName( const QFileInfo &fi, std::optional< bool > isTVShow ) const
        {
            return isValidName( fi.fileName(), fi.isDir(), isTVShow );
        }

        bool CTransformModel::isTVShow( const QModelIndex &idx ) const
        {
            if ( !idx.isValid() )
                return false;
            return isChecked( fileInfo( idx ), EColumns::eIsTVShow );
        }

        bool CTransformModel::isValidName( const QString &path, bool isDir, std::optional< bool > isTVShow ) const
        {
            bool defaultAsTVShow = isTVShow.has_value() ? isTVShow.value() : this->isChecked( path, EColumns::eIsTVShow );

            bool asTVShow = treatAsTVShow( path, defaultAsTVShow );
            if ( ( !asTVShow && fMoviePatterns.isValidName( path, isDir ) ) || ( asTVShow && fTVPatterns.isValidName( path, isDir ) ) )
                return true;

            return false;
        }

        void CTransformModel::slotTVOutputFilePatternChanged( const QString &outPattern )
        {
            fTVPatterns.setFilePattern( outPattern );
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CTransformModel::slotTVOutputDirPatternChanged( const QString &outPattern )
        {
            fTVPatterns.setDirPattern( outPattern );
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CTransformModel::slotMovieOutputFilePatternChanged( const QString &outPattern )
        {
            fMoviePatterns.setFilePattern( outPattern );
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CTransformModel::slotMovieOutputDirPatternChanged( const QString &outPattern )
        {
            fMoviePatterns.setDirPattern( outPattern );
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        std::pair< bool, QString > CTransformModel::transformItem( const QFileInfo &fileInfo ) const
        {
            if ( treatAsTVShow( fileInfo, isChecked( fileInfo, EColumns::eIsTVShow ) ) )
                return transformItem( fileInfo, fTVPatterns );
            else
                return transformItem( fileInfo, fMoviePatterns );
        }

        std::pair< bool, QString > CTransformModel::transformItem( const QFileInfo &fileInfo, const NCore::SPatternInfo &patternInfo ) const
        {
            auto filePath = fileInfo.absoluteFilePath();

            auto pos = fileInfo.isDir() ? fDirMapping.find( filePath ) : fFileMapping.find( filePath );
            auto retVal = std::make_pair( false, QString() );

            if ( pos == ( fileInfo.isDir() ? fDirMapping.end() : fFileMapping.end() ) )
            {
                QString fn = fileInfo.fileName();
                if ( !fileInfo.isDir() )
                {
                    fn = fileInfo.fileName();
                }

                auto transformInfo = getTransformResult( filePath, false );
                if ( !transformInfo )
                {
                    if ( isValidName( fileInfo ) || isIgnoredPathName( fileInfo ) )
                    {
                        retVal.first = true;
                        retVal.second = QString();
                    }
                }
                else if ( transformInfo->isNotFoundResult() )
                {
                    retVal.first = false;
                    retVal.second = transformInfo->getTitle();
                }
                else
                {
                    if ( transformInfo->isDeleteResult() )
                    {
                        retVal.first = true;
                        retVal.second = transformInfo->getTitle();
                    }
                    else
                    {
                        retVal.first = true;
                        retVal.second = transformInfo->transformedName( fileInfo, patternInfo, false );
                    }
                }

                if ( fileInfo.isDir() )
                    fDirMapping[ filePath ] = retVal;
                else
                    fFileMapping[ filePath ] = retVal;
                clearPathStatusCache( filePath );
            }
            else
                retVal = ( *pos ).second;

            return retVal;
        }

        void CTransformModel::computeEpisodesForDiskNumbers()
        {
            TTitleMap titleMap;
            findTitlesPerDiskNumbers( index( 0, 0 ), titleMap );

            for ( auto &&searchNamePos : titleMap )   // search name
            {
                for ( auto &&seasonPos : searchNamePos.second )
                {
                    auto episodeNum = 1;
                    for ( auto &&diskNumPos : seasonPos.second )
                    {
                        for ( auto &&titleNumPos : diskNumPos.second )
                        {
                            auto searchName = QString( "%1 - S%2E%3" ).arg( searchNamePos.first ).arg( seasonPos.first, 2, 10, QChar( '0' ) ).arg( episodeNum, 2, 10, QChar( '0' ) );
                            fDiskRipSearchMap[ titleNumPos.second ] = searchName;
                            episodeNum++;
                        }
                    }
                }
            }
        }

        void CTransformModel::findTitlesPerDiskNumbers( const QModelIndex &parentIdx, TTitleMap &retVal )
        {
            if ( !isDir( parentIdx ) )
                return;

            auto name = parentIdx.data().toString();
            int diskNumber;
            if ( NCore::SSearchTMDBInfo::hasDiskNumber( name, diskNumber, {} ) )
            {
                auto childPaths = getDiskTitles( parentIdx );
                if ( !childPaths.empty() )
                {
                    //QString title;

                    NCore::SSearchTMDBInfo info( name, {} );

                    auto title = info.searchName().toLower();
                    auto season = info.season();

                    retVal[ title ][ season ][ diskNumber ] = childPaths;
                }
            }

            auto rowCount = this->rowCount( parentIdx );
            for ( int ii = 0; ii < rowCount; ++ii )
            {
                auto childIndex = index( ii, 0, parentIdx );
                findTitlesPerDiskNumbers( childIndex, retVal );
            }
        }

        std::map< int, QString > CTransformModel::getDiskTitles( const QModelIndex &parentIdx )
        {
            auto rowCount = this->rowCount( parentIdx );
            std::map< int, QString > retVal;
            for ( int ii = 0; ii < rowCount; ++ii )
            {
                auto childIndex = index( ii, 0, parentIdx );
                if ( !isDir( childIndex ) )
                {
                    auto name = childIndex.data().toString();
                    int titleNum = -1;
                    if ( NCore::SSearchTMDBInfo::isRippedWithMKV( name, &titleNum ) )
                    {
                        retVal[ titleNum ] = childIndex.data( NModels::ECustomRoles::eAbsFilePath ).toString();
                    }
                }
            }
            return retVal;
        }

        QString CTransformModel::getSearchName( const QModelIndex &idx ) const
        {
            if ( !idx.isValid() )
                return {};

            if ( isRootPath( idx ) )
                return {};

            auto path = idx.data( ECustomRoles::eAbsFilePath ).toString();
            auto pos = fDiskRipSearchMap.find( path );
            if ( pos != fDiskRipSearchMap.end() )
                return ( *pos ).second;

            if ( NCore::SSearchTMDBInfo::isRippedWithMKV( fileInfo( idx ) ) )
                return getSearchName( idx.parent() );

            auto nm = index( idx.row(), EColumns::eTransformName, idx.parent() ).data().toString();
            if ( NCore::CTransformResult::isAutoSetText( nm ) || nm.isEmpty() )
            {
                nm = index( idx.row(), NModels::EColumns::eFSName, idx.parent() ).data( ECustomRoles::eAbsFilePath ).toString();
                nm = nm.isEmpty() ? QString() : ( QFileInfo( nm ).isDir() ? QFileInfo( nm ).fileName() : QFileInfo( nm ).completeBaseName() );
            }
            else if ( !nm.isEmpty() && QFileInfo( path ).isFile() )
            {
                nm = QFileInfo( nm ).completeBaseName();
            }

            return nm;
        }

        bool CTransformModel::treatAsTVShow( const QFileInfo &fileInfo, bool isTVByDefault ) const
        {
            auto mediaType = isTVByDefault ? NCore::EMediaType::eTVShow : NCore::EMediaType::eMovie;
            auto transformInfo = getTransformResult( fileInfo, true );
            if ( transformInfo && !transformInfo->isNoMatch() )
                mediaType = transformInfo->mediaType();
            return isTVType( mediaType );
        }

        void CTransformModel::processPostAutoSearch()
        {
            processPostAutoSearch( invisibleRootItem() );
        }

        void CTransformModel::processPostAutoSearch( QStandardItem *item )
        {
            if ( !item )
                return;

            auto myTransformItem = getTransformItem( item );
            auto myTransformResult = getTransformResult( item, false );
            bool transformChanged = false;

            auto currItem = item;
            while ( currItem && ( currItem != invisibleRootItem() ) && !itemSearchOK( indexFromItem( myTransformItem ) ) )
            {
                auto parent = currItem->parent();
                if ( parent )
                {
                    myTransformItem = getTransformItem( parent );
                    myTransformResult = getTransformResult( parent, false );
                    transformChanged = true;
                }
                else
                {
                    myTransformItem = nullptr;
                    myTransformResult.reset();
                }

                currItem = parent;
            }

            if ( myTransformResult && transformChanged )
            {
                setSearchResult( item, myTransformResult, false, true );
            }
            auto numRows = item->rowCount();
            for ( int ii = 0; ii < numRows; ++ii )
            {
                auto child = item->child( ii );
                if ( child )
                {
                    processPostAutoSearch( child );
                }
            }
        }

        NCore::EMediaType CTransformModel::searchForMediaType( const QModelIndex &idx ) const
        {
            auto name = getSearchName( idx );
            auto retVal = NCore::SSearchTMDBInfo::looksLikeTVShow( name, nullptr, nullptr, nullptr, false );
            if ( !isTVType( retVal ) && !isMovieType( retVal ) )
            {
                auto child = index( 0, 0, idx );
                if ( child.isValid() )
                {
                    retVal = searchForMediaType( child );
                    if ( isTVType( retVal ) || isMovieType( retVal ) )
                        return retVal;
                }
                child = index( 1, 0, idx );
                if ( child.isValid() )
                {
                    retVal = searchForMediaType( child );
                    if ( isTVType( retVal ) || isMovieType( retVal ) )
                        return retVal;
                }
                auto parent = idx.parent();
                if ( parent.isValid() && !isRootPath( parent ) )
                {
                    retVal = searchForMediaType( parent );
                    if ( isTVType( retVal ) || isMovieType( retVal ) )
                        return retVal;
                }
            }
            return retVal;
        }

        QModelIndex CTransformModel::findSearchableChild( const QModelIndex &idx ) const
        {
            if ( !idx.isValid() )
                return {};

            QModelIndex nextIdx;
            if ( isDir( idx ) )
            {
                nextIdx = idx.model()->index( 0, 0, idx );
                auto retVal = findSearchableChild( nextIdx );
                while ( nextIdx.isValid() && !retVal.isValid() )
                {
                    nextIdx = idx.model()->index( nextIdx.row() + 1, 0, idx );
                    retVal = findSearchableChild( nextIdx );
                }
                return retVal;
            }
            else
            {
                if ( isSearchableFile( idx.data().toString() ) )
                    return idx;
                nextIdx = index( idx.row() + 1, 0, idx.parent() );
                return findSearchableChild( nextIdx );
            }
        }

        bool CTransformModel::isSearchableFile( const QString &fileName ) const
        {
            return !fileName.contains( "E00", Qt::CaseInsensitive );
        }

        void CTransformModel::setInAutoSearch( bool val, bool reset )
        {
            beginResetModel();
            if ( reset )
                fInAutoSearch.reset();
            else
                fInAutoSearch = val;
            endResetModel();
            if ( filesView() )
                filesView()->expandAll();
        }

        void CTransformModel::setDeleteItem( const QModelIndex &idx )
        {
            auto searchResult = std::make_shared< NCore::CTransformResult >( NCore::EMediaType::eDeleteFileType );
            setSearchResult( idx, searchResult, false, true );
        }

        void CTransformModel::setSearchResult( QStandardItem *item, std::shared_ptr< NCore::CTransformResult > searchResult, bool applyToChildren, bool forceSet )
        {
            auto idx = indexFromItem( item );
            setSearchResult( idx, searchResult, applyToChildren, forceSet );
        }

        void CTransformModel::setSearchResult( const QModelIndex &idx, std::shared_ptr< NCore::CTransformResult > searchResult, bool applyToChildren, bool forceSet )
        {
            if ( !idx.isValid() )
                return;

            if ( searchResult && searchResult->getTitle().isEmpty() )
                searchResult.reset();

            auto fi = fileInfo( idx );
            if ( !isIgnoredPathName( fi ) )
            {
                if ( !searchResult )
                    fTransformResultMap.erase( fi.absoluteFilePath() );
                else
                    fTransformResultMap[ fi.absoluteFilePath() ] = searchResult;

                if ( isDir( idx ) )
                    fDirMapping.erase( fi.absoluteFilePath() );
                else
                    fFileMapping.erase( fi.absoluteFilePath() );

                updateTransformPattern( getPathItemFromIndex( idx ) );
            }
            if ( applyToChildren )
            {
                auto childCount = rowCount( idx );
                for ( int ii = 0; ii < childCount; ++ii )
                {
                    auto childIdx = index( ii, NModels::EColumns::eFSName, idx );

                    // exception for "SRT" files that are of the form X_XXXX.ext dont transform
                    //auto txt = childIdx.data( ECustomRoles::eFullPathRole ).toString();
                    if ( !isSubtitleFile( childIdx ) )
                    {
                        auto childInfo = getTransformResult( childIdx, false );
                        bool isDelete = childInfo && childInfo->isDeleteResult();
                        if ( ( forceSet && !isDelete ) || !childInfo )
                        {
                            setSearchResult( childIdx, searchResult, applyToChildren, forceSet );
                        }
                    }
                }
            }
        }

        void CTransformModel::clearSearchResult( const QModelIndex &idx, bool recursive )
        {
            setSearchResult( idx, {}, recursive, true );
        }

        std::pair< bool, QStandardItem * > CTransformModel::processItem( const QStandardItem *item, bool displayOnly )
        {
            QStandardItem *myItem = nullptr;
            bool aOK = true;
            bool processItem = !NPreferences::NCore::CPreferences::instance()->getOnlyTransformDirectories() || isDir( item );

            QString oldName;
            QString newName;
            if ( processItem )
            {
                oldName = computeTransformPath( item, true );
                newName = computeTransformPath( item, false );
                processItem = processItem && ( oldName != newName );
            }

            if ( processItem )
            {
                QFileInfo oldFileInfo( oldName );
                QFileInfo newFileInfo( newName );

                if ( NCore::CTransformResult::isDeleteThis( newName ) )
                    myItem = new QStandardItem( tr( "Delete '%1'" ).arg( getDispName( oldName ) ) );
                else
                    myItem = new QStandardItem( QString( "'%1' => '%2'" ).arg( getDispName( oldName ) ).arg( getDispName( newName ) ) );

                myItem->setData( oldName, ECustomRoles::eOldName );
                myItem->setData( newName, ECustomRoles::eNewName );

                if ( !displayOnly )
                {
                    bool removeIt = NCore::CTransformResult::isDeleteThis( newName );
                    if ( progressDlg() )
                    {
                        if ( removeIt )
                            progressDlg()->setLabelText( tr( "Removing '%1'" ).arg( getDispName( oldName ) ) );
                        else
                            progressDlg()->setLabelText( tr( "Renaming '%1' => '%2'" ).arg( getDispName( oldName ) ).arg( getDispName( newName ) ) );
                    }

                    bool dirAlreadyExisted = ( oldFileInfo.exists() && oldFileInfo.isDir() && newFileInfo.exists() && newFileInfo.isDir() );
                    if ( dirAlreadyExisted )
                    {
                        // new directory already exists, do move everything from old to new
                        auto it = QDirIterator( oldName, QDir::NoDotAndDotDot | QDir::AllEntries, QDirIterator::Subdirectories );
                        bool allDeletedOK = true;
                        while ( it.hasNext() )
                        {
                            it.next();
                            auto fi = it.fileInfo();
                            auto oldPath = fi.absoluteFilePath();
                            auto relPath = QDir( oldFileInfo.absoluteFilePath() ).relativeFilePath( oldPath );
                            auto newPath = QDir( newFileInfo.absoluteFilePath() ).absoluteFilePath( relPath );
                            if ( !NSABUtils::NFileUtils::backup( newPath ) )
                            {
                                appendError( myItem, tr( "%1: FAILED TO BACKUP ITEM" ).arg( newPath ) );
                                allDeletedOK = false;
                            }
                            if ( !QFile::rename( oldPath, newPath ) )
                            {
                                appendError( myItem, tr( "%1: FAILED TO MOVE ITEM TO %2" ).arg( oldPath ).arg( newPath ) );
                                allDeletedOK = false;
                            }
                        }
                        if ( allDeletedOK )
                        {
                            auto dir = QDir( oldName );
                            aOK = dir.removeRecursively();
                            if ( !aOK )
                            {
                                appendError( myItem, tr( "Failed to Remove '%1'" ).arg( oldName ) );
                            }
                        }
                        progressDlg()->setValue( progressDlg()->value() + 4 );
                    }
                    else
                    {
                        if ( !checkProcessItemExists( oldName, myItem, removeIt ) )
                        {
                        }
                        else if ( oldFileInfo.exists() && removeIt )
                        {
                            QString errorMsg;
                            if ( oldFileInfo.isDir() )
                            {
                                auto dir = QDir( oldName );
                                aOK = dir.removeRecursively();
                            }
                            else
                            {
                                auto parentDir = oldFileInfo.absoluteDir();
                                auto pathToDelete = oldFileInfo.absoluteFilePath();
                                auto file = QFile( pathToDelete );
                                aOK = file.remove();
                                if ( !aOK )
                                    errorMsg = file.errorString();
                                auto peerFiles = parentDir.entryInfoList(
                                    QStringList() << "*"
                                                  << "*.*",
                                    QDir::NoDotAndDotDot | QDir::Files );
                                if ( aOK && peerFiles.isEmpty() )
                                {
                                    oldName = parentDir.absolutePath();
                                    aOK = parentDir.removeRecursively();
                                }
                            }
                            if ( !aOK )
                            {
                                appendError( myItem, tr( "Failed to Remove '%1' - '%2'" ).arg( oldName ).arg( errorMsg ) );
                            }
                        }
                        else
                        {
                            auto timeStamps = NSABUtils::NFileUtils::timeStamps( oldName );
                            if ( progressDlg() )
                            {
                                progressDlg()->setValue( progressDlg()->value() + 1 );
                                qApp->processEvents();
                            }

                            auto transFormItem = getTransformItem( item );
                            bool parentPathOK = true;
                            if ( transFormItem )
                            {
                                auto mySubPath = transFormItem->text();
                                if ( ( mySubPath.indexOf( "/" ) != -1 ) || ( mySubPath.indexOf( "\\" ) != -1 ) )
                                {
                                    auto parentPath = computeTransformPath( item->parent(), false );
                                    mySubPath = computeMergedPath( parentPath, mySubPath );

                                    auto mySplit = mySubPath.split( "/" );
                                    auto leafName = mySplit.back();
                                    mySplit.pop_back();

                                    auto myParentPath = mySplit.join( "/" );
                                    parentPathOK = QDir( myParentPath ).mkpath( "." );
                                    if ( !parentPathOK )
                                    {
                                        appendError( myItem, tr( "'%1' => '%2' : FAILED TO MAKE PARENT DIRECTORY PATH" ).arg( oldName ).arg( newName ) );
                                    }
                                }
                            }
                            if ( progressDlg() )
                            {
                                progressDlg()->setValue( progressDlg()->value() + 1 );
                                qApp->processEvents();
                            }

                            QString errorMsg;
                            if ( parentPathOK )
                            {
                                if ( newFileInfo.exists() && newFileInfo.isFile() && oldFileInfo.isFile() && newFileInfo != oldFileInfo )
                                {
                                    if ( NSABUtils::NFileUtils::CFileCompare( oldFileInfo, newFileInfo ).compare() )
                                    {
                                        aOK = QFile( oldName ).remove();
                                        if ( !aOK )
                                            errorMsg = QString( "Destination file '%1' exists and is identical to the new '%2' file, but the old file can not be deleted" ).arg( oldName ).arg( newName );
                                    }
                                    else
                                    {
                                        aOK = false;
                                        errorMsg =
                                            QString( "Destination file Exists - Old Size: %1 New Size: %2" ).arg( NSABUtils::NFileUtils::byteSizeString( oldName, false ) ).arg( NSABUtils::NFileUtils::byteSizeString( newName, false ) );
                                    }
                                }
                                else
                                {
                                    if ( !NSABUtils::NFileUtils::backup( newName ) )
                                    {
                                        errorMsg = tr("Could not backup %1").arg( newName );
                                    }
                                    auto fi = QFile( oldName );
                                    aOK = fi.rename( newName );
                                    if ( !aOK )
                                        errorMsg = fi.errorString();
                                    else
                                    {
                                        updateTransformResults( newName, oldName );
                                    }
                                }
                            }
                            else
                                aOK = false;
                            if ( progressDlg() )
                            {
                                progressDlg()->setValue( progressDlg()->value() + 1 );
                                qApp->processEvents();
                            }

                            if ( parentPathOK && !aOK )
                            {
                                appendError( myItem, tr( "'%1' => '%2' : FAILED TO RENAME - %3" ).arg( oldName ).arg( newName ).arg( errorMsg ) );
                            }
                            else if ( parentPathOK && !dirAlreadyExisted )
                            {
                                auto transformResult = getTransformResult( oldName, true );
                                while ( !transformResult && item->parent() )
                                {
                                    auto parentsOldName = computeTransformPath( item->parent(), true );
                                    transformResult = getTransformResult( parentsOldName, true );
                                    item = item->parent();
                                }

                                QString msg;
                                auto aOK = setMediaTags( newName, transformResult, msg );
                                if ( progressDlg() )
                                {
                                    progressDlg()->setValue( progressDlg()->value() + 1 );
                                    qApp->processEvents();
                                }

                                if ( !aOK )
                                {
                                    appendError( myItem, tr( "%1: FAILED TO MODIFY TAGS: %2" ).arg( newName ).arg( msg ) );
                                }
                                else
                                {
                                    aOK = NSABUtils::NFileUtils::setTimeStamps( newName, timeStamps );
                                    if ( progressDlg() )
                                    {
                                        progressDlg()->setValue( progressDlg()->value() + 1 );
                                        qApp->processEvents();
                                    }
                                    if ( !aOK )
                                    {
                                        appendError( myItem, tr( "%1: FAILED TO MODIFY TIMESTAMP" ).arg( newName ) );
                                    }
                                }
                            }
                            else
                                aOK = parentPathOK;
                        }
                    }

                    QIcon icon;
                    icon.addFile( aOK ? QString::fromUtf8( ":/resources/ok.png" ) : QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                    myItem->setIcon( icon );
                    if ( progressDlg() )
                    {
                        progressDlg()->setValue( progressDlg()->value() + 1 );
                        qApp->processEvents();
                    }
                }
            }
            return std::make_pair( aOK, myItem );
        }

        QStandardItem *CTransformModel::getTransformItem( const QStandardItem *item ) const
        {
            return getItem( item, EColumns::eTransformName );
        }

        void CTransformModel::transformPatternChanged()
        {
            fPatternTimer->stop();
            fPatternTimer->start();
        }

        void CTransformModel::clear()
        {
            clearResults();
            CDirModel::clear();
        }

        void CTransformModel::clearStatusResults()
        {
            beginResetModel();
            CDirModel::resetStatusCaches();
            fTransformResultMap.clear();
            endResetModel();
            if ( filesView() )
                filesView()->expandAll();
        }

        void CTransformModel::clearResults()
        {
            clearStatusResults();
            fFileMapping.clear();
            fDirMapping.clear();
            fDiskRipSearchMap.clear();
        }

        void CTransformModel::slotPatternChanged()
        {
            clearResults();
            transformPatternChanged( invisibleRootItem() );
        }

        void CTransformModel::transformPatternChanged( const QStandardItem *item )
        {
            if ( !item )
                return;

            updateTransformPattern( item );

            auto numRows = item->rowCount();
            for ( int ii = 0; ii < numRows; ++ii )
            {
                auto child = item->child( ii );
                if ( child )
                {
                    transformPatternChanged( child );
                }
            }
        }

        void CTransformModel::updateTransformPattern( const QStandardItem *item ) const
        {
            if ( item != invisibleRootItem() )
            {
                auto idx = item->index();

                auto baseIdx = ( idx.column() == NModels::EColumns::eFSName ) ? idx : idx.model()->index( idx.row(), NModels::EColumns::eFSName, idx.parent() );
                auto baseItem = this->itemFromIndex( baseIdx );

                auto transformedItem = getTransformItem( baseItem );
                updateTransformPattern( baseItem, transformedItem );
            }
        }

        void CTransformModel::updateTransformPattern( const QStandardItem *item, QStandardItem *transformedItem ) const
        {
            if ( !item || !transformedItem )
                return;

            auto path = item->data( ECustomRoles::eAbsFilePath ).toString();
            auto fileInfo = QFileInfo( path );

            auto transformInfo = transformItem( fileInfo );
            auto currText = item->text();
            transformedItem->setText( QString() );
            if ( !transformInfo.first || ( currText != transformInfo.second ) )
            {
                if ( transformedItem->text() != transformInfo.second )
                    transformedItem->setText( transformInfo.second );
            }
        }

        bool CTransformModel::canComputeStatus() const
        {
            return autoSearchFinished() && CDirModel::canComputeStatus();
        }

        QDate CTransformModel::getMediaDate( const QFileInfo &fi ) const
        {
            auto transformResults = getTransformResult( fi, true );
            if ( transformResults )
                return transformResults->getDate().first;
            return CDirModel::getMediaDate( fi );
        }

        bool CTransformModel::itemSearchOK( const QModelIndex &idx, QString *msg ) const
        {
            bool retVal = true;

            auto fileInfo = this->fileInfo( idx );

            QStringList msgs;
            if ( canAutoSearch( fileInfo, false ) )
            {
                auto transformInfo = transformItem( fileInfo );
                if ( NCore::CTransformResult::isDeleteThis( transformInfo.second ) )
                {
                    msgs << tr( "File Scheduled for deletion." );
                }
                else if ( NCore::CTransformResult::isNoMatch( transformInfo.second ) )
                {
                    retVal = false;
                    msgs << tr( "No Match found" );
                }
                else if ( !transformInfo.first )
                {
                    retVal = false;
                    msgs << tr( "Could not properly transform item" );
                }
                else
                    msgs << transformInfo.second;
            }
            if ( msg )
            {
                msgs.removeAll( QString() );
                *msg = msgs.join( "\n" );
            }
            return retVal;
        }

        std::optional< TItemStatus > CTransformModel::computeItemStatus( const QModelIndex &idx ) const
        {
            if ( isRootPath( idx ) )
                return {};

            if ( idx.column() != EColumns::eTransformName )
                return {};

            auto fileInfo = this->fileInfo( idx );
            TItemStatus retVal = { NPreferences::EItemStatus::eOK, QString() };
            auto path = fileInfo.absoluteFilePath();

            std::function< TItemStatus( const QModelIndex &idx ) > statusFunc = [ this, fileInfo ]( const QModelIndex &idx ) -> TItemStatus
            {
                TItemStatus retVal = { NPreferences::EItemStatus::eOK, QString() };
                if ( idx.model()->index( idx.row(), 0, idx.parent() ).data( Qt::CheckStateRole ).toInt() == Qt::CheckState::Unchecked )
                    return retVal;
                if ( NPreferences::NCore::CPreferences::instance()->getOnlyTransformDirectories() && !fileInfo.isDir() )
                    return retVal;

                auto searchOK = itemSearchOK( idx, &retVal.second );
                if ( !searchOK )
                    retVal.first = NPreferences::EItemStatus::eError;
                else if ( !retVal.second.isEmpty() && ( retVal.second != fileInfo.fileName() ) )
                    retVal.first = NPreferences::EItemStatus::eWarning;
                else
                {
                    retVal.first = NPreferences::EItemStatus::eOK;
                    retVal.second.clear();
                }
                return retVal;
            };

            bool nameOK = false;
            if ( isSeasonDir( idx, &nameOK ) && !nameOK )
                retVal = statusFunc( idx );
            else if ( isIgnoredPathName( fileInfo ) && !NPreferences::NCore::CPreferences::instance()->isSubtitleFile( fileInfo ) )
                retVal.first = NPreferences::EItemStatus::eOK;
            else if ( fileInfo.isDir() || isMediaFile( fileInfo ) )
                retVal = statusFunc( idx );

            return retVal;
        }

        void CTransformModel::postAddItems( const QFileInfo &fileInfo, std::list< SDirNodeItem > &currItems ) const
        {
            bool isTVShow = isTVType( NCore::SSearchTMDBInfo::looksLikeTVShow( fileInfo.fileName(), nullptr ) );

            currItems.front().setData( isTVShow, ECustomRoles::eIsTVShowRole );
        }

        void CTransformModel::setupNewItem( const SDirNodeItem &nodeItem, const QStandardItem *nameItem, QStandardItem *item ) const
        {
            if ( nodeItem.fType == EColumns::eIsTVShow )
                setChecked( item, isTVType( nodeItem.fMediaType ) );
            else if ( nodeItem.fType == EColumns::eTransformName )
            {
                if ( nameItem && item )
                {
                    updateTransformPattern( nameItem, item );
                }
            }
        }

        std::list< SDirNodeItem > CTransformModel::addAdditionalItems( const QFileInfo &fileInfo ) const
        {
            std::list< SDirNodeItem > retVal;

            auto mediaType = NCore::SSearchTMDBInfo::looksLikeTVShow( fileInfo.fileName(), nullptr );
            auto isTVShowItem = SDirNodeItem( QString(), EColumns::eIsTVShow );
            isTVShowItem.fMediaType = mediaType;
            isTVShowItem.fCheckable = { true, true, Qt::CheckState::Unchecked };

            retVal.push_back( isTVShowItem );

            auto transformInfo = transformItem( fileInfo );
            auto transformedItem = SDirNodeItem( transformInfo.second, EColumns::eTransformName );
            retVal.push_back( transformedItem );

            auto mediaInfo = CDirModel::addAdditionalItems( fileInfo );
            retVal.insert( retVal.end(), mediaInfo.begin(), mediaInfo.end() );

            return retVal;
        }

        QStringList CTransformModel::headers() const
        {
            return CDirModel::headers() << tr( "Is TV Show?" ) << tr( "New Name" ) << getMediaHeaders();
        }

        void CTransformModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CTransformModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CTransformModel::attachTreeNodes( QStandardItem *nextParent, QStandardItem *&prevParent, const STreeNode &ii )
        {
            bool isTVShow = nextParent->data( eIsTVShowRole ).toBool();
            bool isParentTVShow = prevParent->data( eIsTVShowRole ).toBool();

            if ( !isTVShow && ( isTVShow != isParentTVShow ) )
            {
                setChecked( ii.item( static_cast< NModels::EColumns >( EColumns::eIsTVShow ) ), isParentTVShow );
                nextParent->setData( isParentTVShow, eIsTVShowRole );
            }
        }

        void CTransformModel::postReloadModelRequest()
        {
            fPatternTimer->stop();   // if its runinng when this timer stops its realoaded anyway
        }

        QString CTransformModel::getMyTransformedName( const QStandardItem *item, bool transformParentsOnly ) const
        {
            auto transformItem = transformParentsOnly ? nullptr : getTransformItem( item );
            auto myName = transformItem ? transformItem->text() : QString();
            if ( myName.isEmpty() || ( NCore::CTransformResult::isNoItems( myName ) ) || ( NCore::CTransformResult::isNoMatch( myName ) ) )
            {
                myName = item->text();
            }
            return myName;
        }

        void CTransformModel::postDirFunction( bool aOK, const QFileInfo & /*dirInfo*/, TParentTree &parentTree, bool /*countOnly*/ )
        {
            if ( !NPreferences::NCore::CPreferences::instance()->getOnlyTransformDirectories() )
                return;

            if ( !aOK )
                return;

            CDirModel::attachTreeNodes( parentTree );
        }

        void CTransformModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree &tree, bool /*countOnly*/ )
        {
            if ( tree.empty() )
                return;

            auto item = tree.back().rootItem( true );
            if ( !item )
                return;
            auto parent = item->parent();
            if ( !parent )
                return;
            filesView()->setRowHidden( 0, parent->index(), true );
        }

        bool CTransformModel::preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > &alreadyAdded, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
            if ( !NPreferences::NCore::CPreferences::instance()->getOnlyTransformDirectories() )
                return true;
            if ( isSearchableFile( fileInfo.fileName() ) )
            {
                auto pos = alreadyAdded.find( fileInfo.absolutePath() );
                if ( pos != alreadyAdded.end() )
                    return false;

                alreadyAdded.insert( fileInfo.absolutePath() );
                return true;
            }
            return false;
        }

        bool CTransformModel::setMediaTags( const QString &fileName, std::shared_ptr< NCore::CTransformResult > &searchResults, QString &msg ) const
        {
            QString year;
            QString title = QFileInfo( fileName ).completeBaseName();
            if ( searchResults )
            {
                year = searchResults->getYear();
                title = searchResults->transformedName( fileName, searchResults->isTVShow() ? fTVPatterns : fMoviePatterns, true );
            }
            return CDirModel::setMediaTags( fileName, title, year, QString(), &msg );
        }

        bool CTransformModel::canAutoSearch( const QModelIndex &index, bool recursive ) const
        {
            auto path = index.data( ECustomRoles::eAbsFilePath ).toString();
            if ( path.isEmpty() )
                return false;
            return canAutoSearch( QFileInfo( path ), recursive );
        }

        bool CTransformModel::canAutoSearch( const QFileInfo &fileInfo, bool recursive ) const
        {
            if ( isIgnoredPathName( fileInfo, false ) || isSkippedPathName( fileInfo, false ) )
                return false;

            bool isLangFormat;
            if ( NPreferences::NCore::CPreferences::instance()->isSubtitleFile( fileInfo, &isLangFormat ) && !isLangFormat )
                return false;

            if ( fileInfo.isFile() && !isMediaFile( fileInfo ) )
                return false;

            if ( !fileInfo.isDir() )
                return true;

            auto files = QDir( fileInfo.absoluteFilePath() ).entryInfoList( QDir::Files );
            bool hasFiles = false;
            for ( auto &&ii : files )
            {
                //qDebug() << ii.absoluteFilePath();
                if ( !recursive && ii.isDir() )
                    continue;
                if ( canAutoSearch( ii, recursive ) )
                {
                    hasFiles = true;
                    break;
                }
            }

            return hasFiles;
        }

        std::shared_ptr< NCore::CTransformResult > CTransformModel::getTransformResult( QStandardItem *item, bool checkParents ) const
        {
            auto idx = indexFromItem( item );
            return getTransformResult( idx, checkParents );
        }

        std::shared_ptr< NCore::CTransformResult > CTransformModel::getTransformResult( const QModelIndex &idx, bool checkParents ) const
        {
            if ( !idx.isValid() )
                return {};

            auto fi = fileInfo( idx );
            return getTransformResult( fi, checkParents );
        }

        std::shared_ptr< NCore::CTransformResult > CTransformModel::getTransformResult( const QString &path, bool checkParents ) const
        {
            return getTransformResult( QFileInfo( path ), checkParents );
        }

        std::shared_ptr< NCore::CTransformResult > CTransformModel::getTransformResult( const QFileInfo &fi, bool checkParents ) const
        {
            auto path = fi.absoluteFilePath();
            if ( path.isEmpty() )
                return {};

            auto pos = fTransformResultMap.find( path );
            if ( pos != fTransformResultMap.end() )
                return ( *pos ).second;

            if ( !checkParents )
                return {};

            if ( isRootPath( fi.absoluteFilePath() ) )
                return {};

            auto regExStr = R"(^([A-Z]\:(\\|\/)|(\/))$)";
            auto regEx = QRegularExpression( regExStr, QRegularExpression::CaseInsensitiveOption );
            auto match = regEx.match( path );
            if ( match.hasMatch() )
                return {};

            return getTransformResult( fi.absolutePath(), true );
        }

        void CTransformModel::updateTransformResults( const QString &newName, const QString &oldName )
        {
            if ( !QFileInfo( newName ).isDir() )
                return;
            for ( auto &&ii : fTransformResultMap )
            {
                if ( ii.first.startsWith( oldName ) )
                {
                    auto newKey = ii.first;
                    newKey.replace( oldName, newName );
                    fTransformResultMap[ newKey ] = ii.second;
                }
            }
        }

        void CTransformModel::updateFile( const QModelIndex &idx, const QString &oldFile, const QString &newFile )
        {
            CDirModel::updateFile( idx, oldFile, newFile );
            auto pos = fFileMapping.find( oldFile );
            if ( pos != fFileMapping.end() )
            {
                fFileMapping[ newFile ] = ( *pos ).second;
                fFileMapping.erase( pos );
            }

            auto pos2 = fTransformResultMap.find( oldFile );
            if ( pos2 != fTransformResultMap.end() )
            {
                fTransformResultMap[ newFile ] = ( *pos2 ).second;
                fTransformResultMap.erase( pos2 );
            }
            fDiskRipSearchMap.clear();
            computeEpisodesForDiskNumbers();
        }

        void CTransformModel::updateDir( const QModelIndex &idx, const QDir &oldDir, const QDir &newDir )
        {
            CDirModel::updateDir( idx, oldDir, newDir );
            auto pos = fDirMapping.find( oldDir.absolutePath() );
            if ( pos != fDirMapping.end() )
            {
                fDirMapping[ newDir.absolutePath() ] = ( *pos ).second;
                fDirMapping.erase( pos );
            }

            auto pos2 = fTransformResultMap.find( oldDir.absolutePath() );
            if ( pos2 != fTransformResultMap.end() )
            {
                fTransformResultMap[ newDir.absolutePath() ] = ( *pos2 ).second;
                fTransformResultMap.erase( pos2 );
            }
        }
    }
}
