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

#ifndef _DIRMODEL_H
#define _DIRMODEL_H

#include "DirNodeItem.h"

#include <QStandardItemModel>
#include <QFileInfo> // filedevice
#include <unordered_set>
#include <optional>
#include <QProcess> // qprocess enums
#include <QMessageBox> // needed for icon type
#include <QDialogButtonBox> // StandardButtons
#include <QDateTime>
#include <QDir>

namespace NSABUtils
{
    class CDoubleProgressDlg;
}

class QTreeView;
class QMediaPlaylist;
class QFileIconProvider;
class QDirIterator;
class QPlainTextEdit;
class QProcess;
class QFileInfo;
class QDir;

namespace NMediaManager
{
    namespace NUi
    {
        class CBasePage;
    }

    namespace NCore
    {
        struct STransformResult;
        class CDirModel;
        enum class EMediaType;
        enum class EItemStatus;

        extern const QString kNoItems;
        extern const QString kNoMatch;
        extern const QString kDeleteThis;

        enum ECustomRoles
        {
            eFullPathRole = Qt::UserRole + 1,
            eIsDir,
            eIsRoot,
            eIsTVShowRole,
            eISOCodeRole,
            eDefaultTrackRole,
            eHearingImparedRole,
            eForcedSubTitleRole,
            eMD5,
            eOldName,
            eNewName,
            eIsErrorNode
        };

        enum class EType
        {
            ePath = QStandardItem::UserType + 1,
            eTitle,
            eLength,
            eDate,
            eComment
        };

        class CDirModelItem : public QStandardItem
        {
        public:
            CDirModelItem( const QString & text, EType type );
            virtual int type() const override { return static_cast< int >( fType ); }
        private:
            EType fType;
        };

        struct STreeNode
        {
            STreeNode() {}
            STreeNode( const QFileInfo & file, const CDirModel * model, bool isRoot );

            ~STreeNode()
            {}

            QString name() const;

            QStandardItem * item( EColumns column, bool createIfNecessary = true ) const;
            QStandardItem * rootItem( bool createIfNecessary = true ) const;
            QList< QStandardItem * > items( bool createIfNecessary = true ) const;
            bool fLoaded{ false };
            bool fIsFile{ false };
        private:
            const CDirModel * fModel{ nullptr };
            std::list< SDirNodeItem > fItems;
            mutable QList< QStandardItem * > fRealItems;
        };

        using TParentTree = std::list< STreeNode >;

        struct SProcessInfo
        {
            SProcessInfo() {}
            void cleanup( CDirModel * model, bool aOK );

            bool fSetMKVTagsOnSuccess{ false };
            QString fCmd;
            QStringList fArgs;
            QStandardItem * fItem{ nullptr };
            QString fOldName;
            QStringList fAncillary;
            QString fNewName;
            std::unordered_map< QFileDevice::FileTime, QDateTime > fTimeStamps;
        };

        using TItemStatus = std::pair< EItemStatus, QString >;
        class CDirModel : public QStandardItemModel
        {
            friend struct SDirNodeItem;
            friend struct STreeNode;
            friend struct SProcessInfo;
            Q_OBJECT
        public:
            CDirModel( NUi::CBasePage * page, QObject * parent = nullptr );
            ~CDirModel();

            bool isDir( const QModelIndex & idx ) const;
            QFileInfo fileInfo( const QModelIndex & idx ) const;
            QString filePath( const QModelIndex & idx ) const;

            bool isDir( const QStandardItem * item ) const;
            QFileInfo fileInfo( const QStandardItem * item ) const;
            QString filePath( const QStandardItem * item ) const;

            QStandardItem * getPathItemFromIndex( const QModelIndex & idx ) const;
            QStandardItem * getItemFromPath( const QFileInfo & fi ) const;

            virtual QVariant data(const QModelIndex & idx, int role) const final;

            bool process( const QModelIndex & idx, const std::function< void( int count, int eventsPerPath ) > & startProgress, const std::function< void( bool finalStep, bool canceled ) > & endProgress, QWidget * parent );

            void setNameFilters( const QStringList & filters );
            void reloadModel();
            void setRootPath( const QString & path );

            virtual bool setData( const QModelIndex & idx, const QVariant & value, int role ) override;

            bool isMediaFile( const QStandardItem * item ) const;
            bool isMediaFile( const QModelIndex & fi ) const;
            bool isMediaFile(const QFileInfo & fi) const;

            bool isSubtitleFile( const QStandardItem * item, bool * isLangFileFormat = nullptr ) const;
            bool isSubtitleFile( const QModelIndex & idx, bool * isLangFileFormat = nullptr ) const;
            bool isSubtitleFile(const QFileInfo & fileInfo, bool * isLangFileFormat) const;

            virtual int eventsPerPath() const { return 1; }

