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

#ifndef _DIRMODEL_H
#define _DIRMODEL_H

#include <QStandardItemModel>
#include <QRegularExpression>
#include <QFileInfo>
#include <memory>
#include <unordered_set>
#include <optional>
#include <QProcess>
#include <QDateTime>
#include <QMessageBox>
#include <QDialogButtonBox>
#include "SABUtils/HashUtils.h"

class CDoubleProgressDlg;
class QTreeView;
class QMediaPlaylist;
class QFileIconProvider;
class QDirIterator;
class QPlainTextEdit;
class QProcess;
namespace NUtils
{
    class CStayAwake;
}

namespace NMediaManager
{
    namespace NCore
    {
        struct SSearchResult;
        class CDirModel;

        enum EColumns
        {
            eFSName,
            eFSSize,
            eFSType,
            eFSModDate,
            eIsTVShow,
            eTransformName,
            eLanguage = eIsTVShow,
            eForced = eTransformName,
            eSDH,
            eOnByDefault
        };
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

        struct STreeNodeItem
        {
            STreeNodeItem() {}
            STreeNodeItem( const QString & text, EColumns nodeType ) : fText( text ), fType( nodeType ) {}

            void setData( QVariant value, int role )
            {
                return fRoles.push_back( std::make_pair( value, role ) );
            }

            QStandardItem * createStandardItem() const;

            QString fText;
            EColumns fType{ EColumns::eFSName };
            QIcon fIcon;
            std::optional< Qt::Alignment > fAlignment;
            std::list< std::pair< QVariant, int > > fRoles;
            bool fIsTVShow{ false };
            std::optional< bool > fCheckable;
        };

        struct STreeNode
        {
            STreeNode() {}
            STreeNode( const QFileInfo & file, const CDirModel * model );

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
            std::list< STreeNodeItem > fItems;
            mutable QList< QStandardItem * > fRealItems;
        };

        using TParentTree = std::list< STreeNode >;

        struct SProcessInfo
        {
            SProcessInfo() {}
            void cleanup( CDirModel * model, bool aOK );

            QString fCmd;
            QStringList fArgs;
            QStandardItem * fItem{ nullptr };
            QString fOldName;
            QStringList fAncillary;
            QString fNewName;
            std::unordered_map< QFileDevice::FileTime, QDateTime > fTimeStamps;
        };

        class CDirModel : public QStandardItemModel
        {
            friend struct STreeNodeItem;
            friend struct STreeNode;
            friend struct SProcessInfo;
            Q_OBJECT
        public:
            CDirModel( QObject * parent = nullptr );
            ~CDirModel();

            bool isDir( const QModelIndex & idx ) const;
            QFileInfo fileInfo( const QModelIndex & idx ) const;
            QString filePath( const QModelIndex & idx ) const;

            bool isDir( const QStandardItem * item ) const;
            QFileInfo fileInfo( const QStandardItem * item ) const;
            QString filePath( const QStandardItem * item ) const;

            QStandardItem * getItemFromindex( QModelIndex idx ) const;
            QStandardItem * getItemFromPath( const QFileInfo & fi ) const;

            bool process( const std::function< std::shared_ptr< CDoubleProgressDlg >( int count ) > & startProgress, const std::function< void( std::shared_ptr< CDoubleProgressDlg > ) > & endProgress, QWidget * parent );

            void setNameFilters( const QStringList & filters, QTreeView * view = nullptr, QPlainTextEdit * resultsView = nullptr, std::shared_ptr< CDoubleProgressDlg >progress = {} );
            void reloadModel( QTreeView * view, QPlainTextEdit * resultsView, std::shared_ptr< CDoubleProgressDlg > dlg );
            void setRootPath( const QString & path, QTreeView * view = nullptr, QPlainTextEdit * resultsView = nullptr, std::shared_ptr< CDoubleProgressDlg > dlg = {} );

            QString getSearchName( const QModelIndex & idx ) const;
            virtual bool setData( const QModelIndex & idx, const QVariant & value, int role ) override;

            bool isMediaFile( const QStandardItem * item ) const;
            bool isMediaFile( const QModelIndex & fi ) const;

            bool isSubtitleFile( const QStandardItem * item, bool * isLangFileFormat = nullptr ) const;
            bool isSubtitleFile( const QModelIndex & idx, bool * isLangFileFormat = nullptr ) const;

