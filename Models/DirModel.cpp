// The MIT License( MIT )
//
// Copyright( c ) 2020-2023 Scott Aron Bloom
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
#include "Core/TransformResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Preferences/Core/Preferences.h"

#include "UI/ProcessConfirm.h"
#include "UI/BasePage.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/BackupFile.h"
#include "SABUtils/FileCompare.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/MKVUtils.h"
#include "SABUtils/ForceUnbufferedProcessModifier.h"

#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/utils.h"

#include <QDebug>
#include <QUrl>
#include <QInputDialog>
#include <QTextStream>
#include <QCollator>
#include <QDir>
#include <QFileIconProvider>
#include <QTimer>
#include <QTreeView>
#include <QApplication>
#include <QBrush>
#include <QDirIterator>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QThreadPool>

#include <QProcess>

#include <set>
#include <list>

QDebug operator<<( QDebug dbg, const NMediaManager::NModels::STreeNode &node )
{
    dbg << "STreeNode(";
    auto name = node.text();
    dbg << "'" << name << "' ";
    dbg << "Path: " << QFileInfo().absoluteFilePath() << " "
        << "Is Loaded? " << node.fLoaded << " "
        << "Is File? " << node.fIsFile << ")";
    return dbg;
}

QDebug operator<<( QDebug dbg, const NMediaManager::NModels::TParentTree &parentTree )
{
    dbg << " TParentTree( ";
    bool first = true;
    for ( auto &&ii : parentTree )
    {
        if ( !first )
            dbg << ", ";
        first = false;
        dbg << ii;
    }
    dbg << ")";
    return dbg;
}

namespace NMediaManager
{
    namespace NModels
    {
        bool useStatusCache()
        {
#ifdef _DEBUG
//#define DISABLE_CACHE
#endif

            static bool sUseCache
#if defined( DISABLE_CACHE ) && defined( _DEBUG )
                = false;
#else
                = true;
#endif
            return sUseCache;
        }

        CDirModelItem::CDirModelItem( const QString &text, EType type ) :
            QStandardItem( text ),
            fType( type )
        {
            setEditable( true );
        }

        std::optional< QString > CDirModelItem::getCompareValue() const
        {
            const int role = model() ? model()->sortRole() : Qt::DisplayRole;
            const auto &&lData = data( role );
            if ( lData.type() == QVariant::Type::String )
            {
                auto value = lData.toString();
                auto dirs = value.split( QRegularExpression( R"([\/\\])" ) );
                QStringList retVal;
                for ( auto &&curr : dirs )
                {
                    QString number;
                    QString extra;
                    bool numIsPrefix;
                    if ( NSABUtils::NStringUtils::startsOrEndsWithNumber( curr, &number, &extra, &numIsPrefix ) )
                    {
                        if ( numIsPrefix )
                            retVal << QString( "%1_%2" ).arg( number, 4, QChar( '0' ) ).arg( extra );
                        else
                            retVal << QString( "%1_%2" ).arg( extra ).arg( number, 4, QChar( '0' ) );
                    }
                    else
                        retVal << curr;
                }
                return retVal.join( "/" );
            }
            return {};
        }

        bool CDirModelItem::operator<( const QStandardItem &rhs ) const
        {
            auto rhsItem = dynamic_cast< const CDirModelItem * >( &rhs );
            if ( column() == 0 && rhsItem )
            {
                auto lhsValue = getCompareValue();
                auto rhsValue = rhsItem->getCompareValue();
                if ( lhsValue.has_value() && rhsValue.has_value() )
                {
                    return lhsValue.value() < rhsValue.value();
                }
            }
            return QStandardItem::operator<( rhs );
        }

        CDirModel::CDirModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            QStandardItemModel( parent ),
            fBasePage( page )
        {
            fIconProvider = new CIconProvider();

            fReloadTimer = new QTimer( this );
            fReloadTimer->setInterval( 50 );
            fReloadTimer->setSingleShot( true );
            connect( fReloadTimer, &QTimer::timeout, this, &CDirModel::slotLoadRootDirectory );

            fProcess = new QProcess( this );
            connect( fProcess, &QProcess::errorOccurred, this, &CDirModel::slotProcessErrorOccured );
            connect( fProcess, qOverload< int, QProcess::ExitStatus >( &QProcess::finished ), this, &CDirModel::slotProcessFinished );
            connect( fProcess, &QProcess::readyReadStandardError, this, &CDirModel::slotProcessStandardError );
            connect( fProcess, &QProcess::readyReadStandardOutput, this, &CDirModel::slotProcessStandardOutput );
            connect( fProcess, &QProcess::started, this, &CDirModel::slotProcessStarted );
            connect( fProcess, &QProcess::stateChanged, this, &CDirModel::slotProcesssStateChanged );

            connect( this, &QStandardItemModel::dataChanged, this, &CDirModel::slotDataChanged );

            connect( NPreferences::NCore::CPreferences::instance(), &NPreferences::NCore::CPreferences::sigMediaInfoLoaded, this, &CDirModel::slotUpdateMediaInfo );
        }

        CDirModel::~CDirModel()
        {
            delete fIconProvider;
        }

        void CDirModel::setRootPath( const QString &rootPath )
        {
            fIsRootPathCache.clear();
            fDispNameCache.clear();
            fRootPath = QDir( rootPath );
            reloadModel();
        }

        void CDirModel::reloadModel()
        {
            fReloadTimer->stop();
            fReloadTimer->start();
            postReloadModelRequest();
        }

        bool CDirModel::isTVShow( const QModelIndex &idx ) const
        {
            if ( !idx.isValid() )
                return false;

            auto fi = this->fileInfo( idx );
            return isTVType( NCore::SSearchTMDBInfo::looksLikeTVShow( fi.fileName(), nullptr ) );
        }

        void CDirModel::slotLoadRootDirectory()
        {
            NSABUtils::CAutoWaitCursor awc;

            preLoad();

            //qDebug() << fRootPath;
            QFileInfo rootFI( fRootPath.path() );

            if ( progressDlg() )
            {
                progressDlg()->setLabelText( tr( "Computing number of Files under '%1'" ).arg( fRootPath.absolutePath() ) );
                qApp->processEvents();
                auto numFiles = computeNumberOfFiles( rootFI ).second;
                if ( !progressCanceled() )
                {
                    progressDlg()->setValue( 0 );
                    progressDlg()->setRange( 0, numFiles );
                }
            }
            if ( progressCanceled() )
            {
                postLoad( false );
                return;
            }

            loadFileInfo( rootFI );

            postLoad( true );
        }

        void CDirModel::preLoad()
        {
            setIsLoading( true );
            clear();
            setHorizontalHeaderLabels( headers() );
            preLoad( filesView() );
        }

        void CDirModel::postLoad( bool /*aOK*/ )
        {
            setIsLoading( false );

            if ( !filesView() )
                return;

            emit sigDirLoadFinished( progressCanceled() );
            postLoad( filesView() );
        }

        void CDirModel::postLoad( QTreeView *treeView )
        {
            resizeColumns();
            (void)treeView;   //treeView->expandAll();
        }

        void CDirModel::preLoad( QTreeView * /*treeView*/ )
        {
            resetStatusCaches();
            fPathMapping.clear();
        }

        void CDirModel::postReloadModelRequest()
        {
        }

        int CDirModel::computeNumberOfItems() const
        {
            return NSABUtils::itemCount( fProcessResults.second.get(), true, getExcludeFuncForItemCount() );
        }

        std::pair< std::function< bool( const QVariant &value ) >, int > CDirModel::getExcludeFuncForItemCount() const
        {
            return { {}, Qt::DisplayRole };
        }

        bool CDirModel::setData( const QModelIndex &idx, const QVariant &value, int role )
        {
            auto item = itemFromIndex( idx );
            if ( role == Qt::CheckStateRole )
            {
                QStandardItemModel::setData( idx, value, role );
                setCheckState( item, static_cast< Qt::CheckState >( value.toInt() ), true );
            }
            else if ( role == Qt::EditRole )
            {
                if ( !item )
                    return false;

                switch ( static_cast< EType >( item->type() ) )
                {
                    case EType::ePath:
                        {
                            auto fi = fileInfo( idx );
                            if ( value.toString().isEmpty() )
                            {
                                return false;
                            }
                            auto currName = fi.absoluteFilePath();
                            auto newName = fi.absoluteDir().absoluteFilePath( value.toString().trimmed() );
                            if ( currName.compare( newName, Qt::CaseInsensitive ) == 0 )
                                return false;

                            auto file = QFile( currName );
                            if ( !file.rename( newName ) )
                            {
                                QMessageBox::critical( nullptr, tr( "Could not rename file" ), tr( "Error renaming '%1' to '%2' please verify permissions. %3" ).arg( currName ).arg( newName ).arg( file.errorString() ) );
                                return false;
                            }

                            updatePath( idx, currName, newName );
                        }
                        break;
                    case EType::eMediaTag:
                        {
                            auto mediaTagType = static_cast< NSABUtils::EMediaTags >( item->data( NModels::ECustomRoles::eMediaTagTypeRole ).toInt() );
                            if ( !setMediaTag( fileInfo( idx ).absoluteFilePath(), { mediaTagType, value.toString() } ) )
                                return false;
                        }
                        break;
                }
            }

            return QStandardItemModel::setData( idx, value, role );
        }

        std::unique_ptr< QDirIterator > CDirModel::getDirIteratorForPath( const QFileInfo &fileInfo ) const
        {
            auto filter = dirModelFilter();
            auto tmp = filter;
            tmp.sort();
            auto retVal = std::make_unique< QDirIterator >( fileInfo.absoluteFilePath(), filter, QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable );
            return std::move( retVal );
        }

