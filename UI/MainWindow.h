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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

class QMenu;
class QToolBar;

#include <QMainWindow>
#include <optional>
#include <tuple>

namespace NSABUtils 
{
    class CStayAwake;
    class CBackgroundFileCheck;
    namespace NBIF
    {
        enum class EButtonsLayout;
    }
}

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
        class CBasePage;
        class CCompleterFileSystemModel;
        namespace Ui { class CMainWindow; };
        class CMainWindow : public QMainWindow
        {
            Q_OBJECT
        public:
            CMainWindow( QWidget *parent = 0 );
            ~CMainWindow();

            bool setBIFFileName( const QString &name );
        public Q_SLOTS:
            virtual void slotOpen();
            virtual void slotDirectoryChanged();

            virtual void slotDirectoryChangedImmediate();
            virtual void slotLoad();
            virtual void slotRun();
            virtual void slotPreferences();
            virtual void slotWindowChanged();
            virtual void slotFileChanged();
            virtual void slotFileFinishedEditing();

            virtual void slotLoadFinished( bool canceled );
            virtual void slotStopStayAwake();
            virtual void slotStartStayAwake();
            virtual void slotFileCheckFinished( bool aOK, const QString & msg );
        private:
            CBasePage * getCurrentBasePage() const;
   
            bool isActivePageFileBased() const;
            bool isActivePageDirBased() const;

            void addUIComponents( QWidget * tab, CBasePage * page );
            void addUIComponents( QWidget * tab, QWidget * page, QMenu * menu, QToolBar * toolbar );

            void validateLoadAction();
            void validateRunAction();

            void fileNameChanged( bool andExecute );

            bool canRun() const;
            bool isTransformActive() const;
            bool isMergeSRTActive() const;
            bool isMakeMKVActive() const;
            bool isBIFViewerActive() const;

            void loadSettings();
            void saveSettings();

            std::unique_ptr< Ui::CMainWindow > fImpl;
            CCompleterFileSystemModel * fDirModel{ nullptr };
            CCompleterFileSystemModel * fFileModel{ nullptr };
            NSABUtils::CBackgroundFileCheck * fFileChecker;
            NSABUtils::CStayAwake * fStayAwake{ nullptr };
            std::map< QWidget *, std::tuple< QWidget *, QAction *, QToolBar * > > fUIComponentMap;
        };
    }
}
#endif 