            bool canAutoSearch( const QModelIndex & index ) const;
            bool canAutoSearch( const QFileInfo & info ) const;
            virtual int eventsPerPath() const { return 1; }

            static bool isAutoSetText( const QString & text );

            const QFileIconProvider * iconProvider() const { return fIconProvider; }
            bool showProcessResults( const QString & title, const QString & label, const QMessageBox::Icon & icon, const QDialogButtonBox::StandardButtons & buttons, QWidget * parent ) const;

            std::pair<QString, bool> & stdOutRemaining() { return fStdOutRemaining; }
        Q_SIGNALS:
            void sigDirReloaded( bool canceled );
            void sigProcessesFinished( bool status, bool cancelled, bool reloadModel );
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
        protected:
            virtual std::pair< bool, QStandardItem * > processItem( const QStandardItem * item, QStandardItem * parentItem, bool displayOnly ) const = 0;
            virtual void preAddItems( const QFileInfo & fileInfo, std::list< NMediaManager::NCore::STreeNodeItem > & currItems ) const = 0;
            virtual std::list< NMediaManager::NCore::STreeNodeItem > addItems( const QFileInfo & fileInfo ) const = 0;
            virtual void setupNewItem( const STreeNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item ) const = 0;
            virtual QStringList headers() const;
            virtual void postLoad() const final;
            virtual void postLoad( QTreeView * treeView ) const = 0;
            virtual void postReloadModel();
            virtual int computeNumberOfItems() const = 0;
            virtual void postProcess( bool /*displayOnly*/ );
            virtual bool postExtProcess( const SProcessInfo & info, QStringList & msgList );

            virtual QString getProgressLabel( const SProcessInfo & processInfo ) const;

            // model overrides during iteration
            virtual void postFileFunction( bool aOK, const QFileInfo & fileInfo ) = 0;
            virtual bool preFileFunction( const QFileInfo & fileInfo, std::unordered_set<QString> & alreadyAdded, TParentTree & tree ) = 0;

            virtual void attachTreeNodes( QStandardItem * nextParent, QStandardItem * prevParent, const STreeNode & treeNode ) = 0;

            virtual QString computeTransformPath( const QStandardItem * item, bool parentsOnly ) const final;
            virtual QString getMyTransformedName( const QStandardItem * item, bool parentsOnly ) const;

            void processFinished( const QString & msg, bool withError );
            bool SetMKVTags( const QString & fileName, std::shared_ptr< SSearchResult > & searchResults, QString & msg ) const;

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
            bool isExcludedDirName( const QFileInfo & ii ) const;

            STreeNode getItemRow( const QFileInfo & path ) const;

            QString getDispName( const QString & absPath ) const;
            QString getDispName( const QFileInfo & absPath ) const;
            void process( bool displayOnly );
            bool process( const QStandardItem * item, bool displayOnly, QStandardItem * resultsParentItem );

            bool checkProcessItemExists( const QString & fileName, QStandardItem * parentItem, bool scheduledForRemoval = false ) const;

            QStandardItem * getItem( const QStandardItem * item, EColumns column ) const;

            bool isChecked( const QFileInfo & fileInfo, EColumns column ) const;
            bool isChecked( const QString & path, EColumns column ) const;

            void setChecked( QStandardItem * item, bool value ) const;
            void setChecked( QStandardItem * item, ECustomRoles role, bool value ) const;

            void addProcessError( const QString & msg );

            QString fRootPath;
            QStringList fNameFilter;

            std::unordered_set< QString > fExcludedDirNames;
            std::unordered_set< QString > fIgnoredNames;
            QFileIconProvider * fIconProvider{ nullptr };

            std::map< QString, QStandardItem * > fPathMapping;

            QTimer * fTimer{ nullptr };
            QTreeView * fTreeView{ nullptr };
            QPlainTextEdit * fResults{ nullptr };
            QProcess * fProcess{ nullptr };
            std::pair< bool, std::shared_ptr< QStandardItemModel > > fProcessResults;

            mutable std::list< SProcessInfo > fProcessQueue;
            std::pair< QString, bool > fStdOutRemaining{ QString(),false };
            std::pair< QString, bool > fStdErrRemaining{ QString(),false };
            mutable std::shared_ptr< CDoubleProgressDlg > fProgressDlg;

            bool fProcessFinishedHandled{ false };
            mutable bool fFirstProcess{ true };
        };
    }
}
#endif