        void CDirModel::iterateEveryFile( const QFileInfo &fileInfo, const SIterateInfo &iterInfo, std::optional< QDateTime > &lastUpdateUI, bool countOnly ) const
        {
            //qDebug() << fileInfo;
            if ( progressCanceled() )
                return;

            if ( fileInfo.isDir() )
            {
                bool iteraterDir = true;
                if ( iterInfo.fPreDirFunction )
                    iteraterDir = iterInfo.fPreDirFunction( fileInfo );

                if ( iteraterDir )
                {
                    auto ii = getDirIteratorForPath( fileInfo );
                    if ( !ii )
                    {
                        if ( iterInfo.fPostDirFunction )
                            iterInfo.fPostDirFunction( fileInfo, countOnly );
                        return;
                    }

                    while ( ii->hasNext() )
                    {
                        ii->next();
                        auto fi = ii->fileInfo();
                        iterateEveryFile( fi, iterInfo, lastUpdateUI, countOnly );
                        if ( progressDlg() && ( !lastUpdateUI.has_value() || ( lastUpdateUI.value().msecsTo( QDateTime::currentDateTime() ) > 250 ) ) )
                        {
                            qApp->processEvents();
                            lastUpdateUI = QDateTime::currentDateTime();
                        }
                    }
                    qApp->processEvents();
                }

                if ( iterInfo.fPostDirFunction )
                    iterInfo.fPostDirFunction( fileInfo, true );
            }
            else if ( fileInfo.isFile() )
            {
                bool aOK = true;
                if ( iterInfo.fPreFileFunction )
                    aOK = iterInfo.fPreFileFunction( fileInfo );
                if ( iterInfo.fPostFileFunction )
                    iterInfo.fPostFileFunction( fileInfo, aOK );
            }
        }

        std::pair< uint64_t, uint64_t > CDirModel::computeNumberOfFiles( const QFileInfo &fileInfo )
        {
            uint64_t numDirs = 0;
            uint64_t numFiles = 0;
            SIterateInfo info;
            info.fPreDirFunction = [ this, &numDirs ]( const QFileInfo &dirInfo )
            {
                if ( isSkippedPathName( dirInfo ) )
                    return false;
                numDirs++;

                if ( progressDlg() )
                {
                    progressDlg()->setPrimaryValue( numDirs );

                    int childDirs = 0;

                    auto iter = QDirIterator(
                        dirInfo.absoluteFilePath(),
                        QStringList() << "*"
                                      << "*.*",
                        QDir::AllDirs | QDir::NoDotAndDotDot | QDir::Readable );
                    int currMax = progressDlg()->primaryMax();
                    while ( iter.hasNext() )
                    {
                        progressDlg()->setPrimaryMaximum( ++currMax );
                        iter.next();
                    }
                }
                return true;
            };

            TParentTree tree;
            std::unordered_set< QString > alreadyAdded;
            info.fPreFileFunction = [ this, &tree, &alreadyAdded, &numFiles ]( const QFileInfo &fileInfo )
            {
                if ( isSkippedPathName( fileInfo ) )
                    return false;

                // need to be children of file
                auto attachFile = preFileFunction( fileInfo, alreadyAdded, tree, true );
                if ( attachFile )
                    numFiles++;
                return false;
            };
            std::optional< QDateTime > lastUpdate;
            iterateEveryFile( fileInfo, info, lastUpdate, true );

            return std::make_pair( numDirs, numFiles );
        }

        void CDirModel::loadFileInfo( const QFileInfo &fileInfo )
        {
            if ( !fileInfo.exists() )
                return;

            TParentTree tree;

            SIterateInfo info;
            info.fPreDirFunction = [ this, &tree ]( const QFileInfo &dirInfo )
            {
                //qDebug().noquote().nospace() << "Pre Directory A: " << dirInfo.absoluteFilePath() << tree;

                if ( !preDirFunction( dirInfo, false ) )
                    return false;

                auto row = getItemRow( dirInfo );
                tree.push_back( std::move( row ) );

                if ( progressDlg() )
                    progressDlg()->setLabelText( tr( "Searching Directory '%1'" ).arg( QDir( fRootPath ).relativeFilePath( dirInfo.absoluteFilePath() ) ) );

                if ( isSkippedPathName( dirInfo ) )
                {
                    //qDebug().noquote().nospace() << "Pre Directory B: returning false" << dirInfo.absoluteFilePath() << tree;
                    return false;
                }

                //qDebug().noquote().nospace() << "Pre Directory B: returning true" << dirInfo.absoluteFilePath() << tree;
                return true;
            };

            std::unordered_set< QString > alreadyAdded;
            info.fPreFileFunction = [ this, &tree, &alreadyAdded ]( const QFileInfo &fileInfo )
            {
                tree.push_back( std::move( getItemRow( fileInfo ) ) );   // mkv file

                if ( isSkippedPathName( fileInfo ) )
                    return false;

                // need to be children of file
                auto attachFile = preFileFunction( fileInfo, alreadyAdded, tree, false );

                //qDebug().noquote().nospace() << "Pre File A: " << fileInfo.absoluteFilePath() << tree;

                if ( attachFile )
                    attachTreeNodes( tree );

                if ( progressDlg() )
                    progressDlg()->setValue( progressDlg()->value() + 1 );

                //qDebug().noquote().nospace() << "Pre File B: " << fileInfo.absoluteFilePath() << tree;

                return true;
            };

            info.fPostDirFunction = [ this, &tree ]( const QFileInfo &dirInfo, bool aOK )
            {
                postDirFunction( aOK, dirInfo, tree, false );
                //qDebug().noquote().nospace() << "Post Dir A: " << dirInfo.absoluteFilePath() << tree << "AOK? " << aOK;
                tree.pop_back();
                //qDebug().noquote().nospace() << "Post Dir B: " << dirInfo.absoluteFilePath() << tree;
            };

            info.fPostFileFunction = [ this, &tree ]( const QFileInfo &fileInfo, bool aOK )
            {
                //qDebug() << fileInfo.absoluteFilePath();
                postFileFunction( aOK, fileInfo, tree, false );

                //qDebug().noquote().nospace() << "Post File A: " << fileInfo.absoluteFilePath() << tree << "AOK? " << aOK;
                while ( tree.back().fIsFile )
                    tree.pop_back();
                //qDebug().noquote().nospace() << "Post File B: " << dirInfo.absoluteFilePath() << tree;
            };

            std::optional< QDateTime > lastUpdate;
            iterateEveryFile( fileInfo, info, lastUpdate, false );
        }

        void CDirModel::appendRow( QStandardItem *parent, QList< QStandardItem * > &items )
        {
            if ( parent )
                parent->appendRow( items );
            else
                QStandardItemModel::appendRow( items );
            auto path = items.front()->data( ECustomRoles::eAbsFilePath ).toString();
            auto pos = fMessagesForFiles.find( path );
            if ( pos != fMessagesForFiles.end() )
            {
                auto &&msgs = ( *pos ).second;
                for ( auto &&ii : msgs )
                {
                    auto msgItem = new QStandardItem( ii );
                    items.front()->appendRow( msgItem );
                    fMsgItems.push_back( msgItem );
                }
            }
        }

        QStandardItem *CDirModel::attachTreeNodes( TParentTree &parentTree )
        {
            QStandardItem *prevParent = nullptr;
            for ( auto &&ii : parentTree )
            {
                auto nextParent = ii.rootItem();
                if ( !ii.fLoaded )   // already been loaded
                {
                    if ( prevParent )
                    {
                        attachTreeNodes( nextParent, prevParent, ii );
                        appendRow( prevParent, ii.items() );
                    }
                    else
                    {
                        appendRow( nullptr, ii.items() );
                        nextParent->setData( true, ECustomRoles::eIsRoot );
                    }
                    ii.fLoaded = true;
                }
                prevParent = nextParent;
                fPathMapping[ nextParent->data( ECustomRoles::eAbsFilePath ).toString() ] = nextParent;

                if ( filesView() && prevParent )
                    filesView()->setExpanded( prevParent->index(), true );
            }
            return prevParent;
        }

        bool CDirModel::isSkippedPathName( const QFileInfo &fi, bool allowIgnore ) const
        {
            if ( allowIgnore && NPreferences::NCore::CPreferences::instance()->getIgnorePathNamesToSkip( ignoreExtrasOnSearch() ) )
                return false;

            return NPreferences::NCore::CPreferences::instance()->isSkippedPath( ignoreExtrasOnSearch(), fi );
        }

        bool CDirModel::isIgnoredPathName( const QFileInfo &fileInfo, bool allowIgnore ) const
        {
            if ( allowIgnore && NPreferences::NCore::CPreferences::instance()->getIgnorePathNamesToIgnore() )
                return false;

            return NPreferences::NCore::CPreferences::instance()->isIgnoredPath( fileInfo );
        }

        STreeNode CDirModel::getItemRow( const QFileInfo &fileInfo ) const
        {
            return STreeNode( fileInfo, this, isRootPath( fileInfo ) );
        }

