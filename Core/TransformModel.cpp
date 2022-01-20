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
#include "TransformResult.h"
#include "SearchTMDBInfo.h"
#include "Preferences.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/FileCompare.h"

#include "SABUtils/DoubleProgressDlg.h"

#include <QDir>
#include <QTimer>
#include <QTreeView>
#include <QApplication>

#include <QDirIterator>

namespace NMediaManager
{
    namespace NCore
    {
        CTransformModel::CTransformModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
            fPatternTimer = new QTimer( this );
            fPatternTimer->setInterval( 50 );
            fPatternTimer->setSingleShot( true );
            connect( fPatternTimer, &QTimer::timeout, this, &CTransformModel::slotPatternChanged );
        }

        CTransformModel::~CTransformModel()
        {}

        bool CTransformModel::setData( const QModelIndex & idx, const QVariant & value, int role )
        {
            if ( role == Qt::CheckStateRole )
            {
                auto isTVShow = value.toInt() == Qt::Checked;
                auto baseItem = getPathItemFromIndex( idx );
                if ( baseItem && idx.column() == EColumns::eIsTVShow )
                    baseItem->setData( isTVShow, eIsTVShowRole );
                auto item = itemFromIndex( idx );
                // set Text calls updateItem, do not explicitly call it
                item->setText( isTVShow ? "Yes" : "No" ); 
                updateTransformPattern( item );
            }
            return CDirModel::setData( idx, value, role );
        }

        bool CTransformModel::isValidName( const QFileInfo & fi ) const
        {
            return isValidName( fi.fileName(), fi.isDir(), {} );
        }

        bool CTransformModel::isValidName( const QString & path, bool isDir, std::optional< bool > isChecked ) const
        {
            bool defaultAsTVShow = isChecked.has_value() ? isChecked.value() : this->isChecked( path, EColumns::eIsTVShow );
            bool asTVShow = treatAsTVShow( path, defaultAsTVShow );
            if ( (!asTVShow && fMoviePatterns.isValidName( path, isDir ))
                 || (asTVShow && fTVPatterns.isValidName( path, isDir )) )
                return true;

            return false;
        }