            static bool isAutoSetText( const QString & text );

            const QFileIconProvider * iconProvider() const { return fIconProvider; }
            bool showProcessResults( const QString & title, const QString & label, const QMessageBox::Icon & icon, const QDialogButtonBox::StandardButtons & buttons, QWidget * parent ) const;

            std::pair<QString, bool> & stdOutRemaining() { return fStdOutRemaining; }
            std::pair<QString, bool> & stdErrRemaining() { return fStdErrRemaining; }

            virtual void clear();

            virtual bool showMediaItems() const { return false; };

            bool canShowMediaInfo() const;
            virtual std::unordered_map< QString, QString > getMediaTags( const QFileInfo & fi ) const;
            virtual void reloadMediaTags(const QModelIndex & idx);
            virtual void reloadMediaTags(const QModelIndex & idx, bool force);
            virtual bool autoSetMediaTags( const QModelIndex & idx, QString * msg = nullptr );

            bool setMediaTags( const QString & fileName, const QString & title, const QString & year, QString * msg = nullptr ) const;
            bool setMediaTag( const QString & filename, const std::pair< QString, QString > & tagData, QString * msg = nullptr ) const; //pair => tag, value

            virtual void updatePath( const QModelIndex & idx, const QString & oldPath, const QString & newPath) final;
            virtual void updateFile(const QModelIndex &idx, const QString & oldFile, const QString & newFile);
            virtual void updateDir(const QModelIndex & idx, const QDir & oldDir, const QDir & newDir);

            bool isRootPath( const QString & path ) const;
            bool isRootPath( const QFileInfo & fileInfo ) const;
            bool isRootPath(const QModelIndex & index ) const;
        Q_SIGNALS:
            void sigDirLoadFinished( bool canceled );
            void sigProcessesFinished( bool status, bool showProcessResults, bool cancelled, bool reloadModel );
            void sigProcessingStarted();
        public Q_SLOTS:
            void slotLoadRootDirectory();

            void slotRunNextProcessInQueue();
            void slotProcessErrorOccured( QProcess::ProcessError error );
            void slotProcessFinished( int exitCode, QProcess::ExitStatus exitStatus );
            void slotProcessStandardError();
            void slotProcessStandardOutput();

            void slotProcessStarted();
            void slotProcesssStateChanged( QProcess::ProcessState newState );
            void slotProgressCanceled();
            virtual void slotDataChanged(const QModelIndex & start, const QModelIndex & end, const QVector<int> &roles);

        protected:
            virtual QVariant getItemBackground(const QModelIndex & idx) const final;
            virtual QVariant getItemForeground(const QModelIndex & idx) const final;
            virtual QVariant getToolTip(const QModelIndex & idx) const  final;
            virtual QVariant getPathDecoration(const QModelIndex & idx, const QVariant & baseDecoration) const final;

            virtual bool canComputeStatus() const;
            virtual std::optional< TItemStatus > getIndexStatus(const QModelIndex & idx) const final; // checks the item then the path status

            virtual std::optional< TItemStatus > getItemStatus(const QModelIndex & idx) const final;
            virtual std::optional< TItemStatus > computeItemStatus(const QModelIndex & idx) const;
            virtual std::optional< TItemStatus > getPathStatus(const QFileInfo & fi) const final;
            virtual std::optional< TItemStatus > computePathStatus(const QFileInfo & fi) const;

            virtual void resetStatusCaches();
            virtual void clearItemStatusCache( const QModelIndex & idx ) const; // const due to possible (often) calls in ::data
            virtual void clearPathStatusCache( const QFileInfo & fi ) const;
            virtual void clearPathStatusCache( const QString & path ) const;

            QString getMediaYear(const QFileInfo & fi) const;
            bool progressCanceled() const;

            QPlainTextEdit * log() const;
            QTreeView * filesView() const;
            NSABUtils::CDoubleProgressDlg * progressDlg() const;

            virtual std::pair< bool, QStandardItem * > processItem( const QStandardItem * item, QStandardItem * parentResultItem, bool displayOnly ) = 0;
            virtual void postAddItems( const QFileInfo & fileInfo, std::list< NMediaManager::NCore::SDirNodeItem > & currItems ) const;
            virtual int firstMediaItemColumn() const { return -1; }
            virtual int lastMediaItemColumn() const;
            virtual int getMediaTitleLoc() const;
            virtual int getMediaLengthLoc() const;
            virtual int getMediaDateLoc() const;
            virtual int getMediaCommentLoc() const;
            virtual std::list< NMediaManager::NCore::SDirNodeItem > addAdditionalItems( const QFileInfo & fileInfo ) const;
            virtual std::list<NMediaManager::NCore::SDirNodeItem> getMediaInfoItems(  const QFileInfo & fileInfo, int firstColumn ) const;