        STreeNode::STreeNode( const QFileInfo &fileInfo, const CDirModel *model, bool isRoot ) :
            fFileInfo( fileInfo ),
            fModel( model )
        {
            fIsFile = fileInfo.isFile();
            //qDebug() << fileInfo.absoluteFilePath() << isRoot;
            QString name;

            auto path = QDir::toNativeSeparators( fileInfo.absoluteFilePath() );
            if ( !NSABUtils::NFileUtils::isIPAddressNetworkPath( fileInfo ) )
                path = QDir::toNativeSeparators( fileInfo.canonicalFilePath() );

            auto nameItem = SDirNodeItem( isRoot ? path : fModel->getTreeNodeName( fileInfo ), EColumns::eFSName );
            nameItem.fIcon = model->iconProvider()->icon( fileInfo );
            nameItem.setData( fileInfo.absoluteFilePath(), ECustomRoles::eAbsFilePath );
            nameItem.setData( fileInfo.isDir(), ECustomRoles::eIsDir );
            nameItem.fEditable = std::make_pair( EType::ePath, static_cast< NSABUtils::EMediaTags >( -1 ) );
            nameItem.fCheckable = { true, false, Qt::CheckState::Checked };
            fItems.push_back( nameItem );
            fItems.emplace_back( fileInfo.isFile() ? NSABUtils::NFileUtils::byteSizeString( fileInfo ) : QString(), EColumns::eFSSize );
            if ( fileInfo.isFile() )
            {
                fItems.back().fAlignment = Qt::AlignRight | Qt::AlignVCenter;
            }
            fItems.emplace_back( model->iconProvider()->type( fileInfo ), EColumns::eFSType );
            fItems.emplace_back( fileInfo.lastModified().toString( "MM/dd/yyyy hh:mm:ss.zzz" ), EColumns::eFSModDate );

            auto modelItems = model->addAdditionalItems( fileInfo );
            fItems.insert( fItems.end(), modelItems.begin(), modelItems.end() );
            model->postAddItems( fileInfo, fItems );
        }

        QString STreeNode::text() const
        {
            return rootItem() ? rootItem()->text() : NCore::CTransformResult::getNoItems();
        }

        QString STreeNode::fullPath() const
        {
            return fFileInfo.absoluteFilePath();
        }

        QStandardItem *STreeNode::item( EColumns column, bool createIfNecessary /*= true */ ) const
        {
            items( createIfNecessary );
            return ( column >= fRealItems.count() ) ? nullptr : fRealItems[ int( column ) ];
        }

        QStandardItem *STreeNode::rootItem( bool createIfNecessary /*= true */ ) const
        {
            return item( static_cast< EColumns >( 0 ), createIfNecessary );
        }

        QList< QStandardItem * > STreeNode::items( bool createIfNecessary ) const
        {
            if ( createIfNecessary && fRealItems.isEmpty() )
            {
                for ( auto &&ii : fItems )
                {
                    auto currItem = ii.createStandardItem();

                    auto nameItem = fRealItems.isEmpty() ? nullptr : fRealItems.front();

                    fModel->setupNewItem( ii, nameItem, currItem );

                    fRealItems << currItem;
                }
            }
            return fRealItems;
        }

        void STreeNode::updateName( const QDir &parentDir )
        {
            auto name = parentDir.relativeFilePath( fFileInfo.absoluteFilePath() );
            fItems.front().fText = name;
        }

        void CDirModel::setCheckState( QStandardItem *item, Qt::CheckState state, bool adjustParents ) const
        {
            item->setCheckState( state );
            if ( item->column() != 0 )
                return;

            for ( auto ii = 0; ii < item->rowCount(); ++ii )
            {
                auto curr = item->child( ii );
                if ( !curr )
                    continue;
                curr->setCheckState( state );
                setCheckState( curr, state, false );
            }

            if ( !adjustParents )
                return;

            updateParentCheckState( item );
        }

        void CDirModel::updateParentCheckState( QStandardItem *item ) const
        {
            if ( !item )
                return;
            auto parent = item->parent();
            if ( !parent || ( parent == invisibleRootItem() ) )
                return;

            auto childState = parent->child( 0 )->checkState();
            for ( auto ii = 1; ii < parent->rowCount(); ++ii )
            {
                auto curr = parent->child( ii );
                if ( !curr )
                    continue;
                //qDebug() << curr->text() << curr->checkState();
                if ( childState != curr->checkState() )
                {
                    childState = Qt::PartiallyChecked;
                    break;
                }
            }
            parent->setCheckState( childState );
            updateParentCheckState( parent );
        }

        void CDirModel::setChecked( QStandardItem *item, bool isChecked ) const
        {
            if ( !item )
                return;

            setCheckState( item, isChecked ? Qt::Checked : Qt::Unchecked, true );
        }

        void CDirModel::setChecked( QStandardItem *item, ECustomRoles role, bool isChecked ) const
        {
            setChecked( item, isChecked );
            item->setData( isChecked, role );
        }

        QStandardItem *CDirModel::getItemFromPath( const QFileInfo &fi ) const
        {
            auto path = fi.absoluteFilePath();
            auto pos = fPathMapping.find( path );
            if ( pos != fPathMapping.end() )
                return ( *pos ).second;
            return nullptr;
        }

        QVariant CDirModel::data( const QModelIndex &idx, int role ) const
        {
            if ( role == Qt::DecorationRole )
            {
                auto basePixmap = QStandardItemModel::data( idx, role );
                auto pixmap = getPathDecoration( idx, basePixmap );
                return pixmap;
            }
            else if ( role == Qt::BackgroundRole )
            {
                auto clr = getItemBackground( idx );
                if ( !clr.isNull() )
                    return clr;
            }
            else if ( role == Qt::ForegroundRole )
            {
                auto clr = getItemForeground( idx );
                if ( !clr.isNull() )
                    return clr;
            }
            else if ( role == Qt::ToolTipRole )
            {
                auto text = getToolTip( idx );
                if ( !text.isNull() )
                    return text;
            }
            else if ( role == Qt::DisplayRole )
            {
                auto item = getPathItemFromIndex( idx );
                if ( item && item->data( ECustomRoles::eYesNoCheckableOnly ).toBool() )
                    return ( item->checkState() == Qt::CheckState::Checked ) ? "Yes" : "No";
            }
            else if ( role == ECustomRoles::eIsSeasonDirRole )
            {
                return isSeasonDir( idx );
            }
            else if ( role == ECustomRoles::eIsSeasonDirCorrectRole )
            {
                bool isNameOK = false;
                bool aOK = isSeasonDir( idx, &isNameOK );
                return aOK && isNameOK;
            }

            return QStandardItemModel::data( idx, role );
        }

        QFileInfo CDirModel::fileInfo( const QStandardItem *item ) const
        {
            if ( !item )
                return {};
            auto baseItem = getItem( item, NModels::EColumns::eFSName );
            if ( !baseItem )
                return QFileInfo();
            auto path = baseItem->data( ECustomRoles::eAbsFilePath ).toString();
            return QFileInfo( path );
        }

        QStandardItem *CDirModel::getPathItemFromIndex( const QModelIndex &idx ) const
        {
            return itemFromIndex( idx );
        }

        bool CDirModel::isDir( const QStandardItem *item ) const
        {
            auto baseItem = getItem( item, NModels::EColumns::eFSName );
            if ( !baseItem )
                return false;

            return baseItem->data( ECustomRoles::eIsDir ).toBool();
        }

        QString CDirModel::filePath( const QStandardItem *item ) const
        {
            auto baseItem = getItem( item, NModels::EColumns::eFSName );
            if ( !baseItem )
                return {};

            return baseItem->data( ECustomRoles::eAbsFilePath ).toString();
        }

        QString CDirModel::filePath( const QModelIndex &idx ) const
        {
            auto item = getPathItemFromIndex( idx );
            return filePath( item );
        }

        QUrl CDirModel::url( const QModelIndex &idx ) const
        {
            if ( isDir( idx ) )
            {
                auto name = fileInfo( idx ).fileName();
                auto regExpStr = R"(\[\s*(tmdbid=(?<tmdbid>\d+)|(imdbid=<?<imdbid>tt.*))\s*\])";
                auto regExp = QRegularExpression( regExpStr, QRegularExpression::CaseInsensitiveOption );
                auto match = regExp.match( name );
                if ( match.hasMatch() )
                {
                    QString urlPath;
                    auto imdbid = match.captured( "imdbid" );
                    auto tmdbid = match.captured( "tmdbid" );
                    if ( !imdbid.isEmpty() )
                        urlPath = QString( "https://imdb.com/title/%1" ).arg( imdbid );
                    else if ( !tmdbid.isEmpty() )
                        urlPath = QString( "https://themoviedb.org/%1/%2" ).arg( this->isTVShow( idx ) ? "tv" : "movie" ).arg( tmdbid );

                    if ( !urlPath.isEmpty() )
                        return QUrl( urlPath );
                }
                return {};
            }
            auto parent = idx.parent();
            if ( parent.isValid() )
                return url( parent );
            return {};
        }

        QFileInfo CDirModel::fileInfo( const QModelIndex &idx ) const
        {
            auto item = getPathItemFromIndex( idx );
            return fileInfo( item );
        }

        bool CDirModel::isDir( const QModelIndex &idx ) const
        {
            auto item = getPathItemFromIndex( idx );
            return isDir( item );
        }

        bool CDirModel::isMediaFile( const QStandardItem *item ) const
        {
            return isMediaFile( fileInfo( item ) );
        }

        bool CDirModel::isMediaFile( const QModelIndex &idx ) const
        {
            return isMediaFile( fileInfo( idx ) );
        }

        bool CDirModel::isMediaFile( const QFileInfo &fileInfo ) const
        {
            //qDebug() << fileInfo;
            return fileInfo.isFile() && NPreferences::NCore::CPreferences::instance()->isMediaFile( fileInfo );
        }

        bool CDirModel::isSubtitleFile( const QFileInfo &fileInfo, bool *isLangFileFormat ) const
        {
            return NPreferences::NCore::CPreferences::instance()->isSubtitleFile( fileInfo, isLangFileFormat );
        }

        bool CDirModel::isSubtitleFile( const QModelIndex &idx, bool *isLangFileFormat ) const
        {
            return isSubtitleFile( fileInfo( idx ), isLangFileFormat );
        }

