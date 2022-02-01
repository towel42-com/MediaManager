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
#include "SABUtils/FileCompare.h"
#include "SABUtils/MKVUtils.h"

#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/utils.h"

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
#include <QBrush>
#include <QDirIterator>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QTemporaryFile>

#include <QProcess>

#include <set>
#include <list>

QDebug operator<<( QDebug dbg, const NMediaManager::NModels::STreeNode & node )
{
    dbg << "STreeNode(";
    auto name = node.name();
    dbg << "'" << name << "' ";
    dbg << "Path: " << (QFileInfo( name ).isFile() ? "Yes" : "No") << " "
        << "Is Loaded? " << node.fLoaded << " "
        << "Is File? " << node.fIsFile
        << ")"
        ;
    return dbg;
}

QDebug operator<<( QDebug dbg, const NMediaManager::NModels::TParentTree & parentTree )
{
    dbg << " TParentTree( ";
    bool first = true;
    for ( auto && ii : parentTree )
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
        bool useCache()
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

        CDirModelItem::CDirModelItem( const QString & text, EType type ) :
            QStandardItem( text ),
            fType( type )
        {
            setEditable( true );
        }

        CDirModel::CDirModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            QStandardItemModel( parent ),
            fBasePage( page )
        {
            fIconProvider = new QFileIconProvider();
            fTimer = new QTimer( this );
            fTimer->setInterval( 50 );
            fTimer->setSingleShot( true );
            connect( fTimer, &QTimer::timeout, this, &CDirModel::slotLoadRootDirectory );

            fProcess = new QProcess( this );
            connect( fProcess, &QProcess::errorOccurred, this, &CDirModel::slotProcessErrorOccured );
            connect( fProcess, qOverload< int, QProcess::ExitStatus >( &QProcess::finished ), this, &CDirModel::slotProcessFinished );
            connect( fProcess, &QProcess::readyReadStandardError, this, &CDirModel::slotProcessStandardError );
            connect( fProcess, &QProcess::readyReadStandardOutput, this, &CDirModel::slotProcessStandardOutput );
            connect( fProcess, &QProcess::started, this, &CDirModel::slotProcessStarted );
            connect( fProcess, &QProcess::stateChanged, this, &CDirModel::slotProcesssStateChanged );

            connect( this, &QStandardItemModel::dataChanged, this, &CDirModel::slotDataChanged );
        }

        CDirModel::~CDirModel()
        {
            delete fIconProvider;
        }

        void CDirModel::setRootPath( const QString & rootPath )
        {
            fRootPath = QDir( rootPath );
            reloadModel();
        }

        void CDirModel::setNameFilters( const QStringList & filters )
        {
            fNameFilter = filters;
            reloadModel();
        }

        void CDirModel::reloadModel()
        {
            fTimer->stop();
            fTimer->start();
            postReloadModelRequest();
        }

        void CDirModel::slotLoadRootDirectory()
        {
            NSABUtils::CAutoWaitCursor awc;

            preLoad();

            QFileInfo rootFI( fRootPath.absolutePath() );

            if ( progressDlg() )
            {
                progressDlg()->setLabelText( tr( "Computing number of Files under '%1'" ).arg( fRootPath.absolutePath() ) );
                qApp->processEvents();
                auto numFiles = computeNumberOfFiles( rootFI ).second;
                if ( !progressCanceled() )
                    progressDlg()->setRange( 0, numFiles );
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
            fLoading = true;
            clear();
            setHorizontalHeaderLabels( headers() );
            preLoad( filesView() );
        }

        void CDirModel::postLoad( bool /*aOK*/ )
        {
            fLoading = false;

            if ( !filesView() )
                return;

            emit sigDirLoadFinished( progressCanceled() );
            postLoad( filesView() );
        }

        void CDirModel::postLoad( QTreeView * treeView )
        {
            resizeColumns();
            treeView->expandAll();
        }

        void CDirModel::preLoad( QTreeView * /*treeView*/ )
        {}

        void CDirModel::postReloadModelRequest()
        {}

        int CDirModel::computeNumberOfItems() const
        {
            return NSABUtils::itemCount( fProcessResults.second.get(), true );
        }

        bool CDirModel::setData( const QModelIndex & idx, const QVariant & value, int role )
        {
            if ( role == Qt::EditRole )
            {
                auto item = itemFromIndex( idx );
                if ( !item )
                    return false;

                switch ( static_cast<EType>(item->type()) )
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
                        auto mediaTagType = static_cast<NSABUtils::EMediaTags>(item->data( NModels::ECustomRoles::eMediaTagTypeRole ).toInt());
                        if ( !setMediaTag( fileInfo( idx ).absoluteFilePath(), { mediaTagType, value.toString() } ) )
                            return false;
                    }
                    break;
                }
            }

            return QStandardItemModel::setData( idx, value, role );
        }

        std::unique_ptr< QDirIterator > CDirModel::getDirIteratorForPath( const QFileInfo & fileInfo ) const
        {
            auto retVal = std::make_unique< QDirIterator >( fileInfo.absoluteFilePath(), fNameFilter, QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable );
            return std::move( retVal );
        }

        void CDirModel::iterateEveryFile( const QFileInfo & fileInfo, const SIterateInfo & iterInfo, std::optional< QDateTime > & lastUpdateUI ) const
        {
            if ( !fileInfo.exists() )
                return;

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
                            iterInfo.fPostDirFunction( fileInfo, false );
                        return;
                    }

                    while ( ii->hasNext() )
                    {
                        ii->next();
                        auto fi = ii->fileInfo();
                        iterateEveryFile( fi, iterInfo, lastUpdateUI );
                        if ( progressDlg() && (!lastUpdateUI.has_value() || (lastUpdateUI.value().msecsTo( QDateTime::currentDateTime() ) > 250)) )
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
            else
            {
                bool aOK = true;
                if ( iterInfo.fPreFileFunction )
                    aOK = iterInfo.fPreFileFunction( fileInfo );
                if ( iterInfo.fPostFileFunction )
                    iterInfo.fPostFileFunction( fileInfo, aOK );
            }
        }

        std::pair< uint64_t, uint64_t > CDirModel::computeNumberOfFiles( const QFileInfo & fileInfo ) const
        {
            if ( progressDlg() )
                progressDlg()->setRange( 0, 0 );

            uint64_t numDirs = 0;
            uint64_t numFiles = 0;
            SIterateInfo info;
            info.fPreDirFunction = [ this, &numDirs ]( const QFileInfo & dirInfo )
            {
                if ( isSkippedDirName( dirInfo ) )
                    return false;
                numDirs++;
                return true;
            };
            info.fPreFileFunction = [ &numFiles ]( const QFileInfo & /*file*/ )
            {
                numFiles++;
                return false;
            };
            std::optional< QDateTime > lastUpdate;
            iterateEveryFile( fileInfo, info, lastUpdate );

            return std::make_pair( numDirs, numFiles );
        }

        void CDirModel::loadFileInfo( const QFileInfo & fileInfo )
        {
            if ( !fileInfo.exists() )
                return;

            TParentTree tree;

            SIterateInfo info;
            info.fPreDirFunction = [ this, &tree ]( const QFileInfo & dirInfo )
            {
                //qDebug().noquote().nospace() << "Pre Directory A: " << dirInfo.absoluteFilePath() << tree;

                auto row = getItemRow( dirInfo );
                tree.push_back( std::move( row ) );

                if ( progressDlg() )
                    progressDlg()->setLabelText( tr( "Searching Directory '%1'" ).arg( QDir( fRootPath ).relativeFilePath( dirInfo.absoluteFilePath() ) ) );

                if ( isSkippedDirName( dirInfo ) )
                {
                    //qDebug().noquote().nospace() << "Pre Directory B: returning false" << dirInfo.absoluteFilePath() << tree;
                    return false;
                }

                //qDebug().noquote().nospace() << "Pre Directory B: returning true" << dirInfo.absoluteFilePath() << tree;
                return true;
            };

            std::unordered_set< QString > alreadyAdded;
            info.fPreFileFunction = [ this, &tree, &alreadyAdded ]( const QFileInfo & fileInfo )
            {
                tree.push_back( std::move( getItemRow( fileInfo ) ) ); // mkv file

                // need to be children of file
                auto attachFile = preFileFunction( fileInfo, alreadyAdded, tree );

                //qDebug().noquote().nospace() << "Pre File A: " << fileInfo.absoluteFilePath() << tree;

                if ( attachFile )
                {
                    attachTreeNodes( tree );
                }

                //qDebug().noquote().nospace() << "Pre File B: " << fileInfo.absoluteFilePath() << tree;

                if ( progressDlg() )
                    progressDlg()->setValue( progressDlg()->value() + 1 );
                return true;
            };

            info.fPostDirFunction = [ this, &tree ]( const QFileInfo & dirInfo, bool aOK )
            {
                (void)dirInfo;
                (void)aOK;
                //qDebug().noquote().nospace() << "Post Dir A: " << dirInfo.absoluteFilePath() << tree << "AOK? " << aOK;  
                tree.pop_back();
                //qDebug().noquote().nospace() << "Post Dir B: " << dirInfo.absoluteFilePath() << tree;
                if ( filesView() )
                    resizeColumns();
            };

            info.fPostFileFunction = [ this, &tree ]( const QFileInfo & fileInfo, bool aOK )
            {
                //qDebug() << fileInfo.absoluteFilePath();
                postFileFunction( aOK, fileInfo );

                //qDebug().noquote().nospace() << "Post File A: " << dirInfo.absoluteFilePath() << tree << "AOK? " << aOK;
                while ( tree.back().fIsFile )
                    tree.pop_back();
                //qDebug().noquote().nospace() << "Post File B: " << dirInfo.absoluteFilePath() << tree;
            };

            std::optional< QDateTime > lastUpdate;
            iterateEveryFile( fileInfo, info, lastUpdate );
        }

        void CDirModel::appendRow( QStandardItem * parent, QList< QStandardItem * > & items )
        {
            if ( parent )
                parent->appendRow( items );
            else
                QStandardItemModel::appendRow( items );
        }

        QStandardItem * CDirModel::attachTreeNodes( TParentTree & parentTree )
        {
            QStandardItem * prevParent = nullptr;
            for ( auto && ii : parentTree )
            {
                auto nextParent = ii.rootItem();
                if ( !ii.fLoaded ) // already been loaded
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
                fPathMapping[nextParent->data( ECustomRoles::eFullPathRole ).toString()] = nextParent;

                if ( filesView() && prevParent )
                    filesView()->setExpanded( prevParent->index(), true );
            }
            return prevParent;
        }

        bool CDirModel::isSkippedDirName( const QFileInfo & ii ) const
        {
            return NPreferences::NCore::CPreferences::instance()->isSkippedPath( ii );
        }

        bool CDirModel::isIgnoredPathName( const QFileInfo & fileInfo ) const
        {
            return NPreferences::NCore::CPreferences::instance()->isIgnoredPath( fileInfo );
        }

        STreeNode CDirModel::getItemRow( const QFileInfo & fileInfo ) const
        {
            return STreeNode( fileInfo, this, isRootPath( fileInfo ) );
        }

        STreeNode::STreeNode( const QFileInfo & fileInfo, const CDirModel * model, bool isRoot ) :
            fModel( model )
        {
            fIsFile = fileInfo.isFile();
            //qDebug() << fileInfo.absoluteFilePath() << isRoot;
            auto nameItem = SDirNodeItem( isRoot ? QDir::toNativeSeparators( fileInfo.canonicalFilePath() ) : fileInfo.fileName(), EColumns::eFSName );
            nameItem.fIcon = model->iconProvider()->icon( fileInfo );
            nameItem.setData( fileInfo.absoluteFilePath(), ECustomRoles::eFullPathRole );
            nameItem.setData( fileInfo.isDir(), ECustomRoles::eIsDir );
            nameItem.fEditable = std::make_pair( EType::ePath, static_cast< NSABUtils::EMediaTags >( -1 ) );
            fItems.push_back( nameItem );
            fItems.emplace_back( fileInfo.isFile() ? NSABUtils::NFileUtils::fileSizeString( fileInfo ) : QString(), EColumns::eFSSize );
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

        QString STreeNode::name() const
        {
            return rootItem() ? rootItem()->text() : NCore::STransformResult::getNoItems();
        }

        QStandardItem * STreeNode::item( EColumns column, bool createIfNecessary /*= true */ ) const
        {
            items( createIfNecessary );
            return (column >= fRealItems.count()) ? nullptr : fRealItems[int( column )];
        }

        QStandardItem * STreeNode::rootItem( bool createIfNecessary /*= true */ ) const
        {
            return item( static_cast<EColumns>(0), createIfNecessary );
        }

        QList< QStandardItem * > STreeNode::items( bool createIfNecessary ) const
        {
            if ( createIfNecessary && fRealItems.isEmpty() )
            {
                for ( auto && ii : fItems )
                {
                    auto currItem = ii.createStandardItem();

                    auto nameItem = fRealItems.isEmpty() ? nullptr : fRealItems.front();

                    fModel->setupNewItem( ii, nameItem, currItem );

                    fRealItems << currItem;
                }
            }
            return fRealItems;
        }

        void CDirModel::setChecked( QStandardItem * item, bool isChecked ) const
        {
            item->setText( isChecked ? "Yes" : "No" );
            item->setCheckState( isChecked ? Qt::Checked : Qt::Unchecked );
        }

        void CDirModel::setChecked( QStandardItem * item, ECustomRoles role, bool isChecked ) const
        {
            setChecked( item, isChecked );
            item->setData( isChecked, role );
        }

        QStandardItem * CDirModel::getItemFromPath( const QFileInfo & fi ) const
        {
            auto path = fi.absoluteFilePath();
            auto pos = fPathMapping.find( path );
            if ( pos != fPathMapping.end() )
                return (*pos).second;
            return nullptr;
        }

        QVariant CDirModel::data( const QModelIndex & idx, int role ) const
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
            return QStandardItemModel::data( idx, role );
        }

        QFileInfo CDirModel::fileInfo( const QStandardItem * item ) const
        {
            if ( !item )
                return {};
            auto baseItem = getItem( item, NModels::EColumns::eFSName );
            auto path = baseItem->data( ECustomRoles::eFullPathRole ).toString();
            return QFileInfo( path );
        }

        QStandardItem * CDirModel::getPathItemFromIndex( const QModelIndex & idx ) const
        {
            return itemFromIndex( idx );
        }

        bool CDirModel::isDir( const QStandardItem * item ) const
        {
            return fileInfo( item ).isDir();
        }

        QString CDirModel::filePath( const QStandardItem * item ) const
        {
            return fileInfo( item ).absoluteFilePath();
        }

        QString CDirModel::filePath( const QModelIndex & idx ) const
        {
            auto item = getPathItemFromIndex( idx );
            return filePath( item );
        }

        QFileInfo CDirModel::fileInfo( const QModelIndex & idx ) const
        {
            auto item = getPathItemFromIndex( idx );
            return fileInfo( item );
        }

        bool CDirModel::isDir( const QModelIndex & idx ) const
        {
            auto item = getPathItemFromIndex( idx );
            return isDir( item );
        }

        bool CDirModel::isMediaFile( const QStandardItem * item ) const
        {
            return isMediaFile( fileInfo( item ) );
        }

        bool CDirModel::isMediaFile( const QModelIndex & idx ) const
        {
            return isMediaFile( fileInfo( idx ) );
        }

        bool CDirModel::isMediaFile( const QFileInfo & fileInfo ) const
        {
            return fileInfo.isFile() && NPreferences::NCore::CPreferences::instance()->isMediaFile( fileInfo );
        }

        bool CDirModel::isSubtitleFile( const QFileInfo & fileInfo, bool * isLangFileFormat ) const
        {
            return NPreferences::NCore::CPreferences::instance()->isSubtitleFile( fileInfo, isLangFileFormat );
        }

        bool CDirModel::isSubtitleFile( const QModelIndex & idx, bool * isLangFileFormat ) const
        {
            return isSubtitleFile( fileInfo( idx ), isLangFileFormat );
        }

        bool CDirModel::isSubtitleFile( const QStandardItem * item, bool * isLangFileFormat /*= nullptr */ ) const
        {
            return isSubtitleFile( fileInfo( item ), isLangFileFormat );
        }

        QString CDirModel::getMyTransformedName( const QStandardItem * item, bool /*transformParentsOnly*/ ) const
        {
            return item->text();
        }

        QString CDirModel::computeTransformPath( const QStandardItem * item, bool transformParentsOnly ) const
        {
            if ( !item || (item == invisibleRootItem()) )
                return {};
            if ( item->data( ECustomRoles::eIsRoot ).toBool() )
                return item->data( ECustomRoles::eFullPathRole ).toString();

            auto parentDir = computeTransformPath( item->parent(), false );
            if ( NCore::STransformResult::isDeleteThis( parentDir ) )
                return parentDir;

            auto myName = getMyTransformedName( item, transformParentsOnly );

            if ( myName.isEmpty() || parentDir.isEmpty() )
                return {};

            if ( NCore::STransformResult::isDeleteThis( myName ) )
                return myName;

            auto retVal = QDir( parentDir ).absoluteFilePath( myName );
            return retVal;
        }

        QString CDirModel::getDispName( const QString & absPath ) const
        {
            if ( fRootPath.isEmpty() )
                return {};
            auto rootDir = QDir( fRootPath );
            return rootDir.relativeFilePath( absPath );
        }

        QString CDirModel::getDispName( const QFileInfo & absPath ) const
        {
            return getDispName( absPath.absoluteFilePath() );
        }

        void CDirModel::appendError( QStandardItem * parent, QStandardItem * errorNode )
        {
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

        bool CDirModel::checkProcessItemExists( const QString & fileName, QStandardItem * parentItem, bool scheduledForRemoval ) const
        {
            QFileInfo fi( fileName );
            if ( !fi.exists() && !scheduledForRemoval )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: '%1' - No Longer Exists" ).arg( fileName ) );
                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                appendError( parentItem, errorItem );

                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                errorItem->setIcon( icon );
                return false;
            }
            return true;
        }

        bool CDirModel::process( const QStandardItem * item, bool displayOnly, QStandardItem * parentItem )
        {
            if ( !item )
                return false;

            bool aOK = true;
            QStandardItem * myItem = nullptr;
            if ( item != invisibleRootItem() )
                std::tie( aOK, myItem ) = processItem( item, parentItem, displayOnly );

            auto numRows = item->rowCount();
            for ( int ii = 0; ii < numRows; ++ii )
            {
                auto child = item->child( ii );
                if ( !child )
                    continue;

                if ( progressCanceled() )
                    break;

                aOK = process( child, displayOnly, myItem ) && aOK;
            }
            return aOK;
        }

        void CDirModel::process( const QModelIndex & idx, bool displayOnly )
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
        }

        void CDirModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( fProcessResults.second->rowCount() );
        }

        bool CDirModel::postExtProcess( const SProcessInfo & info, QStringList & msgList )
        {
            QStringList retVal;
            bool aOK = true;
            for ( auto && ii : info.fAncillary )
            {
                bool tmpAOK = QFile::rename( ii, ii + ".bak" );
                if ( !tmpAOK )
                {
                    msgList << QString( "ERROR: Failed to backup '%1'" ).arg( ii );
                    aOK = false;
                }
            }
            return aOK;
        }

        bool CDirModel::showProcessResults( const QString & title, const QString & label, const QMessageBox::Icon & icon, const QDialogButtonBox::StandardButtons & buttons, QWidget * parent ) const
        {
            if ( !fProcessResults.second || fProcessResults.second->rowCount() == 0 )
                return true;

            NUi::CProcessConfirm dlg( title, label, parent );
            dlg.setModel( fProcessResults.second.get() );
            dlg.setIconLabel( icon );
            dlg.setButtons( buttons );
            return dlg.exec() == QDialog::Accepted;
        }

        void CDirModel::clear()
        {
            fPathMapping.clear();
            QStandardItemModel::clear();
        }

        std::unordered_map< NSABUtils::EMediaTags, QString > CDirModel::getMediaTags( const QFileInfo & fi, const std::list< NSABUtils::EMediaTags > & tags ) const
        {
            if ( !canShowMediaInfo() )
                return {};

            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
                return {};

            NSABUtils::CAutoWaitCursor awc;
            auto retVal = NSABUtils::getMediaTags( fi.absoluteFilePath(), tags );

            return retVal;
        }

        bool CDirModel::canShowMediaInfo() const
        {
            return true;
        }

        void CDirModel::reloadMediaTags( const QModelIndex & idx )
        {
            reloadMediaTags( idx, false );
        }

        void CDirModel::reloadMediaTags( const QModelIndex & idx, bool force )
        {
            if ( !force && (!showMediaItems() || !canShowMediaInfo()) )
                return;

            auto fi = fileInfo( idx );
            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
                return;

            auto mediaInfo = getMediaTags( fi, { NSABUtils::EMediaTags::eTitle, NSABUtils::EMediaTags::eLength, NSABUtils::EMediaTags::eDate, NSABUtils::EMediaTags::eComment } );

            QStandardItem * item = itemFromIndex( index( idx.row(), getMediaTitleLoc(), idx.parent() ) );
            item->setText( mediaInfo[ NSABUtils::EMediaTags::eTitle ] );

            item = itemFromIndex( index( idx.row(), getMediaLengthLoc(), idx.parent() ) );
            item->setText( mediaInfo[ NSABUtils::EMediaTags::eLength ] );

            item = itemFromIndex( index( idx.row(), getMediaDateLoc(), idx.parent() ) );
            item->setText( mediaInfo[ NSABUtils::EMediaTags::eDate ] );

            item = itemFromIndex( index( idx.row(), getMediaCommentLoc(), idx.parent() ) );
            item->setText( mediaInfo[ NSABUtils::EMediaTags::eComment ] );

            clearItemStatusCache( idx );
            clearPathStatusCache( fi );
        }

        std::list<SDirNodeItem> CDirModel::getMediaInfoItems( const QFileInfo & fileInfo, int offset ) const
        {
            if ( !canShowMediaInfo() )
                return {};

            std::list<SDirNodeItem> retVal;
            auto mediaInfo = getMediaTags( fileInfo, { NSABUtils::EMediaTags::eTitle, NSABUtils::EMediaTags::eLength, NSABUtils::EMediaTags::eDate, NSABUtils::EMediaTags::eComment } );

            retVal.emplace_back( mediaInfo[NSABUtils::EMediaTags::eTitle], offset++ );
            retVal.back().fEditable = std::make_pair( EType::eMediaTag, NSABUtils::EMediaTags::eTitle );

            retVal.emplace_back( mediaInfo[NSABUtils::EMediaTags::eLength], offset++ );

            retVal.emplace_back( mediaInfo[NSABUtils::EMediaTags::eDate], offset++ );
            retVal.back().fEditable = std::make_pair( EType::eMediaTag, NSABUtils::EMediaTags::eDate );

            retVal.emplace_back( mediaInfo[NSABUtils::EMediaTags::eComment], offset++ );
            retVal.back().fEditable = std::make_pair( EType::eMediaTag, NSABUtils::EMediaTags::eComment );
            return retVal;
        }

        void CDirModel::setupNewItem( const SDirNodeItem & /*nodeItem*/, const QStandardItem * /*nameItem*/, QStandardItem * /*item*/ ) const
        {

        }

        bool CDirModel::process( const QModelIndex & idx, const std::function< void( int count, int eventsPerPath ) > & startProgress, const std::function< void( bool finalStep, bool canceled ) > & endProgress, QWidget * parent )
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

        QStandardItem * CDirModel::getItem( const QStandardItem * item, int column ) const
        {
            auto idx = indexFromItem( item );
            if ( !idx.isValid() )
                return nullptr;

            auto retVal = itemFromIndex( index( idx.row(), column, idx.parent() ) );
            return retVal;
        }

        bool CDirModel::isChecked( const QFileInfo & fileInfo, int column ) const
        {
            return isChecked( fileInfo.absoluteFilePath(), column );
        }

        bool CDirModel::isChecked( const QString & path, int column ) const
        {
            auto item = getItemFromPath( path );
            auto colItem = getItem( item, column );
            if ( !colItem )
                return false;
            return item->checkState() != Qt::Unchecked;
        }

        bool CDirModel::autoSetMediaTags( const QModelIndex & idx, QString * msg )
        {
            auto fi = fileInfo( idx );
            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fi ) )
                return false;

            auto baseName = fi.completeBaseName();

            auto year = getMediaYear( fi );

            if ( setMediaTags( fi.absoluteFilePath(), baseName, year, msg ) )
            {
                reloadMediaTags( idx );
                return true;
            }
            return false;
        }


        bool CDirModel::setMediaTags( const QString & fileName, const QString & title, const QString & year, QString * msg ) const
        {
            NSABUtils::CAutoWaitCursor awc;

            std::unordered_map< NSABUtils::EMediaTags, QString > tags =
            {
                { NSABUtils::EMediaTags::eTitle, (title.isEmpty() ? QFileInfo( fileName ).completeBaseName() : title) }
                ,{ NSABUtils::EMediaTags::eDate, year }
            };

            QString localMsg;
            auto aOK = NSABUtils::setMediaTags( fileName, tags, NPreferences::NCore::CPreferences::instance()->getMKVPropEditEXE(), &localMsg );
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

        bool CDirModel::setMediaTag( const QString & fileName, const std::pair< NSABUtils::EMediaTags, QString > & data, QString * msg ) const
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

        void CDirModel::addProcessError( const QString & msg )
        {
            if ( fProcessQueue.empty() )
                return;

            if ( !fProcessQueue.front().fItem )
                return;
            auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO PROCESS" ).arg( msg ) );
            errorItem->setData( ECustomRoles::eIsErrorNode, true );
            appendError( fProcessQueue.front().fItem, errorItem );

            QIcon icon;
            icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
            errorItem->setIcon( icon );
            fProcessResults.first = false;
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

            auto && curr = fProcessQueue.front();

            fProcessFinishedHandled = false;
            if ( progressDlg() )
            {
                progressDlg()->setLabelText( getProgressLabel( curr ) );
            }
            if ( log() )
            {
                auto tmp = QStringList() << curr.fCmd << curr.fArgs;
                for ( auto && ii : tmp )
                {
                    if ( ii.contains( " " ) )
                        ii = "\"" + ii + "\"";
                }
                log()->appendPlainText( "Running Command:" + tmp.join( " " ) );
            }

            fProcess->start( curr.fCmd, curr.fArgs );
        }

        QString CDirModel::getProgressLabel( const SProcessInfo & /*processInfo*/ ) const
        {
            return {};
        }

        QString errorString( QProcess::ProcessError error )
        {
            switch ( error )
            {
                case QProcess::FailedToStart: return QObject::tr( "Failed to Start: The process failed to start.Either the invoked program is missing, or you may have insufficient permissions to invoke the program." );
                case QProcess::Crashed: return QObject::tr( "Crashed: The process crashed some time after starting successfully." );
                case QProcess::Timedout: return QObject::tr( "Timed out: The last waitFor...() function timed out.The state of QProcess is unchanged, and you can try calling waitFor...() again." );
                case QProcess::WriteError: return QObject::tr( "Write Error: An error occurred when attempting to write to the process.For example, the process may not be running, or it may have closed its input channel." );
                case QProcess::ReadError: return QObject::tr( "Read Error: An error occurred when attempting to read from the process.For example, the process may not be running." );
                default:
                case QProcess::UnknownError: return QObject::tr( "Unknown Error" );
            }
        }

        QString statusString( QProcess::ExitStatus error )
        {
            switch ( error )
            {
                case QProcess::NormalExit: return QObject::tr( "Normal Exit: The process exited normally." );
                case QProcess::CrashExit: return QObject::tr( "Crashed: The process crashed." );
                default:
                    return {};
            }
        }

        void CDirModel::slotProgressCanceled()
        {
            fProcess->kill();
        }

        QStringList CDirModel::getMediaHeaders() const
        {
            if ( !canShowMediaInfo() )
                return {};
            return QStringList() << tr( "Title" ) << tr( "Length" ) << tr( "Media Date" ) << tr( "Comment" );
        }

        std::list< SDirNodeItem > CDirModel::addAdditionalItems( const QFileInfo & fileInfo ) const
        {
            if ( showMediaItems() && canShowMediaInfo() )
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

        void CDirModel::processFinished( const QString & msg, bool error )
        {
            if ( fProcessQueue.empty() )
                return;

            if ( log() )
                log()->appendPlainText( (error ? tr( "Error: " ) : QString()) + msg );
            else
                qDebug() << (error ? tr( "Error: " ) : QString()) + msg;

            if ( error )
                addProcessError( msg );

            bool wasCanceled = progressCanceled();
            fProcessResults.first = !error && !wasCanceled;
            fProcessQueue.front().cleanup( this, !error && !wasCanceled );

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
            processFinished( msg, (exitStatus != QProcess::NormalExit) );
        }

        void CDirModel::slotProcessStarted()
        {}

        void CDirModel::slotProcesssStateChanged( QProcess::ProcessState newState )
        {
            (void)newState;
        }

        void SProcessInfo::cleanup( CDirModel * model, bool aOK )
        {
            if ( fOldName.isEmpty() || fNewName.isEmpty() )
                return;

            if ( !aOK )
            {
                QFile::remove( fNewName );
                return;
            }

            if ( !QFileInfo::exists( fNewName ) )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: %1:  New file does not exist" ).arg( model->getDispName( fOldName ) ).arg( model->getDispName( fNewName ) ) );
                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                CDirModel::appendError( fItem, errorItem );

                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                errorItem->setIcon( icon );

                model->fProcessResults.first = false;
                return;
            }

            auto backupName = fOldName + ".bak";
            if ( !QFile::rename( fOldName, backupName ) )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MOVE ITEM TO %2" ).arg( model->getDispName( fOldName ) ).arg( model->getDispName( backupName ) ) );
                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                CDirModel::appendError( fItem, errorItem );

                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                errorItem->setIcon( icon );
                model->fProcessResults.first = false;
                return;
            }

            QString msg;
            if ( fSetMKVTagsOnSuccess && !model->setMediaTags( fNewName, QString(), QString(), &msg ) )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO SET MKV Tags - %2" ).arg( model->getDispName( fNewName ) ).arg( msg ) );
                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                CDirModel::appendError( fItem, errorItem );

                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                errorItem->setIcon( icon );
                model->fProcessResults.first = false;
                return;
            }

            QStringList msgs;
            if ( !model->postExtProcess( *this, msgs ) )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MOVE ITEM TO %2" ).arg( model->getDispName( fNewName ) ).arg( model->getDispName( fOldName ) ) );
                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                CDirModel::appendError( fItem, errorItem );

                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                errorItem->setIcon( icon );
                model->fProcessResults.first = false;
            }

            if ( QFileInfo::exists( fNewName ) && !NSABUtils::NFileUtils::setTimeStamps( fNewName, fTimeStamps ) )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MODIFY TIMESTAMP" ).arg( model->getDispName( fOldName ) ) );
                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                CDirModel::appendError( fItem, errorItem );

                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                errorItem->setIcon( icon );
                model->fProcessResults.first = false;
            }

            if ( QFileInfo::exists( fOldName ) && !NSABUtils::NFileUtils::setTimeStamps( fOldName, fTimeStamps ) )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MODIFY TIMESTAMP" ).arg( model->getDispName( fOldName ) ) );
                errorItem->setData( ECustomRoles::eIsErrorNode, true );
                CDirModel::appendError( fItem, errorItem );

                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                errorItem->setIcon( icon );
                model->fProcessResults.first = false;
            }
        }

        QString CDirModel::getMediaYear( const QFileInfo & fi ) const
        {
            auto searchPath = fi;
            QString year;
            while ( year.isEmpty() && !isRootPath( searchPath.absoluteFilePath() ) )
            {
                NCore::SSearchTMDBInfo searchInfo( searchPath.completeBaseName(), {} );
                if ( searchInfo.releaseDateSet() )
                    year = QString::number( searchInfo.releaseDate().first.year() );
                searchPath = searchPath.absolutePath();
            }
            return year;
        }

        QDate CDirModel::getMediaDate( const QFileInfo & fi ) const
        {
            auto searchPath = fi;
            QDate retVal;
            while ( !retVal.isValid() && !isRootPath( searchPath.absoluteFilePath() ) )
            {
                NCore::SSearchTMDBInfo searchInfo( searchPath.completeBaseName(), {} );
                retVal = searchInfo.releaseDate().first;
                searchPath = searchPath.absolutePath();
            }
            return retVal;
        }

        bool CDirModel::progressCanceled() const
        {
            return progressDlg() && progressDlg()->wasCanceled();
        }

        QPlainTextEdit * CDirModel::log() const
        {
            return fBasePage->log();
        }

        NSABUtils::CDoubleProgressDlg * CDirModel::progressDlg() const
        {
            return fBasePage->progressDlg();
        }

        void CDirModel::postAddItems( const QFileInfo & fileInfo, std::list< SDirNodeItem > & currItems ) const
        {
            (void)fileInfo;
            (void)currItems;
        }

        int CDirModel::lastMediaItemColumn() const
        {
            return getMediaCommentLoc();
        }

        int CDirModel::getMediaTitleLoc() const
        {
            if ( !canShowMediaInfo() )
                return -1;
            return firstMediaItemColumn();
        }

        int CDirModel::getMediaLengthLoc() const
        {
            if ( !canShowMediaInfo() )
                return -1;
            return getMediaTitleLoc() + 1;
        }

        int CDirModel::getMediaDateLoc() const
        {
            if ( !canShowMediaInfo() )
                return -1;
            return getMediaLengthLoc() + 1;
        }

        int CDirModel::getMediaCommentLoc() const
        {
            if ( !canShowMediaInfo() )
                return -1;
            return getMediaDateLoc() + 1;
        }

        QTreeView * CDirModel::filesView() const
        {
            return fBasePage->filesView();
        }


        void CDirModel::slotProcessStandardError()
        {
            fBasePage->appendToLog( fProcess->readAllStandardError(), stdErrRemaining(), false, true );
        }

        void CDirModel::slotProcessStandardOutput()
        {
            fBasePage->appendToLog( fProcess->readAllStandardOutput(), stdOutRemaining(), true, true );
        }

        void CDirModel::resizeColumns() const
        {
            NSABUtils::autoSize( filesView() );
        }

        bool CDirModel::isRootPath( const QString & path ) const
        {
            if ( path.isEmpty() )
                return false;

            return isRootPath( QFileInfo( path ) );
        }

        bool CDirModel::isRootPath( const QModelIndex & idx ) const
        {
            auto fi = fileInfo( idx );
            return isRootPath( fi );
        }

        bool CDirModel::isRootPath( const QFileInfo & path ) const
        {
            if ( path.isFile() )
                return false;

            if ( path.isRelative() )
                return false;

            auto retVal = QFileInfo( fRootPath.absoluteFilePath( "." ) ) == path;
            return retVal;
        }

        void CDirModel::updatePath( const QModelIndex & idx, const QString & oldPath, const QString & newPath )
        {
            if ( QFileInfo( newPath ).isDir() )
                updateDir( idx, oldPath, newPath );
            else
                updateFile( idx, oldPath, newPath );
        }

        void CDirModel::updateFile( const QModelIndex & idx, const QString & /*oldPath*/, const QString & newPath )
        {
            auto nameIndex = index( idx.row(), EColumns::eFSName, idx.parent() );
            setData( nameIndex, newPath, ECustomRoles::eFullPathRole );
            auto item = itemFromIndex( nameIndex );
            fPathMapping[newPath] = item;
        }

        void CDirModel::updateDir( const QModelIndex & idx, const QDir & oldDir, const QDir & newDir )
        {
            auto nameIndex = index( idx.row(), EColumns::eFSName, idx.parent() );
            setData( nameIndex, newDir.absolutePath(), ECustomRoles::eFullPathRole );
            auto item = itemFromIndex( nameIndex );
            fPathMapping[newDir.absolutePath()] = item;

            auto numRows = rowCount( idx );
            for ( int ii = 0; ii < numRows; ++ii )
            {
                auto childIdx = index( ii, EColumns::eFSName, idx );
                auto fileName = QFileInfo( data( childIdx, ECustomRoles::eFullPathRole ).toString() ).fileName();

                auto oldPath = oldDir.absoluteFilePath( fileName );
                auto newPath = newDir.absoluteFilePath( fileName );
                updatePath( childIdx, oldPath, newPath );
            }
        }

        void CDirModel::slotDataChanged( const QModelIndex & start, const QModelIndex & end, const QVector<int> &/*roles*/ )
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

        bool CDirModel::canComputeStatus() const
        {
            return !fLoading;
        }

        std::optional<TItemStatus> CDirModel::getRowStatus( const QModelIndex & idx ) const
        {
            auto parent = idx.parent();
            int numCols = columnCount( parent );
            std::optional< TItemStatus > rowStatus;
            for ( int ii = 0; ii < numCols; ++ii )
            {
                if ( ii == idx.column() )
                    continue;

                auto peerIndex = this->index( idx.row(), ii, parent );
                auto status = getItemStatus( peerIndex );
                if ( status.has_value() )
                {
                    if ( rowStatus.has_value() )
                    {
                        rowStatus.value().first = std::max( rowStatus.value().first, status.value().first );
                        rowStatus.value().second += "\n" + status.value().second;
                    }
                    else
                        rowStatus = status;
                }
            }
            return rowStatus;
        }

        std::optional< TItemStatus > CDirModel::getIndexStatus( const QModelIndex & idx ) const
        {
            auto itemStatus = getItemStatus( idx );
            if ( !itemStatus.has_value() )
            {
                auto fi = fileInfo( idx );
                itemStatus = getPathStatus( fi );
            }

            if ( !itemStatus.has_value() && idx.column() == NModels::EColumns::eFSName )
            {
                // when the filename itself has no result, show the union of all other columns
                itemStatus = getRowStatus( idx );
            }
            return itemStatus;
        }

        std::optional< TItemStatus > CDirModel::computeItemStatus( const QModelIndex & idx ) const
        {
            (void)idx;
            return {};
        }

        std::optional< TItemStatus > CDirModel::getItemStatus( const QModelIndex & idx ) const
        {
            auto fi = fileInfo( idx );

            auto pos = fItemStatusCache.find( fi.absoluteFilePath() );
            if ( useCache() && (pos != fItemStatusCache.end()) )
            {
                auto pos2 = (*pos).second.find( idx.column() );
                if ( pos2 != (*pos).second.end() )
                    return (*pos2).second;
            }

            if ( !canComputeStatus() )
                return {};


            auto retVal = computeItemStatus( idx );
            if ( retVal.has_value() )
            {
                fItemStatusCache[fi.absoluteFilePath()][idx.column()] = retVal.value();
            }
            return retVal;
        }

        std::optional< TItemStatus > CDirModel::computePathStatus( const QFileInfo & fi ) const
        {
            (void)fi;
            return {};
        }

        std::optional< TItemStatus > CDirModel::getPathStatus( const QFileInfo & fi ) const
        {
            auto pos = fPathStatusCache.find( fi.absoluteFilePath() );
            if ( useCache() && (pos != fPathStatusCache.end()) )
                return (*pos).second;

            if ( !canComputeStatus() )
                return {};

            auto retVal = computePathStatus( fi );
            if ( retVal.has_value() )
            {
                fPathStatusCache[fi.absoluteFilePath()] = retVal.value();
            }
            return retVal;
        }

        QVariant CDirModel::getItemBackground( const QModelIndex & idx ) const
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

        QVariant CDirModel::getItemForeground( const QModelIndex & idx ) const
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

        QVariant CDirModel::getToolTip( const QModelIndex & idx ) const
        {
            auto itemStatus = getIndexStatus( idx );
            if ( itemStatus.has_value() )
                return itemStatus.value().second;
            return {};
        }

        QVariant CDirModel::getPathDecoration( const QModelIndex & idx, const QVariant & baseDecoration ) const
        {
            (void)idx;
            return baseDecoration;
        }

        void CDirModel::clearItemStatusCache( const QModelIndex & idx ) const
        {
            auto fi = fileInfo( idx );
            auto pos = fItemStatusCache.find( fi.absoluteFilePath() );
            if ( pos == fItemStatusCache.end() )
                return;

            auto pos2 = (*pos).second.find( idx.column() );
            if ( pos2 == (*pos).second.end() )
                return;

            (*pos).second.erase( pos2 );
        }

        void CDirModel::clearPathStatusCache( const QFileInfo & fi ) const
        {
            auto pos = fPathStatusCache.find( fi.absoluteFilePath() );
            if ( pos != fPathStatusCache.end() )
                fPathStatusCache.erase( pos );
        }

        void CDirModel::clearPathStatusCache( const QString & path ) const
        {
            return clearPathStatusCache( QFileInfo( path ) );
        }

        void CDirModel::resetStatusCaches()
        {
            beginResetModel();
            fPathStatusCache.clear();
            fItemStatusCache.clear();
            endResetModel();
        }

    }
}