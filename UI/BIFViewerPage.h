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

#ifndef _BIFVIEWERPAGE_H
#define _BIFVIEWERPAGE_H

#include <QWidget>
#include <optional>
#include <memory>
class QLabel;
class QSpinBox;
class QMenu;
class QToolBar;
namespace NBIF
{
    class CBIFFile;
    class CBIFModel;
    enum class EButtonsLayout;
}
class QSpinBox;

namespace NMediaManager
{
    namespace NCore
    {
        struct SSearchTMDBInfo;
        struct SSearchResult;
        class CDirModel;
        class CSearchTMDB;
    }
}

namespace NMediaManager
{
    namespace NUi
    {
        namespace Ui { class CBIFViewerPage; }
        class CBIFViewerPage : public QWidget
        {
            Q_OBJECT
        public:
            CBIFViewerPage( QWidget *parent = 0 );
            ~CBIFViewerPage();

            void setActive( bool isActive );
            bool setFileName( const QString &fileName, bool andExecute );

            QAction *actionSkipBackward();
            QAction *actionPrev();
            QAction *actionTogglePlayPause( std::optional< bool > asPlayButton = {} ); // if not set no change, if set its changed to pay or not
            QAction *actionPause();
            QAction *actionPlay();
            QAction *actionNext();
            QAction *actionSkipForward();

            void setButtonsLayout( NBIF::EButtonsLayout style );
            NBIF::EButtonsLayout buttonsLayout()const;

            virtual bool eventFilter( QObject *obj, QEvent *event ) override;

            QMenu * menu();
            QToolBar *toolBar();
        public Q_SLOTS:
            void slotPlayingStarted();
            void slotResize();
        private:
            bool outOfDate() const;
            void fileNameChanged();
            bool canLoad() const;
            void load();
            void clear();
            void formatBIFTable();
            void loadSettings( bool init );
            void saveSettings();
        
            QTimer *fResizeTimer{ nullptr };
            std::shared_ptr< NBIF::CBIFFile > fBIF;
            NBIF::CBIFModel *fBIFModel{ nullptr };

            std::unique_ptr< Ui::CBIFViewerPage > fImpl;
            QString fFileName;
        };
    }
}
#endif 