        bool CDirModel::isSubtitleFile( const QStandardItem *item, bool *isLangFileFormat /*= nullptr */ ) const
        {
            return isSubtitleFile( fileInfo( item ), isLangFileFormat );
        }

        QString CDirModel::getMyTransformedName( const QStandardItem *item, bool /*transformParentsOnly*/ ) const
        {
            return item->text();
        }

        QString CDirModel::computeMergedPath( const QString &parentDir, const QString &myName ) const
        {
            auto mySplit = myName.split( QRegularExpression( R"(\\|/)" ) );
            auto leafName = mySplit.back();
            mySplit.pop_back();
            auto myDir = mySplit.join( "/" );
            if ( myDir.endsWith( "/" ) )
                myDir = myDir.left( myDir.length() - 1 );

            auto parentPath = parentDir;
            parentPath = parentPath.replace( "\\", "/" );
            if ( parentPath.endsWith( "/" ) )
                parentPath = parentPath.left( parentPath.length() - 1 );

            if ( !parentPath.endsWith( myDir ) )
                parentPath = QDir( parentPath ).absoluteFilePath( myDir );

            auto retVal = QDir( parentPath ).absoluteFilePath( leafName );
            return retVal;
        }

        QString CDirModel::computeTransformPath( const QStandardItem *item, bool transformParentsOnly ) const
        {
            if ( !item || ( item == invisibleRootItem() ) )
                return {};
            if ( item->data( ECustomRoles::eIsRoot ).toBool() )
                return item->data( ECustomRoles::eAbsFilePath ).toString();

            auto parentDir = computeTransformPath( item->parent(), false );
            if ( NCore::CTransformResult::isDeleteThis( parentDir ) )
                return parentDir;

            auto myName = getMyTransformedName( item, transformParentsOnly );

            if ( myName.isEmpty() || parentDir.isEmpty() )
                return {};

            if ( NCore::CTransformResult::isDeleteThis( myName ) )
                return myName;

            return computeMergedPath( parentDir, myName );
        }

        QString CDirModel::getDispName( const QString &absPath ) const
        {
            auto pos = fDispNameCache.find( absPath );
            if ( pos != fDispNameCache.end() )
                return ( *pos ).second;
            QString retVal;
            if ( !fRootPath.isEmpty() )
            {
                auto rootDir = QDir( fRootPath );
                retVal = rootDir.relativeFilePath( absPath );
            }
            fDispNameCache[ absPath ] = retVal;
            return retVal;
        }

        QString CDirModel::getDispName( const QFileInfo &fi ) const
        {
            return getDispName( fi.absoluteFilePath() );
        }

        QString CDirModel::getTreeNodeName( const QFileInfo &fi ) const
        {
            return fi.fileName();
        }

        QString CDirModel::getTreeNodeName( const QString &path ) const
        {
            return getTreeNodeName( QFileInfo( path ) );
        }

        void CDirModel::appendError( QStandardItem *parent, const QString &msg )
        {
            auto errorNode = new QStandardItem( tr( "ERROR: %1" ).arg( msg ) );
            errorNode->setData( ECustomRoles::eIsErrorNode, true );
            QIcon icon;
            icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
            errorNode->setIcon( icon );

            auto pos = -1;
            for ( int ii = 0; ii < parent->rowCount(); ++ii )
            {
                if ( !parent->child( ii )->data( ECustomRoles::eIsErrorNode ).toBool() )
                {
                    pos = ii;
                    break;
                }
            }
            if ( pos == -1 )
                pos = parent->rowCount();

            parent->insertRow( pos, errorNode );
        }

        bool CDirModel::checkProcessItemExists( const QString &fileName, QStandardItem *parentItem, bool scheduledForRemoval ) const
        {
            QFileInfo fi( fileName );
            if ( !fi.exists() && !scheduledForRemoval )
            {
                appendError( parentItem, tr( "'%1' - No Longer Exists" ).arg( fileName ) );
                return false;
            }
            return true;
        }

        bool CDirModel::process( const QStandardItem *item, bool displayOnly, QStandardItem *parentItem )
        {
            if ( !item )
                return false;

            //qDebug() << item->text() << item->checkState();
            if ( ( item != invisibleRootItem() ) && ( item->checkState() == Qt::CheckState::Unchecked ) )
                return true;

            bool aOK = true;
            std::list< QStandardItem * > myItems;
            if ( item != invisibleRootItem() )
            {
                std::tie( aOK, myItems ) = processItem( item, displayOnly );
                for ( auto &&myItem : myItems )
                {
                    if ( myItem && ( myItem != parentItem ) )
                    {
                        qDebug() << myItem->text() << myItem->rowCount();
                        if ( myItem->text().isEmpty() )
                            int xyz = 0;
                        if ( parentItem )
                            parentItem->appendRow( myItem );
                        else
                            fProcessResults.second->appendRow( myItem );
                    }
                }
            }

            auto numRows = item->rowCount();
            // do not traverse children if there is a failure
            for ( int ii = 0; aOK && ii < numRows; ++ii )
            {
                auto child = item->child( ii );
                if ( !child )
                    continue;

                if ( progressCanceled() )
                    break;

                for ( auto &&myItem : myItems )
                {
                    aOK = process( child, displayOnly, myItem ) && aOK;
                }
                if ( myItems.empty() )
                    aOK = process( child, displayOnly, nullptr ) && aOK;
            }
            return aOK;
        }

        void CDirModel::process( const QModelIndex &idx, bool displayOnly )
        {
            NSABUtils::CAutoWaitCursor awc;
            fProcessResults.second = std::make_shared< QStandardItemModel >();
            if ( progressDlg() )
            {
                disconnect( progressDlg(), &NSABUtils::CDoubleProgressDlg::canceled, this, &CDirModel::slotProgressCanceled );
                connect( progressDlg(), &NSABUtils::CDoubleProgressDlg::canceled, this, &CDirModel::slotProgressCanceled );
            }

            auto item = invisibleRootItem();
            if ( idx.isValid() )
                item = getPathItemFromIndex( idx );
            fProcessResults.first = process( item, displayOnly, nullptr );
            postProcess( displayOnly );
            if ( displayOnly && progressDlg() )
                progressDlg()->stopAutoShowTimer();
        }

        void CDirModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( fProcessResults.second->rowCount() );
        }

        bool CDirModel::postExtProcess( const SProcessInfo * processInfo, QStringList &msgList )
        {
            if ( !processInfo )
                return false;

            QStringList retVal;
            bool aOK = true;
            for ( auto &&ii : processInfo->fAncillary )
            {
                if ( !NSABUtils::NFileUtils::backup( ii ) )
                {
                    msgList << QString( "ERROR: Failed to backup '%1'" ).arg( ii );
                    aOK = false;
                }
            }
            return aOK;
        }

        bool CDirModel::showProcessResults( const QString &title, const QString &label, const QMessageBox::Icon &icon, const QDialogButtonBox::StandardButtons &buttons, QWidget *parent ) const
        {
            if ( !fProcessResults.second || fProcessResults.second->rowCount() == 0 )
                return true;

            NUi::CProcessConfirm dlg( title, label, parent );
            dlg.setModel( fProcessResults.second.get() );
            dlg.setIconLabel( icon );
            dlg.setButtons( buttons );
            auto retVal = dlg.exec() == QDialog::Accepted;
            emit const_cast< CDirModel * >( this )->sigDialogClosed();
            return retVal;
        }

        void CDirModel::clear()
        {
            fPathMapping.clear();
            QStandardItemModel::clear();
        }

        std::unordered_map< NSABUtils::EMediaTags, QString > CDirModel::getMediaTags( const QFileInfo &fi, const std::list< NSABUtils::EMediaTags > &tags ) const
        {
            if ( !canShowMediaInfo() )
                return {};

            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
                return {};

            NSABUtils::CAutoWaitCursor awc;
            auto mediaInfo = getMediaInfo( fi );
            if ( !mediaInfo )
                return {};
            if ( !mediaInfo->aOK() )
                return {};
            return mediaInfo->getMediaTags( tags );
        }

        std::shared_ptr< NSABUtils::CMediaInfo > CDirModel::getMediaInfo( const QString &path, bool force ) const
        {
            return NPreferences::NCore::CPreferences::instance()->getMediaInfo( path, force );
        }

        std::shared_ptr< NSABUtils::CMediaInfo > CDirModel::getMediaInfo( const QFileInfo &fi, bool force ) const
        {
            return NPreferences::NCore::CPreferences::instance()->getMediaInfo( fi, force );
        }

        std::shared_ptr< NSABUtils::CMediaInfo > CDirModel::getMediaInfo( const QModelIndex &idx, bool force ) const
        {
            auto fileInfo = this->fileInfo( idx );
            return getMediaInfo( fileInfo, force );
        }

        void CDirModel::slotUpdateMediaInfo( const QString &path )
        {
            if ( path.isEmpty() )
                return;

            qDebug() << "Updating media info for" << path;
            auto item = getItemFromPath( path );
            if ( item )
            {
                auto idx = indexFromItem( item );

                reloadMediaInfo( idx );

                auto lhs = index( idx.row(), 0, idx.parent() );
                auto rhs = index( idx.row(), columnCount() - 1, idx.parent() );
                emit dataChanged( lhs, rhs );
            }
        }

        bool CDirModel::canShowMediaInfo() const
        {
            return true;
        }

        void CDirModel::reloadMediaInfo( const QModelIndex &idx )
        {
            reloadMediaInfo( idx, false );
        }

