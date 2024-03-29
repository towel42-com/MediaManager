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

#ifndef _VALIDATEMKVMODEL_H
#define _VALIDATEMKVMODEL_H

#include "DirModel.h"

namespace NMediaManager
{
    namespace NModels
    {
        class CValidateMKVModel : public CDirModel
        {
            Q_OBJECT
        public:
            enum EColumns
            {
                eMediaColumnLoc = NModels::EColumns::eFirstCustomColumn,
            };

            CValidateMKVModel( NUi::CBasePage *page, QObject *parent = nullptr );
            virtual ~CValidateMKVModel() override;

        private:
            virtual QStringList dirModelFilter() const override;

            virtual std::pair< bool, std::list< QStandardItem * > > processItem( const QStandardItem *item, bool displayOnly ) override;
            virtual bool showMediaItems() const override { return true; };
            virtual int firstMediaItemColumn() const override { return EColumns::eMediaColumnLoc; }
            virtual QStringList headers() const override;
            virtual QString getProgressLabel( std::shared_ptr< SProcessInfo > processInfo ) const override;
            virtual void postLoad( QTreeView * /*treeView*/ ) override;
            virtual void preLoad( QTreeView * /*treeView*/ ) override;
            virtual void postProcess( bool /*displayOnly*/ ) override;

            virtual void postFileFunction( bool aOK, const QFileInfo &fileInfo, TParentTree & /*tree*/, bool countOnly ) override;
            virtual bool preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > &alreadyAdded, TParentTree &tree, bool countOnly ) override;

            virtual void attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ ) override;

            virtual bool usesQueuedProcessing() const override { return true; }
            virtual std::optional< std::pair< uint64_t, std::optional< uint64_t > > > getCurrentProgress( const QString &string ) override;
        };
    }
}
#endif
