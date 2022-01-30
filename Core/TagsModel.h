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

#ifndef _TAGSMODEL_H
#define _TAGSMODEL_H

#include "DirModel.h"
#include "PatternInfo.h"

namespace NSABUtils
{
    enum class EMediaTags;
}

namespace NMediaManager
{
    namespace NCore
    {
        class CTagsModel : public CDirModel
        {
            Q_OBJECT
        public:
            enum EColumns
            {
                eMediaColumnLoc = NCore::EColumns::eFirstCustomColumn
            };

            CTagsModel( NUi::CBasePage * page, QObject * parent = nullptr );
            virtual ~CTagsModel() override;

        public Q_SLOTS:
        private:
            virtual void preLoad( QTreeView * treeView ) override;
            virtual int firstMediaItemColumn() const override { return fFirstColumn; }
            virtual int lastMediaItemColumn() const override { return fLastColumn; }
            virtual QStringList headers() const override;
            virtual std::pair< bool, QStandardItem * > processItem(const QStandardItem * item, QStandardItem * parentResultItem, bool displayOnly) override;
            virtual void attachTreeNodes( QStandardItem * nextParent, QStandardItem *& prevParent, const STreeNode & treeNode ) override;;
            virtual bool usesQueuedProcessing() const override { return false; }
            virtual void postFileFunction(bool /*aOK*/, const QFileInfo & /*fileInfo*/) override;
            virtual bool preFileFunction(const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/) override;
            virtual std::optional< TItemStatus > computeItemStatus(const QModelIndex & idx ) const override;

            virtual bool showMediaItems() const override { return false; };
            virtual int getMediaTitleLoc() const override { return fTitleColumn; }
            virtual int getMediaLengthLoc() const override { return fLengthColumn; }
            virtual int getMediaDateLoc() const override { return fDateColumn; }
            virtual int getMediaCommentLoc() const override { return fCommentColumn; }
            virtual void reloadMediaTags(const QModelIndex & idx) override;
            virtual std::list< NMediaManager::NCore::SDirNodeItem > addAdditionalItems(const QFileInfo & fileInfo) const override;

            std::list< NSABUtils::EMediaTags > fTagsBeingShown;
            int fFirstColumn{ -1 };
            int fTitleColumn{ -1 };
            int fLengthColumn{ -1 };
            int fDateColumn{ -1 };
            int fCommentColumn{ -1 };
            int fLastColumn{ -1 };
        };
    }
}
#endif // 
