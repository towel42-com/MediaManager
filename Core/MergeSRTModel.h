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

#ifndef _MERGESRTMODEL_H
#define _MERGESRTMODEL_H

#include "DirModel.h"

namespace NMediaManager
{
    namespace NCore
    {
        class CMergeSRTModel : public CDirModel
        {
            Q_OBJECT
        public:
            enum EColumns
            {
                eLanguage = NCore::EColumns::eFirstCustomColumn,
                eForced,
                eSDH,
                eOnByDefault
            };

            CMergeSRTModel( NUi::CBasePage * page, QObject * parent = nullptr );
            ~CMergeSRTModel();

        private:
            virtual std::pair< bool, QStandardItem * > processItem( const QStandardItem * item, QStandardItem * parentResultItem, bool displayOnly ) override;
            QStandardItem * processSRTSubTitle(QStandardItem * mkvFile, const std::unordered_map< QString, std::vector< QStandardItem * > > & srtFiles, QStandardItem * parentItem, bool displayOnly) const;
            QStandardItem * processSUBIDXSubTitle(QStandardItem * mkvFile, const std::list< std::pair< QStandardItem *, QStandardItem * > > & subidxFiles, QStandardItem * parentItem, bool displayOnly) const;
            std::list< std::pair< QStandardItem *, QStandardItem * > > pairSubIDX(const std::list< QStandardItem * > & idxFiles, const std::list< QStandardItem * > & subFiles) const;

            virtual std::list< NMediaManager::NCore::SDirNodeItem > addAdditionalItems( const QFileInfo & fileInfo ) const override;
            virtual void setupNewItem( const SDirNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item ) const override;
            virtual QStringList headers() const override;
            virtual void postLoad( QTreeView * treeView ) override;
            virtual void preLoad(QTreeView * treeView) override;
            virtual void attachTreeNodes( QStandardItem * nextParent, QStandardItem *& prevParent, const STreeNode & treeNode ) override;
            virtual int computeNumberOfItems() const override;

            virtual bool preFileFunction( const QFileInfo & fileInfo, std::unordered_set<QString> & alreadyAdded, TParentTree & tree ) override;
            virtual void postFileFunction( bool aOK, const QFileInfo & fileInfo ) override;

            virtual void postProcess( bool /*displayOnly*/ ) override;
            virtual bool postExtProcess( const SProcessInfo & info, QStringList & msgList ) override;

            virtual bool usesQueuedProcessing() const override { return true; }

            virtual QString getProgressLabel( const SProcessInfo & processInfo ) const override;

            bool isSubtitleFile(const QFileInfo & fileInfo, bool * isLangFileFormat = nullptr) const;

            QList< QFileInfo > getSRTFilesForMKV( const QFileInfo & fi ) const;
            QFileInfoList getSRTFilesInDir(const QDir & dir) const;

            std::optional< std::pair< QFileInfo, QFileInfo > > getIDXSUBFilesForMKV(const QFileInfo & fi) const;
            std::list< std::pair< QFileInfo, QFileInfo > > getIDXSUBFilesInDir(const QDir & dir) const;

            void autoDetermineLanguageAttributes( QStandardItem * parent );

            std::unordered_map< QString, std::vector< QStandardItem * > > getChildSRTFiles( const QStandardItem * item, bool sort ) const; // item should be a MKV file
            std::list< QStandardItem * > getChildFiles(const QStandardItem * item, const QString & ext) const;
            QList< QStandardItem * > getChildMKVFiles(const QStandardItem * item, bool goBelowDirs) const; // item should be a dir file

            QStandardItem * getLanguageItem( const QStandardItem * parent ) const;
        };
    }
}
#endif