        void CDirModel::reloadMediaInfo( const QModelIndex &idx, bool force )
        {
            if ( !force && ( !showMediaItems() || !canShowMediaInfo() ) )
                return;

            auto fi = fileInfo( idx );
            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
                return;

            auto mediaInfo = getDefaultMediaTags( fi );
            auto mediaData = getMediaDataInfo();

            auto &&mediaTags = std::get< 1 >( mediaData );
            auto &&mediaColumns = std::get< 2 >( mediaData );

            auto mediaTagIter = mediaTags.begin();
            auto columnPosIter = mediaColumns.begin();
            for ( ; ( mediaTagIter != mediaTags.end() ) && ( columnPosIter != mediaColumns.end() ); ++mediaTagIter, ++columnPosIter )
            {
                auto col = ( *columnPosIter )();
                auto item = itemFromIndex( index( idx.row(), col, idx.parent() ) );
                if ( !item )
                    continue;

                item->setText( mediaInfo[ *mediaTagIter ] );
            }

            clearItemStatusCache( idx );
            clearPathStatusCache( fi );
        }

        std::list< SDirNodeItem > CDirModel::getMediaInfoItems( const QFileInfo &fileInfo, int offset ) const
        {
            if ( !canShowMediaInfo() )
                return {};

            std::list< SDirNodeItem > retVal;
            auto mediaInfo = getDefaultMediaTags( fileInfo );
            auto mediaData = getMediaDataInfo();

            auto &&mediaTags = std::get< 1 >( mediaData );
            auto &&mediaColumns = std::get< 2 >( mediaData );

            auto mediaTagIter = mediaTags.begin();
            auto columnPosIter = mediaColumns.begin();
            for ( ; ( mediaTagIter != mediaTags.end() ) && ( columnPosIter != mediaColumns.end() ); ++mediaTagIter, ++columnPosIter )
            {
                auto &&currTag = ( *mediaTagIter );
                retVal.emplace_back( mediaInfo[ currTag ], offset++ );
                if ( ( currTag == NSABUtils::EMediaTags::eTitle ) || ( currTag == NSABUtils::EMediaTags::eDate ) || ( currTag == NSABUtils::EMediaTags::eComment ) )
                    retVal.back().fEditable = std::make_pair( EType::eMediaTag, currTag );

                auto col = ( *columnPosIter )();
                auto firstCol = firstMediaItemColumn();
                Q_ASSERT( ( col - firstCol ) == ( retVal.size() - 1 ) );
            }
            return retVal;
        }

        void CDirModel::setupNewItem( const SDirNodeItem & /*nodeItem*/, const QStandardItem * /*nameItem*/, QStandardItem * /*item*/ ) const
        {
        }

        bool CDirModel::process( const QModelIndex &idx, const std::function< void( int count, int eventsPerPath ) > &startProgress, const std::function< void( bool finalStep, bool canceled ) > &endProgress, QWidget *parent )
        {
            process( idx, true );
            if ( fProcessResults.second && fProcessResults.second->rowCount() == 0 )
            {
                QMessageBox::information( parent, tr( "Nothing to change" ), tr( "No files or directories could be processed" ) );
                endProgress( true, false );
                return false;
            }

            bool continueOn = showProcessResults( tr( "Process:" ), tr( "Proceed?" ), QMessageBox::Information, QDialogButtonBox::Yes | QDialogButtonBox::No, parent );
            if ( !continueOn )
            {
                emit sigProcessesFinished( false, false, true, false );
                return false;
            }

            int count = computeNumberOfItems();
            startProgress( count, eventsPerPath() );
            emit sigProcessingStarted();
            process( idx, false );
            if ( !fProcessResults.first )
            {
                showProcessResults( tr( "Error While Processing:" ), tr( "Issues:" ), QMessageBox::Critical, QDialogButtonBox::Ok, parent );
            }
            endProgress( !usesQueuedProcessing(), false );
            return fProcessResults.first;
        }

        QStandardItem *CDirModel::getItem( const QStandardItem *item, int column ) const
        {
            if ( !item )
                return nullptr;

            auto parent = item->parent() ? item->parent() : QStandardItemModel::invisibleRootItem();
            if ( !parent )
                return nullptr;

            return parent->child( item->row(), column );
        }

        bool CDirModel::isChecked( const QFileInfo &fileInfo, int column ) const
        {
            return isChecked( fileInfo.absoluteFilePath(), column );
        }

        bool CDirModel::isChecked( const QString &path, int column ) const
        {
            auto item = getItemFromPath( path );
            auto colItem = getItem( item, column );
            if ( !colItem )
                return false;
            return item->checkState() != Qt::Unchecked;
        }

        bool CDirModel::autoSetMediaTags( const QModelIndex &idx, QString *msg )
        {
            auto fi = fileInfo( idx );
            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
                return false;

            if ( setMediaTags( fi.absoluteFilePath(), QString(), QString(), QString(), msg ) )
            {
                reloadMediaInfo( idx );
                return true;
            }
            return false;
        }

        bool CDirModel::areMediaTagsSameAsAutoSet( const QModelIndex &idx ) const
        {
            auto fi = fileInfo( idx );
            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
                return false;

            return isTitleSameAsAutoSet( idx ) && isDateSameAsAutoSet( idx ) && isCommentSameAsAutoSet( idx );
        }

        bool CDirModel::isTagSameAsAutoSet( const QModelIndex &idx, QString *msg, int location, const QString &tagName, const std::function< QString( const QModelIndex &idx ) > &reference ) const
        {
            auto item = itemFromIndex( idx );
            auto tagItem = getItem( item, location );
            if ( !tagItem )
                return true;

            auto referenceString = reference( idx );
            bool retVal = ( tagItem->text() == referenceString );
            if ( !retVal && msg )
            {
                *msg = tr( "%1: '%2' => '%3'" ).arg( tagName ).arg( tagItem->text().isEmpty() ? "<EMPTY>" : tagItem->text() ).arg( referenceString );
            }
            return retVal;
        }

        bool CDirModel::isTitleSameAsAutoSet( const QModelIndex &idx, QString *msg ) const
        {
            return isTagSameAsAutoSet(
                idx, msg, getMediaTitleLoc(), "Title",
                [ this ]( const QModelIndex &idx )
                {
                    auto fi = fileInfo( idx );
                    return fi.completeBaseName();
                } );
        }

        bool CDirModel::isDateSameAsAutoSet( const QModelIndex &idx, QString *msg ) const
        {
            return isTagSameAsAutoSet(
                idx, msg, getMediaDateLoc(), "Date",
                [ this ]( const QModelIndex &idx )
                {
                    auto fi = fileInfo( idx );
                    return getMediaYear( fi );
                } );
        }

        bool CDirModel::isCommentSameAsAutoSet( const QModelIndex &idx, QString *msg ) const
        {
            return isTagSameAsAutoSet( idx, msg, getMediaCommentLoc(), "Comment", []( const QModelIndex & /*idx*/ ) { return QString(); } );
        }

