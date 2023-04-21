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

#ifndef _TranscodeModel_H
#define _TranscodeModel_H

#include "DirModel.h"

namespace NMediaManager
{
    namespace NCore
    {
        struct SLanguageInfo;
    }

    namespace NModels
    {
        class CTranscodeModel : public CDirModel
        {
            Q_OBJECT
        public:
            enum EColumns
            {
                eLanguage = NModels::EColumns::eFirstCustomColumn,
                eForced,
                eSDH,
                eOnByDefault,
                eMediaColumnLoc
            };

            CTranscodeModel( NUi::CBasePage *page, QObject *parent = nullptr );
            virtual ~CTranscodeModel() override;

        protected:
            virtual std::optional< TItemStatus > computeItemStatus( const QModelIndex &idx ) const;   // the one to override
            virtual void clear() override;

            virtual QStringList dirModelFilter() const override;

            virtual std::pair< bool, QStandardItem * > processItem( const QStandardItem *item, bool displayOnly ) override;

            // returns nothingToDo, aOK
            [[nodiscard]] std::pair< bool, bool >setupProcessItem( SProcessInfo &processInfo, const QString &path, const std::list< NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NCore::SLanguageInfo, QString > > & subIDXFiles, bool displayOnly ) const;
            bool processTranscoding( SProcessInfo &processInfo, const std::list< NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NCore::SLanguageInfo, QString > > &subIDXFiles, const QStandardItem *item, bool displayOnly );
            bool processSRTSubTitle( SProcessInfo &processInfo, const QStandardItem *mkvFile, const std::unordered_map< QString, std::vector< QStandardItem * > > &srtFiles ) const;
            bool processSUBIDXSubTitle( SProcessInfo &processInfo, const QStandardItem *mkvFile, const std::list< std::pair< QStandardItem *, QStandardItem * > > &subIDXFiles ) const;
            QString computeProgressLabel( const SProcessInfo &processInfo ) const;

            virtual bool showMediaItems() const override { return true; };
            virtual int firstMediaItemColumn() const override { return EColumns::eMediaColumnLoc; }
            virtual QStringList headers() const override;
            virtual QString getProgressLabel( const SProcessInfo &processInfo ) const override;
            virtual void postLoad( QTreeView * /*treeView*/ ) override;
            virtual void preLoad( QTreeView * /*treeView*/ ) override;
            virtual void postProcess( bool /*displayOnly*/ ) override;
            virtual std::pair< std::function< bool( const QVariant & ) >, int > getExcludeFuncForItemCount() const override;
            virtual std::list< SDirNodeItem > addAdditionalItems( const QFileInfo &fileInfo ) const override;
            virtual void setupNewItem( const SDirNodeItem &nodeItem, const QStandardItem *nameItem, QStandardItem *item ) const override;

            virtual void postFileFunction( bool aOK, const QFileInfo &fileInfo, TParentTree & /*tree*/, bool countOnly ) override;
            virtual bool preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > &alreadyAdded, TParentTree &tree, bool countOnly ) override;

            virtual void attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ ) override;

            virtual bool usesQueuedProcessing() const override { return true; }

            virtual std::optional< std::pair< uint64_t, std::optional< uint64_t > > > getCurrentProgress( const QString &string ) override;

            virtual std::optional< uint64_t > getNumSeconds( const QString &string ) const;

            std::optional< std::chrono::milliseconds > getMSRemaining( const QString &string, const std::pair< uint64_t, std::optional< uint64_t > > &currProgress ) const override;

            virtual bool currentUnitsAreSeconds() const { return true; }

            QList< QFileInfo > getSRTFilesForMKV( const QFileInfo &fi, bool countOnly ) const;
            std::optional< std::pair< QFileInfo, QFileInfo > > getIDXSUBFilesForMKV( const QFileInfo &fi ) const;
            void autoDetermineLanguageAttributes( QStandardItem *parent ) const;

            std::pair< std::list< std::pair< QStandardItem *, QStandardItem * > >, std::list< std::pair< NCore::SLanguageInfo, QString > > > pairSubIDX( const std::list< QStandardItem * > &idxFiles, const std::list< QStandardItem * > &subFiles ) const;
            std::list< std::pair< QFileInfo, QFileInfo > > getIDXSUBFilesInDir( const QDir &dir ) const;

            std::pair< std::unordered_map< QString, std::vector< QStandardItem * > >, std::list< NCore::SLanguageInfo > > getChildSRTFiles( const QStandardItem *item, bool sort ) const;   // item should be a MKV file

            bool isNameBasedMatch( const QFileInfo &mkvFile, const QFileInfo &srtFile ) const;
            bool nameMatch( const QString &mkvBaseName, const QString &subtitleFile ) const;

            bool isSubtitleFile( const QFileInfo &fileInfo, bool *isLangFileFormat = nullptr ) const;

            std::list< QStandardItem * > getChildFiles( const QStandardItem *item, const QString &ext ) const;
            QList< QStandardItem * > getChildMKVFiles( const QStandardItem *item, bool goBelowDirs ) const;   // item should be a dir file

            QStandardItem *getLanguageItem( const QStandardItem *parent ) const;

            mutable std::unordered_map< QString, std::optional< QList< QFileInfo > > > fSRTFileCache;
            mutable std::map< QStandardItem *, std::pair< QStandardItem *, NCore::SLanguageInfo > > fAllLangInfos;
        };
    }
}
#endif
