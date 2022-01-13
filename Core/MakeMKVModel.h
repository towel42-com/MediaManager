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

#ifndef _MAKEMKVMODEL_H
#define _MAKEMKVMODEL_H

#include "DirModel.h"

namespace NMediaManager
{
    namespace NCore
    {
        class CMakeMKVModel : public CDirModel
        {
            Q_OBJECT
        public:
            enum EColumns
            {
                eMediaTitle = NCore::EColumns::eFirstCustomColumn,
                eMediaYear,
                eMediaComment
            };
            
            CMakeMKVModel( NUi::CBasePage * page, QObject * parent = nullptr );
            ~CMakeMKVModel();

        private:
            virtual std::pair< bool, QStandardItem * > processItem( const QStandardItem * item, QStandardItem * parentItem, bool displayOnly ) const override;
            virtual void preAddItems( const QFileInfo & fileInfo, std::list< NMediaManager::NCore::STreeNodeItem > & currItems ) const override;
            virtual bool showMediaItems() const { return true; };
            virtual int firstMediaItemColumn() const { return EColumns::eMediaTitle; }
            virtual QStringList headers() const override;
            virtual void setupNewItem( const STreeNodeItem & /*nodeItem*/, const QStandardItem * /*nameItem*/, QStandardItem * /*item*/ ) const override;
            virtual QString getProgressLabel( const SProcessInfo & processInfo ) const override;
            virtual void postLoad( QTreeView * /*treeView*/ ) const override;
            virtual int computeNumberOfItems() const override;

            virtual void postFileFunction( bool aOK, const QFileInfo & fileInfo );;
            virtual bool preFileFunction( const QFileInfo & fileInfo, std::unordered_set<QString> & alreadyAdded, TParentTree & tree );

            virtual void attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem * /*prevParent*/, const STreeNode & /*treeNode*/ );

            virtual bool usesQueuedProcessing() const { return true; }

            int64_t getNumberOfSeconds( const QString & fileName ) const;
        };
    }
}
#endif 