        bool CDirModel::setMediaTags( const QString &fileName, QString title, QString year, QString comment, QString *msg, bool ignoreIsMediaFile ) const
        {
            NSABUtils::CAutoWaitCursor awc;

            auto fi = QFileInfo( fileName );
            if ( fi.isDir() )
                return true;

            QString localMsg;
            bool aOK = true;
            if ( !ignoreIsMediaFile && !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
            {
                localMsg = tr( "'%1' is not a supported media file" ).arg( fileName );
                aOK = false;
            }
            else
            {
                if ( title.isEmpty() )
                    title = fi.completeBaseName();
                if ( year.isEmpty() )
                    year = getMediaYear( fi );

                std::unordered_map< NSABUtils::EMediaTags, QString > tags = { { NSABUtils::EMediaTags::eTitle, title }, { NSABUtils::EMediaTags::eDate, year }, { NSABUtils::EMediaTags::eComment, comment } };

                if ( NPreferences::NCore::CPreferences::instance()->getLoadMediaInfo() )
                    aOK = NSABUtils::setMediaTags( fileName, tags, NPreferences::NCore::CPreferences::instance()->getMKVPropEditEXE(), &localMsg );
            }

            if ( !aOK )
            {
                if ( msg )
                    *msg = localMsg;
                else
                {
                    QMessageBox::critical( nullptr, tr( "Could not set tag" ), tr( "Could not set one or more media tags on file '%3'. %4" ).arg( fileName ).arg( localMsg ) );
                }
            }
            return aOK;
        }

        bool CDirModel::setMediaTag( const QString &fileName, const std::pair< NSABUtils::EMediaTags, QString > &data, QString *msg ) const
        {
            NSABUtils::CAutoWaitCursor awc;
            std::unordered_map< NSABUtils::EMediaTags, QString > tags = { data };
            QString localMsg;
            auto aOK = NSABUtils::setMediaTags( fileName, tags, NPreferences::NCore::CPreferences::instance()->getMKVPropEditEXE(), &localMsg );
            if ( !aOK )
            {
                if ( msg )
                    *msg = localMsg;
                else
                {
                    QMessageBox::critical( nullptr, tr( "Could not set tag" ), tr( "Could not set media tag '%1' to '%2' on file '%3'. %4" ).arg( NSABUtils::displayName( data.first ) ).arg( data.second ).arg( fileName ).arg( localMsg ) );
                }
            }
            return aOK;
        }

        void CDirModel::addProcessError( const QString &msg )
        {
            if ( fProcessQueue.empty() )
                return;

            if ( !fProcessQueue.front()->fItem )
                return;
            appendError( fProcessQueue.front()->fItem, tr( "%1: FAILED TO PROCESS" ).arg( msg ) );
        }

        void CDirModel::slotRunNextProcessInQueue()
        {
            if ( fProcess->state() != QProcess::NotRunning )
                return;

            if ( progressDlg() && !fFirstProcess )
                progressDlg()->setValue( progressDlg()->value() + 1 );
            fFirstProcess = false;
            if ( fProcessQueue.empty() )
            {
                emit sigProcessesFinished( fProcessResults.first, true, false, true );
                return;
            }

            auto &&curr = fProcessQueue.front();

            fProcessFinishedHandled = false;
            if ( progressDlg() )
            {
                progressDlg()->setLabelText( getProgressLabel( curr ) );
                if ( curr->fMaximum != 0 )
                    progressDlg()->setSecondaryMaximum( curr->fMaximum );
            }

            if ( log() )
            {
                auto tmp = QStringList() << curr->fCmd << curr->fArgs;
                for ( auto &&ii : tmp )
                {
                    if ( ii.contains( " " ) )
                        ii = "\"" + ii + "\"";
                }
                log()->appendPlainText( "Running Command:" + tmp.join( " " ) );
            }

            if ( curr->fForceUnbuffered )
                fProcess->setCreateProcessArgumentsModifier( NSABUtils::getForceUnbufferedProcessModifier() );
            else
                fProcess->setCreateProcessArgumentsModifier( {} );
            fLastProgress.reset();
            fProcess->start( curr->fCmd, curr->fArgs, QProcess::ReadWrite );
        }

        QString CDirModel::getProgressLabel( std::shared_ptr< SProcessInfo > /*processInfo*/ ) const
        {
            return {};
        }

        QString errorString( QProcess::ProcessError error )
        {
            switch ( error )
            {
                case QProcess::FailedToStart:
                    return QObject::tr( "Failed to Start: The process failed to start.Either the invoked program is missing, or you may have insufficient permissions to invoke the program." );
                case QProcess::Crashed:
                    return QObject::tr( "Crashed: The process crashed some time after starting successfully." );
                case QProcess::Timedout:
                    return QObject::tr( "Timed out: The last waitFor...() function timed out.The state of QProcess is unchanged, and you can try calling waitFor...() again." );
                case QProcess::WriteError:
                    return QObject::tr( "Write Error: An error occurred when attempting to write to the process.For example, the process may not be running, or it may have closed its input channel." );
                case QProcess::ReadError:
                    return QObject::tr( "Read Error: An error occurred when attempting to read from the process.For example, the process may not be running." );
                default:
                case QProcess::UnknownError:
                    return QObject::tr( "Unknown Error" );
            }
        }

        QString statusString( QProcess::ExitStatus error )
        {
            switch ( error )
            {
                case QProcess::NormalExit:
                    return QObject::tr( "Normal Exit: The process exited normally." );
                case QProcess::CrashExit:
                    return QObject::tr( "Crashed: The process crashed." );
                default:
                    return {};
            }
        }

        void CDirModel::slotProgressCanceled()
        {
            fProcess->kill();
        }

        std::list< SDirNodeItem > CDirModel::addAdditionalItems( const QFileInfo &fileInfo ) const
        {
            if ( showMediaItems() && canShowMediaInfo() && ( NSABUtils::CMediaInfoMgr::instance()->isMediaCached( fileInfo ) || NPreferences::NCore::CPreferences::instance()->getLoadMediaInfo() ) )
            {
                return getMediaInfoItems( fileInfo, firstMediaItemColumn() );
            }
            return {};
        }

        QStringList CDirModel::headers() const
        {
            auto retVal = QStringList() << tr( "Name" ) << tr( "Size" ) << tr( "Type" ) << tr( "Date Modified" );
            return retVal;
        }

        void CDirModel::processFinished( const QString &msg, bool error )
        {
            if ( fProcessQueue.empty() )
                return;

            if ( log() )
                log()->appendPlainText( ( error ? tr( "Error: " ) : QString() ) + msg );
            else
                qDebug() << ( error ? tr( "Error: " ) : QString() ) + msg;

            if ( error )
                addProcessError( msg );

            bool wasCanceled = progressCanceled();
            fProcessResults.first = !error && !wasCanceled;
            fProcessQueue.front()->cleanup( this, !error && !wasCanceled );

            if ( !fProcessQueue.empty() )
            {
                fProcessQueue.pop_front();
            }

            if ( wasCanceled )
                fProcessQueue.clear();

            QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
        }

        void CDirModel::slotProcessErrorOccured( QProcess::ProcessError error )
        {
            auto msg = tr( "Error Running Command: %1(%2)" ).arg( errorString( error ) ).arg( error );
            processFinished( msg, true );
            fProcessFinishedHandled = true;
        }

        void CDirModel::slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus )
        {
            if ( fProcessFinishedHandled )
                return;

            auto msg = tr( "Running Finished: %1 Exit Code: %2" ).arg( statusString( exitStatus ) ).arg( exitCode );
            processFinished( msg, ( exitCode != 0 ) || ( exitStatus != QProcess::NormalExit ) );
        }

        void CDirModel::slotProcessStarted()
        {
            if ( fProcessQueue.empty() )
                return;
        }

        void CDirModel::slotProcesssStateChanged( QProcess::ProcessState newState )
        {
            (void)newState;
        }

        void SProcessInfo::cleanup( CDirModel *model, bool aOK )
        {
            if ( fOldName.isEmpty() || fNewNames.isEmpty() )
                return;

            if ( aOK && fPostProcess )
            {
                QString msg;
                aOK = fPostProcess( this, msg );
                if ( !aOK )
                {
                    for ( auto &&ii : fNewNames )
                    {
                        CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO CREATE" ).arg( model->getDispName( ii ) ) );
                    }
                    CDirModel::appendError( fItem, QObject::tr( "Message: %1" ).arg( msg ) );
                }
            }

            if ( !aOK )
            {
                for ( auto &&ii : fNewNames )
                {
                    QFile::remove( ii );
                }
                return;
            }

            if ( fBackupOrig )
            {
                if ( !NSABUtils::NFileUtils::backup( fOldName ) )
                {
                    CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO BACKUP" ).arg( model->getDispName( fOldName ) ) );
                    model->fProcessResults.first = false;
                    return;
                }
            }

            for ( auto &&ii : fNewNames )
            {
                if ( !QFileInfo::exists( ii ) )
                {
                    CDirModel::appendError( fItem, QObject::tr( "%1:  New file does not exist" ).arg( model->getDispName( fOldName ) ).arg( model->getDispName( ii ) ) );
                    model->fProcessResults.first = false;
                    return;
                }
            }

            for ( auto &&ii : fNewNames )
            {
                if ( QFileInfo( ii ).suffix() == "new" )
                {
                    auto newName = ii.mid( 0, ii.length() - 4 );
                    if ( QFileInfo( newName ).exists() )
                    {
                        if ( !NSABUtils::NFileUtils::backup( newName ) )
                        {
                            CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO BACKUP" ).arg( model->getDispName( newName ) ) );
                            model->fProcessResults.first = false;
                            return;
                        }
                    }

                    if ( !QFile::rename( ii, newName ) )
                    {
                        CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO MOVE ITEM TO %2" ).arg( model->getDispName( ii ) ).arg( model->getDispName( newName ) ) );
                        model->fProcessResults.first = false;
                        return;
                    }
                    ii = newName;
                }
            }

            QString msg;
            if ( fSetMetainfoTagsOnSuccess )
            {
                for ( auto &&ii : fNewNames )
                {
                    if ( !model->setMediaTags( ii, QString(), QString(), QString(), &msg, true ) )
                    {
                        CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO SET MKV Tags - %2" ).arg( model->getDispName( ii ) ).arg( msg ) );
                        model->fProcessResults.first = false;
                        return;
                    }
                }
            }

            QStringList msgs;
            if ( !model->postExtProcess( this, msgs ) )
            {
                CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO Post Process ITEM TO %2 - %3" ).arg( model->getDispName( fOldName ) ).arg( model->getDispName( fNewNames.join( ", " ) ).arg( msgs.join( "\n" ) ) ) );
                model->fProcessResults.first = false;
            }

            for ( auto &&ii : fNewNames )
            {
                if ( QFileInfo::exists( ii ) && !NSABUtils::NFileUtils::setTimeStamps( ii, fTimeStamps ) )
                {
                    CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO MODIFY TIMESTAMP ON GENERATED FILE '%2'" ).arg( model->getDispName( fOldName ) ).arg( model->getDispName( ii ) ) );
                    model->fProcessResults.first = false;
                }
            }

            if ( QFileInfo::exists( fOldName ) && !NSABUtils::NFileUtils::setTimeStamps( fOldName, fTimeStamps ) )
            {
                CDirModel::appendError( fItem, QObject::tr( "%1: FAILED TO MODIFY TIMESTAMP" ).arg( model->getDispName( fOldName ) ) );
                model->fProcessResults.first = false;
            }
        }

        QString SProcessInfo::primaryNewName() const
        {
            if ( fNewNames.isEmpty() )
                return {};
            return fNewNames.front();
        }

        QString CDirModel::getMediaYear( const QFileInfo &fi ) const
        {
            auto date = getMediaDate( fi );
            if ( date.isValid() )
                return QString::number( date.year() );
            ;
            return {};
        }

        QDate CDirModel::getMediaDate( const QFileInfo &fi ) const
        {
            auto searchPath = fi;
            QDate retVal;

            if ( searchPath.isFile() )
            {
                retVal = getMediaDate( fi.absoluteDir().absolutePath() );
            }

            while ( !retVal.isValid() )
            {
                auto baseName = searchPath.isDir() ? searchPath.fileName() : searchPath.completeBaseName();
                NCore::SSearchTMDBInfo searchInfo( baseName, {} );
                if ( searchInfo.releaseDateSet() )
                    retVal = searchInfo.releaseDate().first;
                if ( isRootPath( searchPath.absoluteFilePath() ) )
                    break;
                searchPath = searchPath.absolutePath();
            }
            return retVal;
        }

