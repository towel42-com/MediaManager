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
#include "TransformResult.h"
#include "SearchTMDBInfo.h"
#include "Preferences.h"

#include "UI/ProcessConfirm.h"
#include "UI/BasePage.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/FileCompare.h"

#include "SABUtils/DoubleProgressDlg.h"

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

#include <QDirIterator>
#include <QPlainTextEdit>
#include <QMessageBox>
#include <QTemporaryFile>

#include <QProcess>

#include <set>
#include <list>
#include "SABUtils/MD5.h"

QDebug operator<<( QDebug dbg, const NMediaManager::NCore::STreeNode & node )
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

QDebug operator<<( QDebug dbg, const NMediaManager::NCore::TParentTree & parentTree )
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
    namespace NCore
    {
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
        }

        CDirModel::~CDirModel()
        {
            delete fIconProvider;
        }

        void CDirModel::setRootPath( const QString & rootPath )
        {
            fRootPath = rootPath;
            reloadModel();
        }

        bool CDirModel::isAutoSetText( const QString & text )
        {
            return (text == "<NO MATCH>") || (text == "<NO AUTO MATCH>");
        }

        QString CDirModel::getSearchName( const QModelIndex & idx ) const
        {
            auto nm = index( idx.row(), EColumns::eTransformName, idx.parent() ).data().toString();
            if ( isAutoSetText( nm ) || nm.isEmpty() )
            {
                nm = index( idx.row(), EColumns::eFSName, idx.parent() ).data( ECustomRoles::eFullPathRole ).toString();
                nm = nm.isEmpty() ? QString() : (QFileInfo( nm ).isDir() ? QFileInfo( nm ).fileName() : QFileInfo( nm ).completeBaseName());
            }
            return nm;
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
            postReloadModel();
        }

        void CDirModel::slotLoadRootDirectory()
        {
            NSABUtils::CAutoWaitCursor awc;

            clear();

            setHorizontalHeaderLabels( headers() );

            QFileInfo rootFI( fRootPath );

            if ( progressDlg() )
            {
                progressDlg()->setLabelText( tr( "Computing number of Files under '%1'" ).arg( fRootPath ) );
                qApp->processEvents();
                auto numFiles = computeNumberOfFiles( rootFI ).second;
                if ( !progressDlg()->wasCanceled() )
                    progressDlg()->setRange( 0, numFiles );
            }
            if ( progressDlg() && progressDlg()->wasCanceled() )
            {
                emit sigDirReloaded( true );
                return;
            }

            loadFileInfo( rootFI );

            postLoad();

            emit sigDirReloaded( progressDlg() && progressDlg()->wasCanceled() );
        }

        void CDirModel::postLoad() const
        {
            if ( !filesView() )
                return;

            filesView()->resizeColumnToContents( EColumns::eFSName );
            filesView()->resizeColumnToContents( EColumns::eFSSize );
            filesView()->resizeColumnToContents( EColumns::eFSType );
            filesView()->resizeColumnToContents( EColumns::eFSModDate );
            postLoad( filesView() );
        }

        void CDirModel::postReloadModel()
        {
        }

        bool CDirModel::setData( const QModelIndex & idx, const QVariant & value, int role )
        {
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

            if ( progressDlg() && progressDlg()->wasCanceled() )
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
            info.fPreDirFunction = [ &numDirs ]( const QFileInfo & /*dir*/ ) { numDirs++; return true; };
            info.fPreFileFunction = [ &numFiles ]( const QFileInfo & /*file*/ ) { numFiles++; return false; };
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
                    filesView()->resizeColumnToContents( EColumns::eFSName );
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
            return NCore::CPreferences::instance()->isSkippedPath( ii );
        }

        bool CDirModel::isIgnoredPathName( const QFileInfo & ii ) const
        {
            return NCore::CPreferences::instance()->isIgnoredPath( ii );
        }

        STreeNode CDirModel::getItemRow( const QFileInfo & fileInfo ) const
        {
            return STreeNode( fileInfo, this );
        }

        QStandardItem * STreeNodeItem::createStandardItem() const
        {
            auto retVal = new QStandardItem( fText );
            retVal->setIcon( fIcon );
            if ( fAlignment.has_value() )
                retVal->setTextAlignment( fAlignment.value() );
            for ( auto && ii : fRoles )
            {
                retVal->setData( ii.first, ii.second );
            }
            if ( fCheckable.has_value() )
                retVal->setCheckable( fCheckable.value() );
            return retVal;
        }

        STreeNode::STreeNode( const QFileInfo & fileInfo, const CDirModel * model ) :
            fModel( model )
        {
            fIsFile = fileInfo.isFile();
            auto nameItem = STreeNodeItem( fileInfo.fileName(), EColumns::eFSName );
            nameItem.fIcon = model->iconProvider()->icon( fileInfo );
            nameItem.setData( fileInfo.absoluteFilePath(), ECustomRoles::eFullPathRole );
            nameItem.setData( fileInfo.isDir(), ECustomRoles::eIsDir );
            fItems.push_back( nameItem );
            fItems.push_back( STreeNodeItem( fileInfo.isFile() ? NSABUtils::NFileUtils::fileSizeString( fileInfo ) : QString(), EColumns::eFSSize ) );
            if ( fileInfo.isFile() )
            {
                fItems.back().fAlignment = Qt::AlignRight | Qt::AlignVCenter;
            }
            fItems.push_back( STreeNodeItem( model->iconProvider()->type( fileInfo ), EColumns::eFSType ) );
            fItems.push_back( STreeNodeItem( fileInfo.lastModified().toString( "MM/dd/yyyy hh:mm:ss.zzz" ), EColumns::eFSModDate ) );

            model->preAddItems( fileInfo, fItems );
            auto modelItems = model->addItems( fileInfo );
            fItems.insert( fItems.end(), modelItems.begin(), modelItems.end() );
        }

        QString STreeNode::name() const
        {
            return rootItem() ? rootItem()->text() : "<NO ITEMS>";
        }

        QStandardItem * STreeNode::item( EColumns column, bool createIfNecessary /*= true */ ) const
        {
            items( createIfNecessary );
            return (column >= fRealItems.count()) ? nullptr : fRealItems[column];
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

        QFileInfo CDirModel::fileInfo( const QStandardItem * item ) const
        {
            if ( !item )
                return QFileInfo();
            return QFileInfo( item->data( ECustomRoles::eFullPathRole ).toString() );
        }

        QStandardItem * CDirModel::getPathItemFromIndex( QModelIndex idx ) const
        {
            if ( idx.column() != EColumns::eFSName )
            {
                idx = idx.model()->index( idx.row(), EColumns::eFSName, idx.parent() );
            }
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
            return item && CPreferences::instance()->isMediaFile( QFileInfo( item->data( ECustomRoles::eFullPathRole ).toString() ) );
        }

        bool CDirModel::isMediaFile( const QModelIndex & idx ) const
        {
            auto path = idx.data( ECustomRoles::eFullPathRole ).toString();
            if ( path.isEmpty() )
                return false;

            return CPreferences::instance()->isMediaFile( QFileInfo( path ) );
        }

        bool CDirModel::isSubtitleFile( const QModelIndex & idx, bool * isLangFileFormat ) const
        {
            auto path = idx.data( ECustomRoles::eFullPathRole ).toString();
            if ( path.isEmpty() )
                return false;

            return CPreferences::instance()->isSubtitleFile( QFileInfo( path ), isLangFileFormat );
        }

        bool CDirModel::isSubtitleFile( const QStandardItem * item, bool * isLangFileFormat /*= nullptr */ ) const
        {
            return item && CPreferences::instance()->isSubtitleFile( item->data( ECustomRoles::eFullPathRole ).toString(), isLangFileFormat );
        }

        bool CDirModel::canAutoSearch( const QModelIndex & index ) const
        {
            auto path = index.data( ECustomRoles::eFullPathRole ).toString();
            if ( path.isEmpty() )
                return false;
            return canAutoSearch( QFileInfo( path ) );
        }

        bool CDirModel::canAutoSearch( const QFileInfo & fileInfo ) const
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

        QString CDirModel::getMyTransformedName( const QStandardItem * item, bool /*transformParentsOnly*/ ) const
        {
            return item->text();
        }

        QString CDirModel::computeTransformPath( const QStandardItem * item, bool transformParentsOnly ) const
        {
            if ( !item || (item == invisibleRootItem()) )
                return QString();
            if ( item->data( ECustomRoles::eIsRoot ).toBool() )
                return item->data( ECustomRoles::eFullPathRole ).toString();

            auto parentDir = computeTransformPath( item->parent(), false );
            if ( parentDir == "<DELETE THIS>" )
                return parentDir;

            auto myName = getMyTransformedName( item, transformParentsOnly );

            if ( myName.isEmpty() || parentDir.isEmpty() )
                return QString();

            if ( myName == "<DELETE THIS>" )
                return myName;

            auto retVal = QDir( parentDir ).absoluteFilePath( myName );
            return retVal;
        }

        QString CDirModel::getDispName( const QString & absPath ) const
        {
            if ( fRootPath.isEmpty() )
                return QString();
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
            else
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

                if ( progressDlg() && progressDlg()->wasCanceled() )
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

        bool CDirModel::process( const QModelIndex & idx, const std::function< void( int count, int eventsPerPath ) > & startProgress, const std::function< void( bool finalStep ) > & endProgress, QWidget * parent )
        {
            startProgress( 0, 1 );
            process( idx, true );
            if ( fProcessResults.second && fProcessResults.second->rowCount() == 0 )
            {
                QMessageBox::information( parent, tr( "Nothing to change" ), tr( "No files or directories could be processed" ) );
                endProgress( true );
                return false;
            }

            bool continueOn = showProcessResults( tr( "Process:" ), tr( "Proceed?" ), QMessageBox::Information, QDialogButtonBox::Yes | QDialogButtonBox::No, parent );
            endProgress( false );
            if ( !continueOn )
            {
                emit sigProcessesFinished( false, true, false );
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
            endProgress( !usesQueuedProcessing() );
            return fProcessResults.first;
        }

        QStandardItem * CDirModel::getItem( const QStandardItem * item, EColumns column ) const
        {
            auto idx = indexFromItem( item );
            if ( !idx.isValid() )
                return nullptr;

            auto retVal = itemFromIndex( index( idx.row(), column, idx.parent() ) );
            return retVal;
        }

        bool CDirModel::isChecked( const QFileInfo & fileInfo, EColumns column ) const
        {
            return isChecked( fileInfo.absoluteFilePath(), column );
        }

        bool CDirModel::isChecked( const QString & path, EColumns column ) const
        {
            auto item = getItemFromPath( path );
            auto colItem = getItem( item, column );
            if ( !colItem )
                return false;
            return item->checkState() != Qt::Unchecked;
        }

        bool CDirModel::SetMKVTags( const QString & fileName, QString title, const QString & year, QString * msg ) const
        {
            Q_INIT_RESOURCE( core );
            if ( !QFileInfo( fileName ).isFile() )
                return true;

            auto mkvpropedit = CPreferences::instance()->getMKVPropEditEXE();
            if ( !QFileInfo( mkvpropedit ).isExecutable() )
            {
                if ( msg )
                    *msg = tr( "MKVPropEdit not found or is not an executable" );
                return false;
            }

            auto file = QFile( ":/resources/BlankTags.xml" );
            if ( !file.open( QFile::ReadOnly ) )
            {
                if ( msg )
                    *msg = tr( "Internal error, could not open blank tags file" );
                return false;
            }

            if ( title.isEmpty() )
                title = QFileInfo( fileName ).completeBaseName();

            auto xml = file.readAll();
            xml.replace( QByteArray( "%TITLE%" ), title.toUtf8() );
            xml.replace( QByteArray( "%YEAR%" ), year.toUtf8() );

            auto templateName = QDir( QDir::tempPath() ).absoluteFilePath( "XXXXXX.xml" );
            QTemporaryFile tmpFile( templateName );
            auto tmplate = tmpFile.fileTemplate();
            if ( !tmpFile.open() )
            {
                if ( msg )
                    *msg = tr( "Internal error, could not open blank tags file" );
                return false;
            }

            tmpFile.write( xml );
            auto tmpFileName = tmpFile.fileName();
            tmpFile.close();

            auto args = QStringList()
                << fileName
                << "--tags"
                << QString( "global:%1" ).arg( tmpFileName )
                << "--edit"
                << "info"
                << "--set"
                << QString( "title=%2" ).arg( title )
                ;
            auto retVal = QProcess::execute( mkvpropedit, args );

            if ( retVal == -1 )
            {
                if ( msg )
                    *msg = "MKVPropEdit crashed";
            }
            else if ( retVal == -2 )
            {
                if ( msg )
                    *msg = "MKVPropEdit could not be started";
            }
            else if ( retVal != 0 )
            {
                if ( msg )
                    *msg = "MKVPropEdit returned with an unknown error";
            }

            return retVal == 0;
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
                emit sigProcessesFinished( fProcessResults.first, false, true );
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
            return QString();
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
            return QString();
            }
        }

        void CDirModel::slotProgressCanceled()
        {
            fProcess->kill();
        }

        QStringList CDirModel::headers() const
        {
            return QStringList() << tr( "Name" ) << tr( "Size" ) << tr( "Type" ) << tr( "Date Modified" );
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

            bool wasCanceled = progressDlg() && progressDlg()->wasCanceled();
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
        {
        }

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

            if ( fSetMKVTagsOnSuccess && !model->SetMKVTags( fNewName ) )
            {
                auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO SET MKV Tags" ).arg( model->getDispName( fNewName ) ) );
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

        QPlainTextEdit * CDirModel::log() const
        {
            return fBasePage->log();
        }
        
        NSABUtils::CDoubleProgressDlg * CDirModel::progressDlg() const
        {
            return fBasePage->progressDlg();
        }

        QTreeView * CDirModel::filesView() const
        {
            return fBasePage->filesView();
        }


        void CDirModel::slotProcessStandardError()
        {
            fBasePage->appendToLog( fProcess->readAllStandardError(), stdErrRemaining(), false );
        }

        void CDirModel::slotProcessStandardOutput()
        {
            fBasePage->appendToLog( fProcess->readAllStandardOutput(), stdOutRemaining(), true );
        }
    }
}
