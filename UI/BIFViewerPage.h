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

#ifndef _BIFVIEWERPAGE_H
#define _BIFVIEWERPAGE_H

#include "BasePage.h"

#include <optional>
#include <memory>
class QLabel;
class QSpinBox;
class QMenu;
class QToolBar;
namespace NSABUtils
{
    namespace NBIF
    {
        class CFile;
        class CModel;
        enum class EButtonsLayout;
    }
    class CImageScrollBar;
    class CDelayComboBox;
}
class QSpinBox;

namespace NMediaManager
{
    namespace NCore
    {
        class CDirModel;
    }
}

namespace NMediaManager
{
    namespace NUi
    {
        namespace Ui
        {
            class CBIFViewerPage;
        }
        class CBIFViewerPage : public CBasePage
        {
            Q_OBJECT
        public:
            CBIFViewerPage( QWidget *parent = nullptr );
            virtual ~CBIFViewerPage() override;

            virtual void setActive( bool isActive ) override;
            bool setFileName( NSABUtils::CDelayComboBox *comboBox, const QString &fileName, bool andExecute );

            QAction *actionSkipBackward();
            QAction *actionPrev();
            QAction *actionTogglePlayPause( std::optional< bool > asPlayButton = {} );   // if not set no change, if set its changed to pay or not
            QAction *actionPause();
            QAction *actionPlay();
            QAction *actionNext();
            QAction *actionSkipForward();

            void setButtonsLayout( NSABUtils::NBIF::EButtonsLayout style );
            NSABUtils::NBIF::EButtonsLayout buttonsLayout() const;

            virtual bool eventFilter( QObject *obj, QEvent *event ) override;

            virtual bool isDirBased() const override { return false; }
            virtual bool isFileBased() const override { return true; }

            virtual QMenu *menu() override;
            virtual QToolBar *toolBar() override;

            virtual QString selectFileFilter() const override;
        public Q_SLOTS:
            void slotPlayingStarted();
            void slotResize();

            void slotFileChanged( const QString &text );
            void slotFileFinishedEditing( const QString &text );
        private Q_SLOTS:
            virtual void slotPostInit() override;

        private:
            void connectToCB( NSABUtils::CDelayComboBox *comboBox, bool connect );
            virtual NModels::CDirModel *createDirModel() override { return nullptr; }
            virtual QString loadTitleName() const override { return {}; }
            virtual QString loadCancelName() const override { return {}; }

            virtual QString actionTitleName() const override { return {}; }
            virtual QString actionCancelName() const override { return {}; }
            virtual QString actionErrorName() const override { return {}; }

            bool outOfDate() const;
            void fileNameChanged( NSABUtils::CDelayComboBox *comboBox, const QString &text, bool andExecute );
            void fileNameChanged();
            bool canLoad() const;
            virtual void load( bool postRun ) override;
            void clear();
            void formatBIFTable();
            void loadSettings( bool init );
            virtual void saveSettings() override;

            QTimer *fResizeTimer{ nullptr };
            std::shared_ptr< NSABUtils::NBIF::CFile > fBIF;
            NSABUtils::NBIF::CModel *fBIFModel{ nullptr };
            NSABUtils::CImageScrollBar *fBIFScrollBar{ nullptr };
            std::unique_ptr< Ui::CBIFViewerPage > fImpl;
            QString fFileName;
        };
    }
}
#endif
