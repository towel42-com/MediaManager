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

#ifndef _TRANSFORMMODEL_H
#define _TRANSFORMMODEL_H

#include "DirModel.h"
#include "PatternInfo.h"

namespace NMediaManager
{
    namespace NCore
    {
        using TTitleMap = std::unordered_map< QString, std::map< int, std::map< int, QString > > >;

        class CTransformModel : public CDirModel
        {
            Q_OBJECT
        public:
            enum EColumns
            {
                eIsTVShow = NCore::EColumns::eFirstCustomColumn,
                eTransformName,
                eMediaColumnLoc
            };

            CTransformModel( NUi::CBasePage * page, QObject * parent = nullptr );
            ~CTransformModel();

            virtual bool setData(const QModelIndex & idx, const QVariant & value, int role) override;

            virtual std::optional< TItemStatus > computeItemStatus(const QModelIndex & idx ) const override;
            virtual bool canComputeStatus() const override;

            void setSearchResult( const QModelIndex & idx, std::shared_ptr< STransformResult > info, bool applyToChilren, bool forceSet );
            void setSearchResult( QStandardItem * item, std::shared_ptr< STransformResult > info, bool applyToChilren, bool forceSet);
            void clearSearchResult( const QModelIndex & idx, bool recursive );

            bool treatAsTVShow( const QFileInfo & fileInfo, bool defaultValue ) const;
            virtual int eventsPerPath() const override { return 5; }// get timestamp, create parent paths, rename, setting tag info, settimestamp}
            virtual void clear() override;
            void clearResults();
            void clearStatusResults();

            void computeEpisodesForDiskNumbers();
            virtual QString getSearchName( const QModelIndex & idx ) const;

            bool inAutoSearch() const { return fInAutoSearch; }
            void setInAutoSearch(bool val);

            void setDeleteItem( const QModelIndex & idx );
            bool canAutoSearch( const QModelIndex & index, bool recursive ) const;
            bool canAutoSearch( const QFileInfo & info, bool recursive) const;

            std::shared_ptr< STransformResult > getTransformResult( const QModelIndex & idx, bool checkParents ) const;
            std::shared_ptr< STransformResult > getTransformResult( const QString & path, bool checkParents ) const;
            std::shared_ptr< STransformResult > getTransformResult( const QFileInfo & path, bool checkParents ) const;
        public Q_SLOTS:
            void slotPatternChanged();

            void slotTVOutputFilePatternChanged( const QString & outPattern );
            void slotTVOutputDirPatternChanged( const QString & outPattern );
            void slotMovieOutputDirPatternChanged( const QString & outPattern );
            void slotMovieOutputFilePatternChanged( const QString & outPattern );
        private:
            virtual void updateFile(const QModelIndex &idx, const QString & oldFile, const QString & newFile) override;
            virtual void updateDir(const QModelIndex & idx, const QDir & oldDir, const QDir & newDir) override;
            virtual void postAddItems( const QFileInfo & fileInfo, std::list< NMediaManager::NCore::SDirNodeItem > & currItems ) const override;
            virtual std::list< NMediaManager::NCore::SDirNodeItem > addAdditionalItems( const QFileInfo & fileInfo ) const override;
            virtual bool showMediaItems() const override { return true; };
            virtual int firstMediaItemColumn() const override { return EColumns::eMediaColumnLoc; }

            std::pair< bool, QString > transformCorrect(const QModelIndex & idx) const;

            virtual void setupNewItem( const SDirNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item ) const override;
            virtual QStringList headers() const override;

            virtual void postLoad( QTreeView * treeView ) override;
            virtual void preLoad(QTreeView * treeView) override;
            virtual void attachTreeNodes( QStandardItem * nextParent, QStandardItem *& prevParent, const STreeNode & treeNode ) override;

            virtual void postReloadModelRequest() override;
            QString getMyTransformedName( const QStandardItem * item, bool transformParentsOnly ) const override;

            // model overrides during iteration
            virtual void postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/ ) override;
            virtual bool preFileFunction( const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/ ) override;

            virtual bool usesQueuedProcessing() const override { return false; }

            virtual std::pair< bool, QStandardItem * > processItem( const QStandardItem * item, QStandardItem * parentResultItem, bool displayOnly ) override;

            QStandardItem * getTransformItem( const QStandardItem * parent ) const;

            bool setMediaTags( const QString & fileName, std::shared_ptr< STransformResult > & searchResults, QString & msg ) const;

            bool isValidName(const QFileInfo & fi, std::optional< bool > isTVShow = {} ) const;
            bool isValidName( const QString & absPath, bool isDir, std::optional< bool > isTVShow ) const;

            void transformPatternChanged();
            void transformPatternChanged( const QStandardItem * parent );

            void updateTransformPattern( const QStandardItem * item ) const;
            void updateTransformPattern( const QStandardItem * transformedItem, QStandardItem * item ) const;
            [[nodiscard]] std::pair< bool, QString > transformItem( const QFileInfo & path ) const;
            [[nodiscard]] std::pair< bool, QString > transformItem( const QFileInfo & fileInfo, const SPatternInfo & patternInfo ) const;

            // title_season -> disk number -> titleNumber -> fullPath
            void findTitlesPerDiskNumbers( const QModelIndex & parentIdx, TTitleMap & retVal );
            std::map< int, QString > getDiskTitles( const QModelIndex & parentIdx );

            SPatternInfo fTVPatterns;
            SPatternInfo fMoviePatterns;

            mutable std::map< QString, std::pair< bool, QString > > fFileMapping;
            mutable std::map< QString, std::pair< bool, QString > > fDirMapping;
            std::unordered_map< QString, std::shared_ptr< STransformResult > > fTransformResultMap;
            std::unordered_map< QString, QString > fDiskRipSearchMap;

            //bool fTreatAsTVShowByDefault{ false };
            QTimer * fPatternTimer{ nullptr };
            bool fInAutoSearch{ false };
        };
    }
}
#endif // 
