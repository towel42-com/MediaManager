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

#ifndef _TRANSFORMMEDIAFILENAMESPAGE_H
#define _TRANSFORMMEDIAFILENAMESPAGE_H

#include "BasePage.h"

#include <unordered_map>
#include <optional>

namespace NMediaManager
{
    namespace NCore
    {
        struct SSearchTMDBInfo;
        struct STransformResult;
        class CTransformModel;
        class CSearchTMDB;
        enum class EMediaType;
    }
}

namespace NMediaManager
{
    namespace NUi
    {
        class CTransformMediaFileNamesPage : public CBasePage
        {
            Q_OBJECT
        public:
            CTransformMediaFileNamesPage( QWidget *parent = 0 );
            ~CTransformMediaFileNamesPage();

            virtual bool useSecondaryProgressBar() const override { return false; }

            virtual QString loadTitleName() const override;
            virtual QString loadCancelName() const override;
            virtual QStringList dirModelFilter() const override;

            virtual QString actionTitleName() const override;
            virtual QString actionCancelName() const override;
            virtual QString actionErrorName() const override;

            virtual NCore::CDirModel * createDirModel() override;
            virtual void setupModel() override;

            virtual void postLoadFinished( bool canceled ) override;
            virtual void postNonQueuedRun( bool finalStep, bool canceled ) override;

            void setExactMatchesOnly( bool value );
            void setTreatAsTVByDefault( bool value );

            virtual bool extendContextMenu( QMenu * menu, const QModelIndex & idx ) override;

            virtual QMenu * menu() override;
            virtual void setActive( bool isActive ) override;

            void search( const QModelIndex & idx );

        Q_SIGNALS:
        public Q_SLOTS:
        protected Q_SLOTS:
            void slotAutoSearchForNewNames();
            void slotAutoSearchFinished( const QString & path, NCore::SSearchTMDBInfo * searchInfo, bool searchesRemaining );
            void slotTreatAsTVShowByDefault();
            void slotExactMatchesOnly();
            void slotMenuAboutToShow();
        protected:
            virtual void loadSettings() override;
            bool autoSearchForNewNames( const QModelIndex & rootIdx, bool searchChildren, std::optional< NCore::EMediaType > mediaType );
            
            NCore::CTransformModel * model();

            NCore::CSearchTMDB * fSearchTMDB{ nullptr };
            uint64_t fSearchesCompleted{ 0 };
            QMenu * fMenu{ nullptr };
            QAction * fTreatAsTVShowByDefaultAction{ nullptr };
            QAction * fExactMatchesOnlyAction{ nullptr };
            QAction * fDeleteKnownPaths{ nullptr };
        };
    }
}
#endif 
