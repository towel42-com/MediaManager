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
#include "SABUtils/HashUtils.h"

class QProgressDialog;
class QTreeView;
class QMediaPlaylist;
class QFileIconProvider;
class QDirIterator;
class QPlainTextEdit;
class QProcess;
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
            eMD5
        };

        struct STreeNodeItem
        {
            STreeNodeItem() {}
            STreeNodeItem( const QString &text, EColumns nodeType ) : fText( text ), fType( nodeType ) {}

            void setData( QVariant value, int role )
            {
                return fRoles.push_back( std::make_pair( value, role ) );
            }

            QStandardItem *createStandardItem() const;

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
            STreeNode( const QFileInfo &file, const CDirModel *model );

            ~STreeNode()
            {
            }

            QString name() const;

            QStandardItem *item( EColumns column, bool createIfNecessary = true ) const;
            QStandardItem *rootItem( bool createIfNecessary = true ) const;
            QList< QStandardItem * > items( bool createIfNecessary = true ) const;
            bool fLoaded{ false };
            bool fIsFile{ false };
        private:
            const CDirModel *fModel{ nullptr };
            std::list< STreeNodeItem > fItems;
            mutable QList< QStandardItem * > fRealItems;
        };

        using TParentTree = std::list< STreeNode >;

        struct SPatternInfo
        {
            friend class CDirModel;
            bool isValidName( const QString &name, bool isDir ) const;
            bool isValidName( const QFileInfo &fi ) const;

        private:
            QString fOutFilePattern;
            QString fOutDirPattern;
        };

        class CDirModel : public QStandardItemModel
        {
            friend struct STreeNodeItem;
            friend struct STreeNode;
            Q_OBJECT
        public:
            enum EModelType
            {
                eUnknown,
                eTransform,
                eMergeSRT
            };
            CDirModel( EModelType modelType, QObject *parent = nullptr );
            ~CDirModel();

            bool isMergeSRTModel() const;
            bool isTransformModel() const;

            bool isDir( const QModelIndex &idx ) const;
            QFileInfo fileInfo( const QModelIndex &idx ) const;
            QString filePath( const QModelIndex &idx ) const;

            bool isDir( const QStandardItem *item ) const;
            QFileInfo fileInfo( const QStandardItem *item ) const;
            QString filePath( const QStandardItem *item ) const;

            QStandardItem *getItemFromindex( QModelIndex idx ) const;
            QStandardItem *getItemFromPath( const QFileInfo &fi ) const;

            bool process( const std::function< QProgressDialog *( int count ) > &startProgress, const std::function< void( QProgressDialog * ) > &endProgress, QWidget *parent );
            void setSearchResult( const QModelIndex &idx, std::shared_ptr< SSearchResult > info, bool applyToChilren );
            void setSearchResult( QStandardItem *item, std::shared_ptr< SSearchResult > info, bool applyToChilren );
            std::shared_ptr< SSearchResult > getSearchResultInfo( const QModelIndex &idx ) const;

            void setNameFilters( const QStringList &filters, QTreeView *view = nullptr, QPlainTextEdit * resultsView = nullptr, QProgressDialog *progress = nullptr );
            void reloadModel( QTreeView *view, QPlainTextEdit *resultsView, QProgressDialog *dlg );
            void setRootPath( const QString &path, QTreeView *view = nullptr, QPlainTextEdit *resultsView = nullptr, QProgressDialog *dlg = nullptr );

            QString getSearchName( const QModelIndex &idx ) const;
            bool treatAsTVShow( const QFileInfo &fileInfo, bool defaultValue ) const;
            virtual bool setData( const QModelIndex &idx, const QVariant &value, int role ) override;

            bool isMediaFile( const QStandardItem *item ) const;
            bool isMediaFile( const QModelIndex &fi ) const;

            bool isSubtitleFile( const QStandardItem *item, bool *isLangFileFormat = nullptr ) const;
            bool isSubtitleFile( const QModelIndex &idx, bool *isLangFileFormat = nullptr ) const;

            bool canAutoSearch( const QModelIndex &index ) const;
            bool canAutoSearch( const QFileInfo &info ) const;
            int eventsPerPath() const;// 4 events, get timestamp, create parent paths, rename, setting tag info, settimestamp}

            static bool isAutoSetText( const QString &text );

            const QFileIconProvider *iconProvider() const { return fIconProvider; }
        Q_SIGNALS:
            void sigDirReloaded( bool canceled );
            void sigProcessesFinished( bool cancelled );
        public Q_SLOTS:
            void slotTVOutputFilePatternChanged( const QString &outPattern );
            void slotTVOutputDirPatternChanged( const QString &outPattern );
            void slotMovieOutputDirPatternChanged( const QString &outPattern );
            void slotMovieOutputFilePatternChanged( const QString &outPattern );
            void slotLoadRootDirectory();
            void slotPatternChanged();
            void slotTreatAsTVByDefaultChanged( bool treatAsTVShowByDefault );

            void slotRunNextProcessInQueue();
            void slotProcessErrorOccured(QProcess::ProcessError error);
            void slotProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
            void slotProcessStandardError();
            void slotProcessStandardOutput();
            void slotProcessStarted();
            void slotProcesssStateChanged(QProcess::ProcessState newState);
            void slotProgressCanceled();
        private:
            void processFinished(const QString & msg, bool withError );
            bool SetMKVTags(const QString & fileName, std::shared_ptr< SSearchResult > & searchResults, QString & msg) const;
            QList< QFileInfo > getSRTFilesForMKV(const QFileInfo & fi) const;

            void autoDetermineLanguageAttributes( QStandardItem *parent );
            
            std::unordered_map< QString, std::vector< QStandardItem * > > getChildSRTFiles( const QStandardItem *item, bool sort ) const; // item should be a MKV file
            QList< QStandardItem * > getChildMKVFiles( const QStandardItem *item, bool goBelowDirs) const; // item should be a dir file

            void appendRow( QStandardItem *parent, QList< QStandardItem * > &items );

            struct SIterateInfo
            {
                std::function< bool( const QFileInfo &dir ) > fPreDirFunction;
                std::function< void( const QFileInfo &dir, bool aOK ) > fPostDirFunction;
                std::function< bool( const QFileInfo &dir ) > fPreFileFunction;
                std::function< void( const QFileInfo &dir, bool aOK ) > fPostFileFunction;
            };

            void CDirModel::iterateEveryFile( const QFileInfo &fileInfo, const SIterateInfo &iterInfo, std::optional< QDateTime > & lastUpdateUI ) const;
            std::pair< uint64_t, uint64_t > computeNumberOfFiles( const QFileInfo &fileInfo ) const;
            void loadFileInfo( const QFileInfo &info );

            std::unique_ptr< QDirIterator > getDirIteratorForPath( const QFileInfo &fileInfo ) const;

            QStandardItem * attachTreeNodes( TParentTree &parentTree ); // returns the root item (col 0) of the leaf node

            bool isIgnoredPathName( const QFileInfo &ii ) const;
            bool isExcludedDirName( const QFileInfo &ii ) const;

            STreeNode getItemRow( const QFileInfo &path ) const;

            QString getDispName( const QString &absPath ) const;
            QString getDispName( const QFileInfo & absPath ) const;
            std::pair< bool, QStandardItemModel * > process( bool displayOnly ) const;
            bool process( const QStandardItem *item, bool displayOnly, QStandardItemModel *resultsModel, QStandardItem *resultsParentItem ) const;

            std::pair< bool, QStandardItem * > processItem( const QStandardItem *item, QStandardItem *parentItem, QStandardItemModel *resultModel, bool displayOnly ) const;
            std::pair< bool, QStandardItem * > transform( const QStandardItem *item, QStandardItem *parentItem, QStandardItemModel *resultModel, bool displayOnly ) const;
            std::pair< bool, QStandardItem * > mergeSRT( const QStandardItem *item, QStandardItem *parentItem, QStandardItemModel *resultModel, bool displayOnly ) const;

            bool checkProcessItemExists( const QString & fileName, QStandardItem * parentItem, bool scheduledForRemoval=false ) const;

            QStandardItem *getItem( const QStandardItem *item, EColumns column ) const;
            QStandardItem *getTransformItem( const QStandardItem *parent ) const;
            QStandardItem *getLanguageItem( const QStandardItem *parent ) const;

            bool isValidName( const QFileInfo &fi ) const;
            bool isValidName( const QString &absPath, bool isDir, std::optional< bool > isTVShow ) const;

            bool isChecked( const QFileInfo &fileInfo, EColumns column ) const;
            bool isChecked( const QString &path, EColumns column ) const;

            void transformPatternChanged();
            void transformPatternChanged( const QStandardItem *parent );

            void updateTransformPattern( const QStandardItem *item ) const;
            void updateTransformPattern( const QStandardItem *transformedItem, QStandardItem *item ) const;

            [[nodiscard]] std::pair< bool, QString > transformItem( const QFileInfo &path ) const;
            [[nodiscard]] std::pair< bool, QString > transformItem( const QFileInfo &fileInfo, const SPatternInfo &patternInfo ) const;

            [[nodiscard]] QString computeTransformPath( const QStandardItem *item, bool parentsOnly ) const;

            void setChecked( QStandardItem *item, bool value ) const;
            void setChecked( QStandardItem *item, ECustomRoles role, bool value ) const;

            void addProcessError( const QString & msg );

            EModelType fModelType { EModelType::eUnknown };
            QString fRootPath;
            QStringList fNameFilter;

            SPatternInfo fTVPatterns;
            SPatternInfo fMoviePatterns;

            std::unordered_set< QString > fExcludedDirNames;
            std::unordered_set< QString > fIgnoredNames;
            QFileIconProvider *fIconProvider{ nullptr };

            mutable std::map< QString, std::pair< bool, QString > > fFileMapping;
            mutable std::map< QString, std::pair< bool, QString > > fDirMapping;
            std::map< QString, std::shared_ptr< SSearchResult > > fSearchResultMap;
            std::map< QString, QStandardItem * > fPathMapping;

            bool fTreatAsTVShowByDefault{ false };
            QTimer *fTimer{ nullptr };
            QTimer *fPatternTimer{ nullptr };
            QTreeView *fTreeView{ nullptr };
            QPlainTextEdit *fResults{ nullptr };
            QProcess * fProcess{ nullptr };
            struct SProcessInfo
            {
                SProcessInfo(){}
                void cleanup( bool aOK );

                QString fCmd;
                QStringList fArgs;
                QStandardItem * fItem{ nullptr };
                QString fOldName;
                QStringList fSrtFiles;
                QString fNewName;
                std::unordered_map< QFileDevice::FileTime, QDateTime > fTimeStamps;
            };
            mutable std::list< SProcessInfo > fProcessQueue;
            std::pair< QString, bool > fStdOutRemaining{ QString(),false };
            std::pair< QString, bool > fStdErrRemaining{ QString(),false };
            mutable QProgressDialog *fProgressDlg{ nullptr };
            bool fProcessFinishedHandled{ false };
        };
    }
}
#endif // 
