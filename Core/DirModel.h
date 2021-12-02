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
#include "SABUtils/HashUtils.h"

class QProgressDialog;
class QTreeView;
class QMediaPlaylist;
class QFileIconProvider;
class QDirIterator;
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
            eISOCodeRole
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

            QString name() const
            {
                return rootItem() ? rootItem()->text() : "<NO ITEMS>";
            }

            QStandardItem *item( EColumns column, bool createIfNecessary = true ) const
            {
                items( createIfNecessary );
                return ( column > fRealItems.count() ) ? nullptr : fRealItems[column];
            }
            QStandardItem *rootItem( bool createIfNecessary = true ) const
            {
                return item( static_cast<EColumns>( 0 ), createIfNecessary );
            }
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

            bool process( const std::function< QProgressDialog *( int count ) > &startProgress, const std::function< void( QProgressDialog * ) > &endProgress, QWidget *parent ) const;
            void setSearchResult( const QModelIndex &idx, std::shared_ptr< SSearchResult > info, bool applyToChilren );
            void setSearchResult( QStandardItem *item, std::shared_ptr< SSearchResult > info, bool applyToChilren );
            std::shared_ptr< SSearchResult > getSearchResultInfo( const QModelIndex &idx ) const;

            void setNameFilters( const QStringList &filters, QTreeView *view = nullptr, QProgressDialog *progress = nullptr );

            void reloadModel( QTreeView *view, QProgressDialog *dlg );

            void setRootPath( const QString &path, QTreeView *view = nullptr, QProgressDialog *dlg = nullptr );

            QString getSearchName( const QModelIndex &idx ) const;
            bool treatAsTVShow( const QFileInfo &fileInfo, bool defaultValue ) const;
            virtual bool setData( const QModelIndex &idx, const QVariant &value, int role ) override;

            bool isSubtitleFile( const QModelIndex &idx, bool *isLangFileFormat = nullptr ) const;
            bool isSubtitleFile( const QFileInfo &info, bool *isLangFileFormat = nullptr ) const;

            bool canAutoSearch( const QModelIndex &index ) const;
            bool canAutoSearch( const QFileInfo &info ) const;
            int eventsPerPath() const { return 4; }// 4 events, get timestamp, create parent paths, rename, settimestamp}

            static bool isAutoSetText( const QString &text );

            const QFileIconProvider *iconProvider() const { return fIconProvider; }
        Q_SIGNALS:
            void sigDirReloaded( bool canceled );
        public Q_SLOTS:
            void slotTVOutputFilePatternChanged( const QString &outPattern );
            void slotTVOutputDirPatternChanged( const QString &outPattern );
            void slotMovieOutputDirPatternChanged( const QString &outPattern );
            void slotMovieOutputFilePatternChanged( const QString &outPattern );
            void slotLoadRootDirectory();
            void slotPatternChanged();
            void slotTreatAsTVByDefaultChanged( bool treatAsTVShowByDefault );
        private:
            void autoDetermineLanguageAttributes( QStandardItem *parent );
            std::unordered_map< QString, std::vector< QStandardItem * > > getChildSRTFiles( QStandardItem *item ) const;
            void appendRow( QStandardItem *parent, QList< QStandardItem * > &items );

            struct SIterateInfo
            {
                std::function< bool( const QFileInfo &dir ) > fPreDirFunction;
                std::function< void( const QFileInfo &dir, bool aOK ) > fPostDirFunction;
                std::function< bool( const QFileInfo &dir ) > fPreFileFunction;
                std::function< void( const QFileInfo &dir, bool aOK ) > fPostFileFunction;
            };

            void CDirModel::iterateEveryFile( const QFileInfo &fileInfo, const SIterateInfo &iterInfo ) const;
            std::pair< uint64_t, uint64_t > computeNumberOfFiles( const QFileInfo &fileInfo ) const;
            void loadFileInfo( const QFileInfo &info );

            std::unique_ptr< QDirIterator > getDirIteratorForPath( const QFileInfo &fileInfo ) const;

            QStandardItem * attachTreeNodes( TParentTree &parentTree ); // returns the root item (col 0) of the leaf node

            bool isIgnoredPathName( const QFileInfo &ii ) const;
            bool isExcludedDirName( const QFileInfo &ii ) const;

            STreeNode getItemRow( const QFileInfo &path ) const;

            QString getDispName( const QString &absPath ) const;
            std::pair< bool, QStandardItemModel * > process( bool displayOnly, QProgressDialog *progress = nullptr ) const;
            bool process( const QStandardItem *item, bool displayOnly, QStandardItemModel *resultsModel, QStandardItem *resultsParentItem, QProgressDialog *progressDlg ) const;

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

            void setChecked( QStandardItem *item, bool isTVShow ) const;

            EModelType fModelType{ EModelType::eUnknown };
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
            QProgressDialog *fProgressDlg{ nullptr };
        };
    }
}
#endif // 