            virtual void setupNewItem( const SDirNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item ) const = 0;
            virtual QStringList headers() const;
            virtual void preLoad() final;
            virtual void postLoad( bool aOK ) final;
            virtual void postLoad( QTreeView * treeView );
            virtual void preLoad( QTreeView * treeView );

            virtual bool isLoading() const final { return fLoading; }

            QStringList getMediaHeaders() const;

            virtual void resizeColumns() const;

            virtual void postReloadModelRequest();
            virtual int computeNumberOfItems() const;
            virtual void postProcess( bool /*displayOnly*/ );
            virtual bool postExtProcess( const SProcessInfo & info, QStringList & msgList );
            virtual QString getProgressLabel( const SProcessInfo & processInfo ) const;
            virtual bool usesQueuedProcessing() const = 0;

            // model overrides during iteration
            virtual void postFileFunction( bool aOK, const QFileInfo & fileInfo ) = 0;
            virtual bool preFileFunction( const QFileInfo & fileInfo, std::unordered_set<QString> & alreadyAdded, TParentTree & tree ) = 0;

            virtual void attachTreeNodes( QStandardItem * nextParent, QStandardItem *& prevParent, const STreeNode & treeNode ) = 0;

            virtual QString computeTransformPath( const QStandardItem * item, bool parentsOnly ) const final;
            virtual QString getMyTransformedName( const QStandardItem * item, bool parentsOnly ) const;

            void processFinished( const QString & msg, bool withError );

            void appendRow( QStandardItem * parent, QList< QStandardItem * > & items );
            static void appendError( QStandardItem * parent, QStandardItem * errorNode );

            struct SIterateInfo
            {
                std::function< bool( const QFileInfo & dir ) > fPreDirFunction;
                std::function< void( const QFileInfo & dir, bool aOK ) > fPostDirFunction;
                std::function< bool( const QFileInfo & dir ) > fPreFileFunction;
                std::function< void( const QFileInfo & dir, bool aOK ) > fPostFileFunction;
            };

            void iterateEveryFile( const QFileInfo & fileInfo, const SIterateInfo & iterInfo, std::optional< QDateTime > & lastUpdateUI ) const;
            std::pair< uint64_t, uint64_t > computeNumberOfFiles( const QFileInfo & fileInfo ) const;
            void loadFileInfo( const QFileInfo & info );

            std::unique_ptr< QDirIterator > getDirIteratorForPath( const QFileInfo & fileInfo ) const;

            QStandardItem * attachTreeNodes( TParentTree & parentTree ); // returns the root item (col 0) of the leaf node

            bool isIgnoredPathName( const QFileInfo & ii ) const;
            bool isSkippedDirName( const QFileInfo & ii ) const;

            STreeNode getItemRow( const QFileInfo & path ) const;

            QString getDispName( const QString & absPath ) const;
            QString getDispName( const QFileInfo & absPath ) const;
            void process( const QModelIndex & idx, bool displayOnly );
            bool process( const QStandardItem * item, bool displayOnly, QStandardItem * resultsParentItem );

            bool checkProcessItemExists( const QString & fileName, QStandardItem * parentItem, bool scheduledForRemoval = false ) const;

            QStandardItem * getItem( const QStandardItem * item, int column ) const;

            bool isChecked( const QFileInfo & fileInfo, int column ) const;
            bool isChecked( const QString & path, int column ) const;

            void setChecked( QStandardItem * item, bool value ) const;
            void setChecked( QStandardItem * item, ECustomRoles role, bool value ) const;

            void addProcessError( const QString & msg );

            QDir fRootPath;
            QStringList fNameFilter;

            QFileIconProvider * fIconProvider{ nullptr };

            std::map< QString, QStandardItem * > fPathMapping;

            QTimer * fTimer{ nullptr };
            NUi::CBasePage * fBasePage{ nullptr };
            QProcess * fProcess{ nullptr };
            std::pair< bool, std::shared_ptr< QStandardItemModel > > fProcessResults;

            mutable std::list< SProcessInfo > fProcessQueue;
            std::pair< QString, bool > fStdOutRemaining{ QString(),false };
            std::pair< QString, bool > fStdErrRemaining{ QString(),false };

            bool fProcessFinishedHandled{ false };
            mutable bool fFirstProcess{ true };
            mutable bool fLoading{ false };

        private:
            mutable std::unordered_map< QString, TItemStatus > fPathStatusCache;
            mutable std::unordered_map< QString, std::unordered_map< int, TItemStatus > > fItemStatusCache;
        };
    }
}

QDebug operator<<( QDebug dbg, const NMediaManager::NCore::STreeNode & node );
QDebug operator<<( QDebug dbg, const NMediaManager::NCore::TParentTree & parentTree );
#endif

