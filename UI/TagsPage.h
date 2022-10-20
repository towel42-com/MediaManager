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

#ifndef _TAGSPAGE_H
#define _TAGSPAGE_H

#include "BasePage.h"

#include <unordered_map>
#include <optional>
class QLineEdit;

namespace NMediaManager
{
    namespace NModels
    {
        class CTagsModel;
        class CTagsFilterModel;
    }
}

namespace NMediaManager
{
    namespace NUi
    {
        class CTagsPage : public CBasePage
        {
            Q_OBJECT
        public:
            CTagsPage( QWidget *parent = nullptr );
            ~CTagsPage() override;

            virtual bool useSecondaryProgressBar() const override { return false; }

            virtual QString loadTitleName() const override;
            virtual QString loadCancelName() const override;
            virtual QStringList dirModelFilter() const override;

            virtual QString actionTitleName() const override;
            virtual QString actionCancelName() const override;
            virtual QString actionErrorName() const override;

            virtual NModels::CDirModel * createDirModel() override;
            virtual QAbstractItemModel * getDirModel() const override;
            virtual void setupModel() override;

            virtual void postLoadFinished( bool canceled ) override;
            virtual void postNonQueuedRun( bool finalStep, bool canceled ) override;

            virtual bool extendContextMenu( QMenu * menu, const QModelIndex & idx ) override;

            virtual QMenu * menu() override;
        Q_SIGNALS:
        public Q_SLOTS:
        protected Q_SLOTS:
            void slotMenuAboutToShow();
            void slotPreferencesChanged( NPreferences::EPreferenceTypes prefTypes );

        protected:
            virtual void loadSettings() override;
            NModels::CTagsModel * tagsModel();

            QMenu * fMenu{ nullptr };
            QAction * fIgnoreSkippedPathSettings{ nullptr };
            QAction * fVerifyMediaTitle{ nullptr };
            QAction * fVerifyMediaDate{ nullptr };
            QAction * fVerifyMediaComment{ nullptr };
            QAction * fVerifyMediaTags{ nullptr };

            QLineEdit * fFilter{ nullptr };

            NModels::CTagsFilterModel * fFilterModel{ nullptr };
        };
    }
}
#endif 