        bool CDirModel::progressCanceled() const
        {
            return progressDlg() && progressDlg()->wasCanceled();
        }

        QPlainTextEdit *CDirModel::log() const
        {
            return fBasePage->log();
        }

        NSABUtils::CDoubleProgressDlg *CDirModel::progressDlg() const
        {
            return fBasePage->progressDlg();
        }

        void CDirModel::postAddItems( const QFileInfo &fileInfo, std::list< SDirNodeItem > &currItems ) const
        {
            (void)fileInfo;
            (void)currItems;
        }

        int CDirModel::firstMediaItemColumn() const
        {
            return -1;
        }

        int CDirModel::lastMediaItemColumn() const
        {
            return fLastMediaColumn;
        }

        std::list< NSABUtils::EMediaTags > CDirModel::getMediaColumnsList() const
        {
            auto tmp = std::get< 1 >( getMediaDataInfo() );
            return std::list< NSABUtils::EMediaTags >( { tmp.begin(), tmp.end() } );
        }

        void CDirModel::clearMediaColumnMap()
        {
            fMediaColumnMap.reset();
        }

        void CDirModel::computeMediaColumnMap() const
        {
            if ( !fMediaColumnMap.has_value() )
            {
                auto map = new std::unordered_map< NSABUtils::EMediaTags, int >();
                fMediaColumnMap = map;
                auto mediaColumnsList = getMediaColumnsList();
                int currColumn = firstMediaItemColumn();
                for ( auto ii : mediaColumnsList )
                {
                    fLastMediaColumn = ( *map )[ ii ] = currColumn++;
                }
            }
        }

        int CDirModel::getMediaColumn( NSABUtils::EMediaTags mediaTag ) const
        {
            if ( !canShowMediaInfo() )
                return -1;
            computeMediaColumnMap();
            auto pos = fMediaColumnMap.value()->find( mediaTag );
            if ( pos == fMediaColumnMap.value()->end() )
                return -1;
            return ( *pos ).second;
        }

