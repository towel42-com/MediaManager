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

#include <QMainWindow>
#include <optional>
namespace NSABUtils 
{
    class CStayAwake;
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
            void slotOpen();
            void slotDirectoryChanged();

            void slotDirectoryChangedImmediate();
            void slotLoad();
            void slotRun();
            void slotTreatAsTVShowByDefault();
            void slotExactMatchesOnly();
            void slotPreferences();
            void slotWindowChanged();
            void slotFileChanged();
            void slotFileFinishedEditing();

            void slotLoadFinished( bool canceled );
            void slotStopStayAwake();
            void slotStartStayAwake();
        private:
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
            NSABUtils::CStayAwake * fStayAwake{ nullptr };
        };
    }
}
#endif 