        void CTransformModel::slotTVOutputFilePatternChanged( const QString & outPattern )
        {
            fTVPatterns.setFilePattern( outPattern );
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CTransformModel::slotTVOutputDirPatternChanged( const QString & outPattern )
        {
            fTVPatterns.setDirPattern(outPattern);
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CTransformModel::slotMovieOutputFilePatternChanged( const QString & outPattern )
        {
            fMoviePatterns.setFilePattern(outPattern);
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CTransformModel::slotMovieOutputDirPatternChanged( const QString & outPattern )
        {
            fMoviePatterns.setDirPattern(outPattern);
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        std::pair< bool, QString > CTransformModel::transformItem( const QFileInfo & fileInfo ) const
        {
            if ( treatAsTVShow( fileInfo, isChecked( fileInfo, EColumns::eIsTVShow ) ) )
                return transformItem( fileInfo, fTVPatterns );
            else
                return transformItem( fileInfo, fMoviePatterns );
        }

        std::pair< bool, QString > CTransformModel::transformItem( const QFileInfo & fileInfo, const SPatternInfo & patternInfo ) const
        {
            auto filePath = fileInfo.absoluteFilePath();

            //if ( !info.fInPatternRegExp.isValid() )
            //    return std::make_pair( false, QObject::tr( "<INVALID INPUT REGEX>" ) );

            auto pos = fileInfo.isDir() ? fDirMapping.find( filePath ) : fFileMapping.find( filePath );
            auto retVal = std::make_pair( false, QString() );

            if ( pos == (fileInfo.isDir() ? fDirMapping.end() : fFileMapping.end()) )
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
                        retVal.second = QString();
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
                        retVal.second = transformInfo->transformedName( fileInfo, patternInfo, false );
                        retVal.first = true;
                    }
                }

                if ( fileInfo.isDir() )
                    fDirMapping[filePath] = retVal;
                else
                    fFileMapping[filePath] = retVal;
            }
            else
                retVal = (*pos).second;

            return retVal;
        }

        void CTransformModel::computeEpisodesForDiskNumbers()
        {
            TTitleMap titleMap;
            findTitlesPerDiskNumbers( index( 0, 0 ), titleMap );

            for ( auto && ii : titleMap )
            {
                NCore::SSearchTMDBInfo info( ii.first, {} );
                auto seasonNum = info.season();
                auto title = info.searchName();
                auto episodeNum = 1;
                for ( auto && jj : ii.second )
                {
                    for ( auto && kk : jj.second )
                    {
                        QString searchName;
                        if ( seasonNum > 0 )
                            searchName = QString( "S%2E%3" ).arg( seasonNum, 2, 10, QChar( '0' ) ).arg( episodeNum, 2, 10, QChar( '0' ) );
                        else
                            searchName = QString( "E%3" ).arg( episodeNum, 2, 10, QChar( '0' ) );
                        if ( !title.isEmpty() )
                            searchName = title + " - " + searchName;
                        //qDebug() << kk.second << " = " << searchName;
                        episodeNum++;
                        fDiskRipSearchMap[kk.second] = searchName;
                    }
                }
            }
        }

        void CTransformModel::findTitlesPerDiskNumbers( const QModelIndex & parentIdx, TTitleMap & retVal )
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
                    retVal[name][diskNumber] = childPaths;
                }
            }

            auto rowCount = this->rowCount( parentIdx );
            for ( int ii = 0; ii < rowCount; ++ii )
            {
                auto childIndex = index( ii, 0, parentIdx );
                findTitlesPerDiskNumbers( childIndex, retVal );
            }
        }

        std::map< int, QString > CTransformModel::getDiskTitles( const QModelIndex & parentIdx )
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
                    if ( NCore::SSearchTMDBInfo::isRippedFromMKV( name, &titleNum ) )
                    {
                        retVal[titleNum] = childIndex.data( NCore::ECustomRoles::eFullPathRole ).toString();
                    }
                }
            }
            return retVal;
        }

        QString CTransformModel::getSearchName( const QModelIndex & idx ) const
        {
            if ( !idx.isValid() )
                return QString();

            auto path = idx.data( ECustomRoles::eFullPathRole ).toString();
            auto pos = fDiskRipSearchMap.find( path );
            if ( pos != fDiskRipSearchMap.end() )
                return (*pos).second;

            if ( SSearchTMDBInfo::isRippedFromMKV( fileInfo( idx ) ) )
                return getSearchName( idx.parent() );

            auto nm = index( idx.row(), EColumns::eTransformName, idx.parent() ).data().toString();
            if ( isAutoSetText( nm ) || nm.isEmpty() )
            {
                nm = index( idx.row(), NCore::EColumns::eFSName, idx.parent() ).data( ECustomRoles::eFullPathRole ).toString();
                nm = nm.isEmpty() ? QString() : (QFileInfo( nm ).isDir() ? QFileInfo( nm ).fileName() : QFileInfo( nm ).completeBaseName());
            }

            return nm;
        }

        bool CTransformModel::treatAsTVShow( const QFileInfo & fileInfo, bool isTVByDefault ) const
        {
            EMediaType mediaType = isTVByDefault ? EMediaType::eTVShow : EMediaType::eMovie;
            auto transformInfo = getTransformResult( fileInfo, true );
            if ( transformInfo )
                mediaType = transformInfo->mediaType();
            return isTVType( mediaType );
        }

        void CTransformModel::setSearchResult( QStandardItem * item, std::shared_ptr< STransformResult > searchResult, bool applyToChildren )
        {
            auto idx = indexFromItem( item );
            setSearchResult( idx, searchResult, applyToChildren );
        }

        void CTransformModel::setDeleteItem( const QModelIndex & idx )
        {
            auto searchResult = std::make_shared< STransformResult >( EMediaType::eDeleteFileType );
            setSearchResult( idx, searchResult, false );
        }

        void CTransformModel::setSearchResult( const QModelIndex & idx, std::shared_ptr< STransformResult > searchResult, bool applyToChildren )
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
                    fTransformResultMap[fi.absoluteFilePath()] = searchResult;
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
                    auto childIdx = index( ii, NCore::EColumns::eFSName, idx );

                    // exception for "SRT" files that are of the form X_XXXX.ext dont transform
                    //auto txt = childIdx.data( ECustomRoles::eFullPathRole ).toString();
                    if ( !isSubtitleFile( childIdx ) )
                    {
                        auto childInfo = getTransformResult( childIdx, false );
                        if ( !childInfo )
                        {
                            setSearchResult( childIdx, searchResult, applyToChildren );
                        }
                    }
                }
            }
        }

        void CTransformModel::clearSearchResult( const QModelIndex & idx, bool recursive )
        {
            setSearchResult( idx, {}, recursive );
        }

        std::pair< bool, QStandardItem * > CTransformModel::processItem( const QStandardItem * item, QStandardItem * parentItem, bool displayOnly ) const
        {
            QStandardItem * myItem = nullptr;
            bool aOK = true;
            auto oldName = computeTransformPath( item, true );
            auto newName = computeTransformPath( item, false );

            if ( oldName != newName )
            {
                QFileInfo oldFileInfo( oldName );
                QFileInfo newFileInfo( newName );

                if ( newName == "<DELETE THIS>" )
                    myItem = new QStandardItem( QString( "Delete '%1'" ).arg( getDispName( oldName ) ) );
                else
                    myItem = new QStandardItem( QString( "'%1' => '%2'" ).arg( getDispName( oldName ) ).arg( getDispName( newName ) ) );


                myItem->setData( oldName, ECustomRoles::eOldName );
                myItem->setData( newName, ECustomRoles::eNewName );
                if ( parentItem )
                    parentItem->appendRow( myItem );
                else
                    fProcessResults.second->appendRow( myItem );

                if ( !displayOnly )
                {
                    bool removeIt = newName == "<DELETE THIS>";
                    if ( progressDlg() )
                    {
                        if ( removeIt )
                            progressDlg()->setLabelText( tr( "Removing '%1'" ).arg( getDispName( oldName ) ) );
                        else
                            progressDlg()->setLabelText( tr( "Renaming '%1' => '%2'" ).arg( getDispName( oldName ) ).arg( getDispName( newName ) ) );
                    }

                    bool dirAlreadyExisted = (oldFileInfo.exists() && oldFileInfo.isDir() && newFileInfo.exists() && newFileInfo.isDir());
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
                            if ( !QFile::rename( oldPath, newPath ) )
                            {
                                auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MOVE ITEM TO %2" ).arg( oldPath ).arg( newPath ) );
                                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                                appendError( myItem, errorItem );

                                QIcon icon;
                                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                errorItem->setIcon( icon );
                                allDeletedOK = false;
                            }
                        }
                        if ( allDeletedOK )
                        {
                            auto dir = QDir( oldName );
                            aOK = dir.removeRecursively();
                            if ( !aOK )
                            {
                                auto errorItem = new QStandardItem( QString( "ERROR: Failed to Remove '%1'" ).arg( oldName ) );
                                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                                appendError( myItem, errorItem );
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
                            if ( oldFileInfo.isDir() )
                            {
                                auto dir = QDir( oldName );
                                aOK = dir.removeRecursively();
                            }
                            else
                            {
                                aOK = QFile( oldFileInfo.absoluteFilePath() ).remove();
                            }
                            if ( !aOK )
                            {
                                auto errorItem = new QStandardItem( QString( "ERROR: Failed to Remove '%1'" ).arg( oldName ) );
                                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                                appendError( myItem, errorItem );
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
                                if ( (mySubPath.indexOf( "/" ) != -1) || (mySubPath.indexOf( "\\" ) != -1) )
                                {
                                    auto pos = mySubPath.lastIndexOf( QRegularExpression( "[\\/\\\\]" ) );
                                    auto myParentPath = mySubPath.left( pos );

                                    auto parentPath = computeTransformPath( item->parent(), false );
                                    parentPathOK = QDir( parentPath ).mkpath( myParentPath );
                                    if ( !parentPathOK )
                                    {
                                        auto errorItem = new QStandardItem( QString( "ERROR: '%1' => '%2' : FAILED TO MAKE PARENT DIRECTORY PATH" ).arg( oldName ).arg( newName ) );
                                        errorItem->setData( ECustomRoles::eIsErrorNode, true );
                                        appendError( myItem, errorItem );

                                        QIcon icon;
                                        icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                        errorItem->setIcon( icon );
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
                                if ( newFileInfo.exists() )
                                {
                                    if ( newFileInfo.isFile() && oldFileInfo.isFile() )
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
                                            errorMsg = QString( "Destination file Exists - Old Size: %1 New Size: %2" ).arg( NSABUtils::NFileUtils::fileSizeString( oldName, false ) ).arg( NSABUtils::NFileUtils::fileSizeString( newName, false ) );
                                        }
                                    }
                                    else
                                        aOK = true;
                                }
                                else
                                {
                                    auto fi = QFile( oldName );
                                    aOK = fi.rename( newName );
                                    if ( !aOK )
                                        errorMsg = fi.errorString();
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
                                auto errorItem = new QStandardItem( QString( "ERROR: '%1' => '%2' : FAILED TO RENAME - %3" ).arg( oldName ).arg( newName ).arg( errorMsg ) );
                                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                                appendError( myItem, errorItem );

                                QIcon icon;
                                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                errorItem->setIcon( icon );
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
                                    auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MODIFY TAGS: %2" ).arg( newName ).arg( msg ) );
                                    errorItem->setData( ECustomRoles::eIsErrorNode, true );
                                    appendError( myItem, errorItem );

                                    QIcon icon;
                                    icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                    errorItem->setIcon( icon );
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
                                        auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MODIFY TIMESTAMP" ).arg( newName ) );
                                        errorItem->setData( ECustomRoles::eIsErrorNode, true );
                                        appendError( myItem, errorItem );

                                        QIcon icon;
                                        icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                        errorItem->setIcon( icon );
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

        QStandardItem * CTransformModel::getTransformItem( const QStandardItem * item ) const
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

        void CTransformModel::clearResults()
        {
            fFileMapping.clear();
            fDirMapping.clear();
            fTransformResultMap.clear();
            fDiskRipSearchMap.clear();
        }

        void CTransformModel::slotPatternChanged()
        {
            clearResults();
            transformPatternChanged( invisibleRootItem() );
        }

        void CTransformModel::transformPatternChanged( const QStandardItem * item )
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

        void CTransformModel::updateTransformPattern( const QStandardItem * item )const
        {
            if ( item != invisibleRootItem() )
            {
                auto idx = item->index();

                auto baseIdx = (idx.column() == NCore::EColumns::eFSName) ? idx : idx.model()->index( idx.row(), NCore::EColumns::eFSName, idx.parent() );
                auto baseItem = this->itemFromIndex( baseIdx );

                auto transformedItem = getTransformItem( baseItem );
                updateTransformPattern( baseItem, transformedItem );
            }
        }

        void CTransformModel::updateTransformPattern( const QStandardItem * item, QStandardItem * transformedItem ) const
        {
            if ( !item || !transformedItem )
                return;

            auto nameItem = getItem( item, NCore::EColumns::eFSName );

            transformedItem->setBackground( QBrush() );

            auto path = item->data( ECustomRoles::eFullPathRole ).toString();
            auto fileInfo = QFileInfo( path );

            if ( isIgnoredPathName( fileInfo ) && !CPreferences::instance()->isSubtitleFile( fileInfo ) )
                return;

            auto transformInfo = transformItem( fileInfo );
            auto currText = item->text();
            if ( !transformInfo.first || (currText != transformInfo.second) )
            {
                if ( transformedItem->text() != transformInfo.second )
                    transformedItem->setText( transformInfo.second );

                //auto isTVShow = treatAsTVShow( fileInfo, this->isChecked( path, EColumns::eIsTVShow ) );
                //if ( canAutoSearch( fileInfo ) && (CTransformModel::isAutoSetText( transformInfo.second ) || (!isValidName( transformInfo.second, fileInfo.isDir(), isTVShow ) && !isValidName( fileInfo ))) )
                //    transformedItem->setBackground( Qt::red );
            }
        }

        void CTransformModel::postAddItems( const QFileInfo & fileInfo, std::list< NMediaManager::NCore::SDirNodeItem > & currItems ) const
        {
            bool isTVShow = isTVType( SSearchTMDBInfo::looksLikeTVShow( fileInfo.fileName(), nullptr ) );

            currItems.front().setData( isTVShow, ECustomRoles::eIsTVShowRole );
        }

        void CTransformModel::setupNewItem( const SDirNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item ) const
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

        std::list< NMediaManager::NCore::SDirNodeItem > CTransformModel::addAdditionalItems( const QFileInfo & fileInfo ) const
        {
            std::list< NMediaManager::NCore::SDirNodeItem > retVal;

            auto mediaType = SSearchTMDBInfo::looksLikeTVShow( fileInfo.fileName(), nullptr );
            auto isTVShowItem = SDirNodeItem( QString(), EColumns::eIsTVShow );
            isTVShowItem.fMediaType = mediaType;
            isTVShowItem.fCheckable = true;

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
            return CDirModel::headers()
                << tr( "Is TV Show?" )
                << tr( "New Name" )
                << getMediaHeaders()
                ;
        }

        void CTransformModel::postLoad( QTreeView * treeView ) const
        {
            if ( !treeView )
                return;
        }

        void CTransformModel::attachTreeNodes( QStandardItem * nextParent, QStandardItem *& prevParent, const STreeNode & ii )
        {
            bool isTVShow = nextParent->data( eIsTVShowRole ).toBool();
            bool isParentTVShow = prevParent->data( eIsTVShowRole ).toBool();

            if ( !isTVShow && (isTVShow != isParentTVShow) )
            {
                setChecked( ii.item( static_cast< NCore::EColumns >( EColumns::eIsTVShow ) ), isParentTVShow );
                nextParent->setData( isParentTVShow, eIsTVShowRole );
            }
        }

        int CTransformModel::computeNumberOfItems() const
        {
            return NSABUtils::itemCount( fProcessResults.second.get(), true );
        }

        void CTransformModel::postReloadModel()
        {
            fPatternTimer->stop(); // if its runinng when this timer stops its realoaded anyway
        }

        QString CTransformModel::getMyTransformedName( const QStandardItem * item, bool transformParentsOnly ) const
        {
            auto transformItem = transformParentsOnly ? nullptr : getTransformItem( item );
            auto myName = transformItem ? transformItem->text() : QString();
            if ( myName.isEmpty() || isAutoSetText( myName ) )
            {
                myName = item->text();
            }
            return myName;
        }

        bool CTransformModel::preFileFunction( const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/ )
        {
            return true;
        }

        void CTransformModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/ )
        {

        }

        bool CTransformModel::setMediaTags( const QString & fileName, std::shared_ptr< STransformResult > & searchResults, QString & msg ) const
        {
            QString year;
            QString title = QFileInfo( fileName ).completeBaseName();
            if ( searchResults )
            {
                year = searchResults->getYear();
                title = searchResults->transformedName( fileName, searchResults->isTVShow() ? fTVPatterns : fMoviePatterns, true );
            }
            return CDirModel::setMediaTags( fileName, title, year, &msg );
        }

        bool CTransformModel::canAutoSearch( const QModelIndex & index ) const
        {
            auto path = index.data( ECustomRoles::eFullPathRole ).toString();
            if ( path.isEmpty() )
                return false;
            return canAutoSearch( QFileInfo( path ) );
        }

        bool CTransformModel::canAutoSearch( const QFileInfo & fileInfo ) const
        {
            if ( CPreferences::instance()->isIgnoredPath( fileInfo ) || CPreferences::instance()->isSkippedPath( fileInfo ) )
                return false;

            bool isLangFormat;
            if ( CPreferences::instance()->isSubtitleFile( fileInfo, &isLangFormat ) && !isLangFormat )
                return false;

            if ( !fileInfo.isDir() )
                return true;

            auto files = QDir( fileInfo.absoluteFilePath() ).entryInfoList( QDir::Files );
            bool hasFiles = false;
            for ( auto && ii : files )
            {
                if ( canAutoSearch( ii ) )
                {
                    hasFiles = true;
                    break;
                }
            }

            return hasFiles;
        }

        std::shared_ptr< STransformResult > CTransformModel::getTransformResult( const QModelIndex & idx, bool checkParents ) const
        {
            if ( !idx.isValid() )
                return {};

            auto fi = fileInfo( idx );
            return getTransformResult( fi, checkParents );
        }

        std::shared_ptr< STransformResult > CTransformModel::getTransformResult( const QFileInfo & fi, bool checkParents ) const
        {
            auto path = fi.absoluteFilePath();
            if ( path.isEmpty() )
                return {};

            auto pos = fTransformResultMap.find( path );
            if ( pos != fTransformResultMap.end() )
                return (*pos).second;

            if ( !checkParents )
                return {};

            if ( isRootPath( fi.absoluteFilePath() ) )
                 return {};

            auto regExStr = "^([A-Z]\\:(\\\\|\\/)|(\\/))$";
            auto regEx = QRegularExpression( regExStr, QRegularExpression::CaseInsensitiveOption );
            auto match = regEx.match( path );
            if ( match.hasMatch() )
                return {};

            return getTransformResult( fi.absolutePath(), true );
        }

        std::shared_ptr< STransformResult > CTransformModel::getTransformResult( const QString & path, bool checkParents ) const
        {
            return getTransformResult( QFileInfo( path ), checkParents );
        }

    }
}