        int CDirModel::getMediaTitleLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eTitle );
        }

        int CDirModel::getMediaLengthLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eLength );
        }

        int CDirModel::getMediaDateLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eDate );
        }

        int CDirModel::getMediaResolutionLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eResolution );
        }

        int CDirModel::getMediaVideoCodecLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eAllVideoCodecs );
        }

        int CDirModel::getMediaVideoBitrateLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eVideoBitrateString );
        }

        int CDirModel::getMediaVideoHDRLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eHDRInfo );
        }

        int CDirModel::getMediaAudioCodecLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eAllAudioCodecsDisp );
        }

        int CDirModel::getMediaAudioSampleRateLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eAudioSampleRateString );
        }

        int CDirModel::getMediaSubtitlesLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eAllSubtitleLanguages );
        }

        int CDirModel::getMediaCommentLoc() const
        {
            return getMediaColumn( NSABUtils::EMediaTags::eComment );
        }

        QTreeView *CDirModel::filesView() const
        {
            return fBasePage->filesView();
        }

        void CDirModel::slotProcessStandardError()
        {
            auto currText = fProcess->readAllStandardError();
            fBasePage->appendToLog( currText, stdErrRemaining(), false, true );
        }

        void CDirModel::slotProcessStandardOutput()
        {
            auto currText = fProcess->readAllStandardOutput();
            fBasePage->appendToLog( currText, stdOutRemaining(), true, true );
        }

        void CDirModel::resizeColumns() const
        {
            NSABUtils::autoSize( filesView() );
        }

        bool CDirModel::isRootPath( const QString &path ) const
        {
            if ( path.isEmpty() )
                return false;

            return isRootPath( QFileInfo( path ) );
        }

        bool CDirModel::isRootPath( const QModelIndex &idx ) const
        {
            auto fi = fileInfo( idx );
            return isRootPath( fi );
        }

        bool CDirModel::isRootPath( const QFileInfo &path ) const
        {
            auto pos = fIsRootPathCache.find( path );
            if ( pos != fIsRootPathCache.end() )
                return ( *pos ).second;

            bool retVal = false;
            if ( !path.isFile() && !path.isRelative() )
            {
                retVal = QFileInfo( fRootPath.absoluteFilePath( "." ) ) == path;
            }
            fIsRootPathCache[ path ] = retVal;
            return retVal;
        }

        void CDirModel::updatePath( const QModelIndex &idx, const QString &oldPath, const QString &newPath )
        {
            if ( QFileInfo( newPath ).isDir() )
                updateDir( idx, oldPath, newPath );
            else
                updateFile( idx, oldPath, newPath );
        }

        void CDirModel::updateFile( const QModelIndex &idx, const QString & /*oldPath*/, const QString &newPath )
        {
            auto nameIndex = index( idx.row(), EColumns::eFSName, idx.parent() );
            setData( nameIndex, newPath, ECustomRoles::eAbsFilePath );
            auto item = itemFromIndex( nameIndex );
            fPathMapping[ newPath ] = item;
        }

        void CDirModel::updateDir( const QModelIndex &idx, const QDir &oldDir, const QDir &newDir )
        {
            auto nameIndex = index( idx.row(), EColumns::eFSName, idx.parent() );
            setData( nameIndex, newDir.absolutePath(), ECustomRoles::eAbsFilePath );
            auto item = itemFromIndex( nameIndex );
            fPathMapping[ newDir.absolutePath() ] = item;

            auto numRows = rowCount( idx );
            for ( int ii = 0; ii < numRows; ++ii )
            {
                auto childIdx = index( ii, EColumns::eFSName, idx );
                auto fileName = QFileInfo( data( childIdx, ECustomRoles::eAbsFilePath ).toString() ).fileName();

                auto oldPath = oldDir.absoluteFilePath( fileName );
                auto newPath = newDir.absoluteFilePath( fileName );
                updatePath( childIdx, oldPath, newPath );
            }
        }

        void CDirModel::slotDataChanged( const QModelIndex &start, const QModelIndex &end, const QVector< int > & /*roles*/ )
        {
            auto parent = start.parent();
            if ( end.parent() != start.parent() )
                return;

            for ( int ii = start.row(); ii <= end.row(); ++ii )
            {
                for ( int jj = start.column(); jj <= end.column(); ++jj )
                {
                    auto index = this->index( ii, jj, parent );
                    auto fi = fileInfo( index );

                    clearPathStatusCache( fi );
                    clearItemStatusCache( index );
                }
            }
        }

        void CDirModel::setIsLoading( bool isLoading )
        {
            beginResetModel();
            fIsLoading = isLoading;
            endResetModel();
            if ( filesView() )
                filesView()->expandAll();
        }

        bool CDirModel::canComputeStatus() const
        {
            return !isLoading();
        }

        std::optional< TItemStatus > CDirModel::getRowStatus( const QModelIndex &idx ) const
        {
            auto parent = idx.parent();
            int numCols = columnCount( parent );
            std::optional< TItemStatus > rowStatus;
            for ( int ii = 0; ii < numCols; ++ii )
            {
                auto peerIndex = this->index( idx.row(), ii, parent );
                auto status = getItemStatus( peerIndex );
                if ( status.has_value() )
                {
                    if ( rowStatus.has_value() )
                    {
                        rowStatus.value().first = std::max( rowStatus.value().first, status.value().first );

                        rowStatus.value().second += status.value().second;

                        rowStatus.value().second.replace( "<p", "<li" );
                        rowStatus.value().second.replace( "</p>", "</li>" );
                    }
                    else
                        rowStatus = status;
                }
            }

            if ( rowStatus.has_value() && rowStatus.value().second.startsWith( "<li" ) )
                rowStatus.value().second = "<ul>" + rowStatus.value().second + "</ul>";

            return rowStatus;
        }

        std::optional< TItemStatus > CDirModel::getIndexStatus( const QModelIndex &idx ) const
        {
            std::optional< TItemStatus > retVal;
            if ( idx.column() == NModels::EColumns::eFSName )
                retVal = getRowStatus( idx );

            if ( !retVal.has_value() )
                retVal = getItemStatus( idx );

            if ( !retVal.has_value() )
            {
                auto fi = fileInfo( idx );
                retVal = getPathStatus( fi );
            }

            return retVal;
        }

        std::optional< TItemStatus > CDirModel::computeItemStatus( QStandardItem *item ) const
        {
            auto idx = indexFromItem( item );
            if ( !idx.isValid() )
                return {};
            return computeItemStatus( idx );
        }

        std::optional< TItemStatus > CDirModel::computeItemStatus( const QModelIndex &idx ) const
        {
            (void)idx;
            return {};
        }

        std::optional< TItemStatus > CDirModel::getItemStatus( const QModelIndex &idx ) const
        {
            auto fi = fileInfo( idx );

            auto pos = fItemStatusCache.find( fi.absoluteFilePath() );
            if ( useStatusCache() && ( pos != fItemStatusCache.end() ) )
            {
                auto pos2 = ( *pos ).second.find( idx.column() );
                if ( pos2 != ( *pos ).second.end() )
                    return ( *pos2 ).second;
            }

            if ( !canComputeStatus() )
                return {};

            auto retVal = computeItemStatus( idx );
            fItemStatusCache[ fi.absoluteFilePath() ][ idx.column() ] = retVal;
            return retVal;
        }

        std::optional< TItemStatus > CDirModel::computePathStatus( const QFileInfo &fi ) const
        {
            (void)fi;
            return {};
        }

        std::optional< TItemStatus > CDirModel::getPathStatus( const QFileInfo &fi ) const
        {
            auto pos = fPathStatusCache.find( fi.absoluteFilePath() );
            if ( useStatusCache() && ( pos != fPathStatusCache.end() ) )
                return ( *pos ).second;

            if ( !canComputeStatus() )
                return {};

            auto retVal = computePathStatus( fi );
            fPathStatusCache[ fi.absoluteFilePath() ] = retVal;
            return retVal;
        }

        QVariant CDirModel::getItemBackground( const QModelIndex &idx ) const
        {
            auto itemStatus = getIndexStatus( idx );
            if ( itemStatus.has_value() )
            {
                auto retVal = NPreferences::NCore::CPreferences::instance()->getColorForStatus( itemStatus.value().first, true );
                if ( retVal.isValid() )
                    return retVal;
            }
            return {};
        }

        QVariant CDirModel::getItemForeground( const QModelIndex &idx ) const
        {
            auto itemStatus = getIndexStatus( idx );
            if ( itemStatus.has_value() )
            {
                auto retVal = NPreferences::NCore::CPreferences::instance()->getColorForStatus( itemStatus.value().first, false );
                if ( retVal.isValid() )
                    return retVal;
            }
            return {};
        }

        QVariant CDirModel::getToolTip( const QModelIndex &idx ) const
        {
            auto itemStatus = getIndexStatus( idx );
            if ( itemStatus.has_value() )
                return itemStatus.value().second;
            return {};
        }

        QVariant CDirModel::getPathDecoration( const QModelIndex &idx, const QVariant &baseDecoration ) const
        {
            (void)idx;
            return baseDecoration;
        }

        void CDirModel::clearItemStatusCache( const QModelIndex &idx ) const
        {
            auto fi = fileInfo( idx );
            auto pos = fItemStatusCache.find( fi.absoluteFilePath() );
            if ( pos == fItemStatusCache.end() )
                return;

            fItemStatusCache.erase( pos );
        }

        void CDirModel::clearPathStatusCache( const QFileInfo &fi ) const
        {
            auto pos = fPathStatusCache.find( fi.absoluteFilePath() );
            if ( pos != fPathStatusCache.end() )
                fPathStatusCache.erase( pos );
        }

        void CDirModel::clearPathStatusCache( const QString &path ) const
        {
            return clearPathStatusCache( QFileInfo( path ) );
        }

        void CDirModel::resetStatusCaches()
        {
            beginResetModel();
            fPathStatusCache.clear();
            fItemStatusCache.clear();
            endResetModel();
            if ( filesView() )
                filesView()->expandAll();
        }

        bool CDirModel::isSeasonDir( const QModelIndex &origIdx, bool *isNameOK ) const
        {
            if ( isRootPath( origIdx ) )
                return false;
            QModelIndex idx = origIdx;
            if ( origIdx.column() != NModels::EColumns::eFSName )
            {
                idx = origIdx.model()->index( origIdx.row(), NModels::EColumns::eFSName, origIdx.parent() );
            }
            auto fileInfo = this->fileInfo( idx );
            auto baseName = fileInfo.baseName();
            if ( fileInfo.isDir() && baseName.startsWith( "season", Qt::CaseInsensitive ) )
            {
                QRegularExpression regExp1( R"(Season\s+\d+\s+\(\d{4}\))" );
                QRegularExpression regExp2( R"(Season\s+\d+)" );
                auto match1 = regExp1.match( baseName );
                auto match2 = regExp2.match( baseName );
                if ( isNameOK )
                    *isNameOK = match1.hasMatch() && ( match1.capturedLength() == baseName.length() );
                return ( match1.hasMatch() && ( match1.capturedLength() == baseName.length() ) ) || ( match2.hasMatch() && ( match2.capturedLength() == baseName.length() ) );
            }
            return false;
        }

        void CDirModel::clearMessages()
        {
            fMessagesForFiles.clear();
            fMsgItems.clear();
        }

        void CDirModel::addMessageForFile( const QString &msg )
        {
            if ( fProcessQueue.empty() )
                return;
            if ( msg.isEmpty() )
                return;

            auto fi = QFileInfo( fProcessQueue.front()->fOldName );
            fMessagesForFiles[ fi.absoluteFilePath() ] << msg;
        }

        std::list< QStandardItem * > CDirModel::messageItems( bool andClear )
        {
            auto retVal = fMsgItems;
            if ( andClear )
                fMsgItems.clear();
            return retVal;
        }

        QIcon CIconProvider::icon( const QFileInfo &info ) const
        {
            if ( NSABUtils::NFileUtils::isIPAddressNetworkPath( info ) )
                return {};
            return QFileIconProvider::icon( info );
        }

        QStringList CDirModel::getMediaHeaders() const
        {
            if ( !canShowMediaInfo() )
                return {};
            return std::get< 0 >( getMediaDataInfo() );
        }

        std::unordered_map< NSABUtils::EMediaTags, QString > CDirModel::getDefaultMediaTags( const QFileInfo &fi ) const
        {
            return getMediaTags( fi, std::get< 1 >( getMediaDataInfo() ) );
        }

        std::tuple< QStringList, std::list< NSABUtils::EMediaTags >, std::list< std::function< int() > > > CDirModel::getMediaDataInfo() const
        {
            static auto sDefaultHeaders =   //
                QStringList()   //
                << tr( "Title" )   //
                << tr( "Length" )   //
                << tr( "Media Date" )   //
                << tr( "Resolution" )   //
                << tr( "Video Codec(s)" )   //
                << tr( "Video Bitrate" )   //
                << tr( "HDR Info" )   //
                << tr( "Audio Codec(s)" )   //
                << tr( "Audio Sample Rate" )   //
                << tr( "Subtitles(s)" )   //
                << tr( "Comment" );

            static auto sDefaultTags =   //
                std::list< NSABUtils::EMediaTags >( {
                    NSABUtils::EMediaTags::eTitle,   //
                    NSABUtils::EMediaTags::eLength,   //
                    NSABUtils::EMediaTags::eDate,   //
                    NSABUtils::EMediaTags::eResolution,   //
                    NSABUtils::EMediaTags::eAllVideoCodecs,   //
                    NSABUtils::EMediaTags::eVideoBitrateString,   //
                    NSABUtils::EMediaTags::eHDRInfo,   //
                    NSABUtils::EMediaTags::eAllAudioCodecsDisp,   //
                    NSABUtils::EMediaTags::eAudioSampleRateString,   //
                    NSABUtils::EMediaTags::eAllSubtitleLanguages,   //
                    NSABUtils::EMediaTags::eComment   //
                } );

            auto getPosFuncs = std::list< std::function< int() > >( {
                { [ this ]()
                  {
                      return getMediaTitleLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaLengthLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaDateLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaResolutionLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaVideoCodecLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaVideoBitrateLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaVideoHDRLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaAudioCodecLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaAudioSampleRateLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaSubtitlesLoc();
                  } }   //
                ,
                { [ this ]()
                  {
                      return getMediaCommentLoc();
                  } }   //
            } );

            Q_ASSERT( sDefaultHeaders.count() == sDefaultTags.size() );
            Q_ASSERT( sDefaultHeaders.count() == getPosFuncs.size() );
            return std::make_tuple( sDefaultHeaders, sDefaultTags, getPosFuncs );
        }

        QString CDirModel::getSecondaryProgressFormat( NSABUtils::CDoubleProgressDlg *progressDlg ) const
        {
            return progressDlg->defaultSecondaryFormat();
        }

        void CDirModel::processLog( const QString &string, NSABUtils::CDoubleProgressDlg *progressDlg )
        {
            auto newProgress = getCurrentProgress( string );
            if ( newProgress.has_value() )
            {
                if ( newProgress.value().second.has_value() )
                {
                    progressDlg->setSecondaryMaximum( newProgress.value().second.value() );
                }
                progressDlg->setSecondaryValue( newProgress.value().first );
            }

            auto format = getSecondaryProgressFormat( progressDlg );

            if ( newProgress.has_value() )
            {
                if ( !newProgress.value().second.has_value() )
                    newProgress.value().second = progressDlg->secondaryMax();

                auto msecsRemaining = this->getMSRemaining( string, newProgress.value() );
                if ( !msecsRemaining.has_value() )
                {
                    if ( fLastProgress.has_value() )
                    {
                        auto msecs = fLastProgress.value().first.msecsTo( QDateTime::currentDateTime() );
                        auto numSteps = newProgress.value().first - fLastProgress.value().second;
                        auto msecsPerStep = static_cast< double >( msecs ) / static_cast< double >( numSteps );
                        auto remainingMsecs = static_cast< uint64_t >( msecsPerStep * ( newProgress.value().second.value() - newProgress.value().first ) );

                        msecsRemaining = std::chrono::milliseconds( remainingMsecs );
                    }
                    fLastProgress = std::make_pair( QDateTime::currentDateTime(), newProgress.value().first );
                }

                if ( msecsRemaining.has_value() )
                {
                    auto ts = NSABUtils::CTimeString( msecsRemaining.value() );
                    if ( this->currentUnitsAreSeconds() )
                    {
                        auto currTS = NSABUtils::CTimeString( progressDlg->secondaryValue() * 1000 );
                        auto endTS = NSABUtils::CTimeString( progressDlg->secondaryMax() * 1000 );
                        format = QString( "Processing Position: %1 of %2 ETA: %3  " ).arg( currTS.toString( "hh:mm:ss", false ) ).arg( endTS.toString( "hh:mm:ss", false ) ).arg( ts.toString( "hh:mm:ss", false ) );
                    }
                    else
                    {
                        format = format + ts.toString( " ETA: hh:mm:ss  ", false );
                    }
                }
            }
            progressDlg->setSecondaryFormat( format );
        }
    }
}
