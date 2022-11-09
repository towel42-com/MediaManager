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
class QUrl;
class QTabWidget;

#include <QMainWindow>
#include <optional>
#include <tuple>
#include "Preferences/Core/Preferences.h"

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
    namespace NUi
    {
        class CBasePage;
        class CCompleterFileSystemModel;
        namespace Ui { class CMainWindow; };
        struct STabDef
        {
            STabDef( CBasePage * page, const QString & name, const QString & iconPath, QTabWidget * tabWidget );

            void setVisiblePerPrefs();
            int fTabIndex{ -1 };
            QWidget * fTab{ nullptr };
            QString fName;
            QIcon fIcon;
            CBasePage * fPage{ nullptr };
            QAction * fMenuAction{ nullptr };
            QToolBar * fToolbar{ nullptr };
            QAction * fViewAction{ nullptr };
            QTabWidget * fTabWidget{ nullptr };
        };

        class CMainWindow : public QMainWindow
        {
            Q_OBJECT
        public:
            CMainWindow( QWidget * parent = nullptr );
            virtual ~CMainWindow() override;

            bool setBIFFileName( const QString & name );
            bool titleBarClicked( const QPoint & pt ) const;
        public Q_SLOTS:
            virtual void slotOpen();
            virtual void slotDirectoryChanged();

            virtual void slotDirectoryChangedImmediate();
            virtual void slotLoad();
            virtual void slotRun();
            virtual void slotPreferences();

            void slotQueuedPrefChange();

            virtual void slotWindowChanged();

            virtual void slotLoadFinished( bool canceled );
            virtual void slotStopStayAwake();
            virtual void slotStartStayAwake();
            virtual void slotFileCheckFinished( bool aOK, const QString & msg );
            virtual void slotValidateDefaults();
            virtual void slotPreferencesChanged( NPreferences::EPreferenceTypes prefType );

        Q_SIGNALS:
            void sigPreferencesChanged( NPreferences::EPreferenceTypes prefType );
        private:
            void addPages();
            std::shared_ptr< STabDef > addPage( std::shared_ptr< STabDef > & tabDef );

            CBasePage * getCurrentBasePage() const;
            void clearDirModel();

            bool isActivePageFileBased() const;
            bool isActivePageDirBased() const;

            void connectBasePage( CBasePage * basePage );

            void validateLoadAction();
            void validateRunAction();

            bool canRun() const;

            void loadSettings();
            void saveSettings();

            std::unique_ptr< Ui::CMainWindow > fImpl;
            CCompleterFileSystemModel * fDirModel{ nullptr };
            CCompleterFileSystemModel * fFileModel{ nullptr };
            NSABUtils::CBackgroundFileCheck * fFileChecker;
            NSABUtils::CStayAwake * fStayAwake{ nullptr };

            std::optional< NPreferences::EPreferenceTypes > fQueuedPrefChanged;
            std::list< std::shared_ptr< STabDef > > fUIComponentMap;
        };
    }
}
#endif 
