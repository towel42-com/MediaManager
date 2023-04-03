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

#include "MainWindow.h"
#include "TranscodePage.h"
#include "ValidateMKVPage.h"
#include "MergeSRTPage.h"
#include "MediaNamingPage.h"
#include "TagsPage.h"
#include "BIFViewerPage.h"
#include "GenerateBIFPage.h"

#include "ui_MainWindow.h"

#include "Preferences/UI/Preferences.h"
#include "Preferences/Core/Preferences.h"
#include "Models/DirModel.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/MediaInfo.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/DelayLineEdit.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/StayAwake.h"
#include "SABUtils/BackgroundFileCheck.h"
#include "SABUtils/SelectFileUrl.h"
#include "SABUtils/uiUtils.h"

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QTimer>
#include <QPixmap>
#include <QLabel>
#include <QSpinBox>
#include <QThreadPool>
#include <QAbstractNativeEventFilter>

#ifdef Q_OS_WINDOWS
    #include <qt_windows.h>
    #include <windowsx.h>
#endif

namespace NMediaManager
{
    namespace NUi
    {
        class CCompleterFileSystemModel : public QFileSystemModel
        {
        public:
            CCompleterFileSystemModel( QObject *parent = nullptr ) :
                QFileSystemModel( parent )
            {
            }

            QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override
            {
                if ( role == Qt::DisplayRole && index.column() == 0 )
                {
                    QString path = filePath( index );
                    if ( path.endsWith( "\\" ) || path.endsWith( "/" ) )
                        path.chop( 1 );
                    return path;
                }
                return QFileSystemModel::data( index, role );
            }
        };

        class CClickOnTitleBar : public QAbstractNativeEventFilter
        {
        public:
            CClickOnTitleBar() = default;
            ~CClickOnTitleBar() override = default;

            bool nativeEventFilter( const QByteArray &eventType, void *message, long *result ) override
            {
#ifdef Q_OS_WINDOWS
                (void)result;
                if ( eventType == "windows_generic_MSG" )
                {
                    auto msg = static_cast< MSG * >( message );
                    if ( msg->message == WM_NCLBUTTONDBLCLK )
                    {
                        auto hitTest = static_cast< int >( msg->wParam );
                        if ( hitTest == HTCAPTION )
                        {
                            auto pt = QPoint( GET_X_LPARAM( msg->lParam ), GET_Y_LPARAM( msg->lParam ) );
                            auto rootWindow = qApp->topLevelAt( pt );
                            if ( dynamic_cast< CMainWindow * >( rootWindow ) )
                                return dynamic_cast< CMainWindow * >( rootWindow )->titleBarClicked( pt );
                        }
                    }
                }
#endif
                return false;
            }
        };

        CMainWindow::CMainWindow( QWidget *parent ) :
            QMainWindow( parent ),
            fImpl( new Ui::CMainWindow )
        {
            fImpl->setupUi( this );

            fFileChecker = new NSABUtils::CBackgroundFileCheck( this );
            connect( fFileChecker, &NSABUtils::CBackgroundFileCheck::sigFinished, this, &CMainWindow::slotFileCheckFinished );

            NSABUtils::CMediaInfo::setFFProbeEXE( NPreferences::NCore::CPreferences::instance()->getFFProbeEXE() );

            addPages();

            fImpl->directory->setDelay( 1000 );
            auto delayLE = new NSABUtils::CPathBasedDelayLineEdit;
            delayLE->setCheckExists( true );
            delayLE->setCheckIsDir( true );
            delayLE->setCheckIsExecutable( true );
            fImpl->directory->setDelayLineEdit( delayLE );

            auto completer = new QCompleter( this );
            fDirModel = new CCompleterFileSystemModel( completer );
            fDirModel->setRootPath( "/" );
            completer->setModel( fDirModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->directory->setCompleter( completer );
            connect( fImpl->directory, &NSABUtils::CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotDirectoryChanged );
            connect( fImpl->directory, &NSABUtils::CDelayComboBox::editTextChanged, this, &CMainWindow::slotDirectoryChangedImmediate );
            connect( fImpl->directory->lineEdit(), &NSABUtils::CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotLoad );

            completer = new QCompleter( this );
            fFileModel = new CCompleterFileSystemModel( completer );
            fFileModel->setRootPath( "/" );
            completer->setModel( fFileModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->fileName->setCompleter( completer );

            fImpl->fileName->setDelay( 1000 );
            delayLE = new NSABUtils::CPathBasedDelayLineEdit;
            delayLE->setCheckExists( true );
            delayLE->setCheckIsFile( true );
            delayLE->setCheckIsReadable( true );
            fImpl->fileName->setDelayLineEdit( delayLE );

            connect( fImpl->actionOpen, &QAction::triggered, this, &CMainWindow::slotOpen );
            connect( fImpl->actionLoad, &QAction::triggered, this, &CMainWindow::slotLoad );
            connect( fImpl->actionRun, &QAction::triggered, this, &CMainWindow::slotRun );

            connect( fImpl->actionPreferences, &QAction::triggered, this, &CMainWindow::slotPreferences );

            connect( fImpl->tabWidget, &QTabWidget::currentChanged, this, &CMainWindow::slotWindowChanged );

            loadSettings();

            connect( NPreferences::NCore::CPreferences::instance(), &NPreferences::NCore::CPreferences::sigPreferencesChanged, this, &CMainWindow::slotPreferencesChanged );

            new NSABUtils::CSelectFileUrl( this );

            qApp->installNativeEventFilter( new CClickOnTitleBar );

            QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChangedImmediate );
            QTimer::singleShot( 0, this, &CMainWindow::slotWindowChanged );
            QTimer::singleShot( 0, this, &CMainWindow::slotValidateDefaults );
        }

        CMainWindow::~CMainWindow()
        {
            saveSettings();
            if ( fStayAwake )
                delete fStayAwake;
        }

        void CMainWindow::connectBasePage( CBasePage *basePage )
        {
            connect( this, &CMainWindow::sigPreferencesChanged, basePage, &CBasePage::slotPreferencesChanged );
            connect( basePage, &CBasePage::sigLoadFinished, this, &CMainWindow::slotLoadFinished );
            connect( basePage, &CBasePage::sigStartStayAwake, this, &CMainWindow::slotStartStayAwake );
            connect( basePage, &CBasePage::sigStopStayAwake, this, &CMainWindow::slotStopStayAwake );
            connect( basePage, &CBasePage::sigDialogClosed, this, &CMainWindow::slotQueuedPrefChange );
        }

        std::shared_ptr< STabDef > CMainWindow::addPage( std::shared_ptr< STabDef > &tabDef )
        {
            fImpl->menuView->addAction( tabDef->fViewAction );

            bool isVisible = NPreferences::NCore::CPreferences::instance()->getPageVisible( tabDef->fName );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
            fImpl->tabWidget->setTabVisible( tabDef->fTabIndex, isVisible );
#else
            fImpl->tabWidget->setTabEnabled( tabDef->fTabIndex, isVisible );
#endif
            auto menu = tabDef->fPage->menu();
            tabDef->fToolbar = tabDef->fPage->toolBar();

            if ( menu )
                tabDef->fMenuAction = menuBar()->addMenu( menu );
            if ( tabDef->fToolbar )
                addToolBar( tabDef->fToolbar );
            fUIComponentMap.emplace_back( tabDef );
            connectBasePage( tabDef->fPage );

            return tabDef;
        }

        void CMainWindow::addPages()
        {
            addPage( std::make_shared< STabDef >( new CTranscodePage( nullptr ), tr( "Transcode Media" ), QString::fromUtf8( ":/convert.png" ), fImpl->tabWidget ) );
            addPage( std::make_shared< STabDef >( new CMergeSRTPage( nullptr ), tr( "Merge SRT" ), QString::fromUtf8( ":/cc.png" ), fImpl->tabWidget ) );
            addPage( std::make_shared< STabDef >( new CMediaNamingPage( nullptr ), tr( "Media Namer" ), QString::fromUtf8( ":/rename.png" ), fImpl->tabWidget ) );
            addPage( std::make_shared< STabDef >( new CTagsPage( nullptr ), tr( "Media Tags" ), QString::fromUtf8( ":/tag.png" ), fImpl->tabWidget ) );
            addPage( std::make_shared< STabDef >( new CValidateMKVPage( nullptr ), tr( "Validate MKV" ), QString::fromUtf8( ":/mkv.png" ), fImpl->tabWidget ) );
            addPage( std::make_shared< STabDef >( new CGenerateBIFPage( nullptr ), tr( "Generate Thumbnail Videos" ), QString::fromUtf8( ":/roku.png" ), fImpl->tabWidget ) );
            auto bifPage = addPage( std::make_shared< STabDef >( new CBIFViewerPage( nullptr ), tr( "Thumbnail Viewer" ), QString::fromUtf8( ":/roku.png" ), fImpl->tabWidget ) );

            connect( fImpl->fileName, &NSABUtils::CDelayComboBox::sigEditTextChangedAfterDelay, dynamic_cast< CBIFViewerPage * >( bifPage->fPage ), &CBIFViewerPage::slotFileChanged );
            connect( fImpl->fileName->lineEdit(), &NSABUtils::CDelayLineEdit::sigFinishedEditingAfterDelay, dynamic_cast< CBIFViewerPage * >( bifPage->fPage ), &CBIFViewerPage::slotFileFinishedEditing );
        }

        void CMainWindow::slotValidateDefaults()
        {
#ifdef _DEBUG
            NPreferences::NCore::CPreferences::instance()->showValidateDefaults( this, false );
#endif
        }

        void CMainWindow::slotPreferencesChanged( NPreferences::EPreferenceTypes prefType )
        {
            if ( ( prefType & NPreferences::EPreferenceType::eSystemPrefs ) != 0 )
            {
                for ( auto &&ii : fUIComponentMap )
                {
                    ii->setVisiblePerPrefs();
                }
            }

            fQueuedPrefChanged.reset();
            if ( !qApp->activeModalWidget() )
                emit sigPreferencesChanged( prefType );
            else
                fQueuedPrefChanged = prefType;
        }

        bool CMainWindow::isActivePageFileBased() const
        {
            auto basePage = getCurrentBasePage();
            if ( basePage )
                return basePage->isFileBased();

            return false;
        }

        bool CMainWindow::isActivePageDirBased() const
        {
            auto basePage = getCurrentBasePage();
            if ( basePage )
                return basePage->isDirBased();

            return false;
        }

        bool CMainWindow::setBIFFileName( const QString &name )
        {
            QSettings settings;
            int index = -1;
            CBIFViewerPage *page = nullptr;
            for ( auto &&ii : fUIComponentMap )
            {
                page = dynamic_cast< CBIFViewerPage * >( ii->fPage );
                if ( page )
                {
                    index = fImpl->tabWidget->indexOf( ii->fTab );
                    break;
                }
            }

            Q_ASSERT( index != -1 );

            if ( ( index == -1 ) || ( page == nullptr ) )
                return false;

            fImpl->tabWidget->setCurrentIndex( index );
            return page->setFileName( fImpl->fileName, name, true );
        }

        void CMainWindow::loadSettings()
        {
            fImpl->directory->addItems( NPreferences::NCore::CPreferences::instance()->getDirectories(), true );
            fImpl->fileName->addItems( NPreferences::NCore::CPreferences::instance()->getFileNames(), true );

            QSettings settings;
            int index = -1;
            for ( auto &&ii : fUIComponentMap )
            {
                if ( dynamic_cast< CMediaNamingPage * >( ii->fPage ) )
                {
                    index = fImpl->tabWidget->indexOf( ii->fTab );
                    break;
                }
            }

            fImpl->tabWidget->setCurrentIndex( settings.value( "LastFunctionalityPage", index ).toInt() );
        }

        void CMainWindow::saveSettings()
        {
            NPreferences::NCore::CPreferences::instance()->setDirectories( fImpl->directory->getAllText() );
            NPreferences::NCore::CPreferences::instance()->setFileNames( fImpl->fileName->getAllText() );
            QSettings settings;
            settings.setValue( "LastFunctionalityPage", fImpl->tabWidget->currentIndex() );
        }

        void CMainWindow::slotWindowChanged()
        {
            fImpl->dirLabel->setVisible( isActivePageDirBased() );
            fImpl->directory->setVisible( isActivePageDirBased() );

            fImpl->fileNameLabel->setVisible( isActivePageFileBased() );
            fImpl->fileName->setVisible( isActivePageFileBased() );
            fImpl->actionLoad->setVisible( isActivePageDirBased() );

            if ( isActivePageFileBased() )
                fImpl->actionOpen->setText( tr( "Open File..." ) );
            else if ( isActivePageDirBased() )
                fImpl->actionOpen->setText( tr( "Set Directory..." ) );

            validateRunAction();
            validateLoadAction();

            auto activePage = fImpl->tabWidget->currentWidget();
            for ( auto &&ii : fUIComponentMap )
            {
                auto tabWidget = ii->fTab;

                bool isActive = ( activePage == tabWidget );
                ii->fPage->setActive( isActive );

                if ( ii->fMenuAction )
                    ii->fMenuAction->setVisible( isActive );
                if ( ii->fToolbar )
                    ii->fToolbar->setVisible( isActive );
            }
        }

        void CMainWindow::slotDirectoryChangedImmediate()
        {
            fImpl->actionLoad->setEnabled( false );
            fImpl->actionRun->setEnabled( false );
        }

        void CMainWindow::slotDirectoryChanged()
        {
            slotDirectoryChangedImmediate();
            fImpl->actionRun->setEnabled( false );

            NSABUtils::CAutoWaitCursor awc;
            qApp->processEvents();

            validateLoadAction();
        }

        void CMainWindow::validateLoadAction()
        {
            fFileChecker->checkPath( fImpl->directory->currentText() );
        }

        void CMainWindow::slotFileCheckFinished( bool aOK, const QString &msg )
        {
            fImpl->actionLoad->setEnabled( aOK );
            if ( !aOK )
                fImpl->actionLoad->setToolTip( msg );
        }

        void CMainWindow::validateRunAction()
        {
            fImpl->actionRun->setEnabled( canRun() );
        }

        void CMainWindow::slotOpen()
        {
            bool changed = false;
            if ( isActivePageFileBased() )
            {
                auto prev = fImpl->fileName->currentText();
                auto fileName = QFileDialog::getOpenFileName( this, tr( "Select File:" ), prev, getCurrentBasePage()->selectFileFilter() );
                if ( !fileName.isEmpty() )
                {
                    fImpl->fileName->setCurrentText( fileName );
                    changed = true;
                }
            }
            else if ( isActivePageDirBased() )
            {
                auto prev = fImpl->directory->isOK() ? fImpl->directory->currentText() : QString();
                auto dir = QFileDialog::getExistingDirectory( this, tr( "Select Directory:" ), prev );
                if ( !dir.isEmpty() )
                {
                    fImpl->directory->setCurrentText( dir );
                    changed = true;
                }
            }
            if ( changed )
                slotLoad();
        }

        void CMainWindow::slotPreferences()
        {
            NPreferences::NUi::CPreferences dlg;
            if ( dlg.exec() == QDialog::Accepted )
            {
                slotQueuedPrefChange();
            }
        }

        void CMainWindow::clearDirModel()
        {
            auto basePage = getCurrentBasePage();
            if ( basePage )
                return basePage->clearDirModel();
        }

        bool CMainWindow::canRun() const
        {
            auto basePage = getCurrentBasePage();
            if ( basePage )
                return basePage->canRun();
            return false;
        }

        void CMainWindow::slotLoadFinished( bool canceled )
        {
            (void)canceled;
            validateRunAction();
        }

        void CMainWindow::slotLoad()
        {
            bool aOK = true;
            fImpl->directory->addCurrentItem();
            auto basePage = getCurrentBasePage();
            if ( basePage )
            {
                fImpl->actionRun->setEnabled( false );
                basePage->load( fImpl->directory->currentText() );
            }
        }

        CBasePage *CMainWindow::getCurrentBasePage() const
        {
            auto currWidget = fImpl->tabWidget->currentWidget();
            auto basePage = currWidget->findChild< CBasePage * >();
            return basePage;
        }

        void CMainWindow::slotRun()
        {
            auto basePage = getCurrentBasePage();
            if ( basePage )
                basePage->run( QModelIndex() );
        }

        void CMainWindow::slotStartStayAwake()
        {
            if ( !fStayAwake )
                fStayAwake = new NSABUtils::CStayAwake( true );

            QThreadPool::globalInstance()->start( fStayAwake );
        }

        void CMainWindow::slotStopStayAwake()
        {
            if ( !fStayAwake )
                return;
            fStayAwake->stop();
        }

        bool CMainWindow::titleBarClicked( const QPoint &pt ) const
        {
            auto localPt = mapFromGlobal( pt );
            auto title = windowTitle();
            return NSABUtils::launchIfURLClicked( title, localPt, fImpl->menubar->font() );
        }

        void CMainWindow::slotQueuedPrefChange()
        {
            if ( fQueuedPrefChanged.has_value() )
            {
                emit sigPreferencesChanged( fQueuedPrefChanged.value() );
                fQueuedPrefChanged.reset();
            }
        }

        STabDef::STabDef( CBasePage *page, const QString &name, const QString &iconPath, QTabWidget *tabWidget ) :
            fPage( page ),
            fName( name ),
            fTabWidget( tabWidget )
        {
            fIcon.addFile( iconPath, QSize(), QIcon::Normal, QIcon::Off );
            Q_ASSERT( !fIcon.isNull() );

            fTab = new QWidget();
            fTab->setObjectName( QString( "%1-Tab" ).arg( name ) );
            auto vertLayout = new QVBoxLayout( fTab );
            vertLayout->setSpacing( 6 );
            vertLayout->setObjectName( QString( "%1-Vertlayout" ).arg( name ) );
            vertLayout->setContentsMargins( 0, 0, 0, 0 );

            fPage->setParent( fTab );
            fPage->setObjectName( QString( "%1-Page" ).arg( name ) );

            vertLayout->addWidget( fPage );

            fViewAction = new QAction( fIcon, QObject::tr( "View %1?" ).arg( fName ), nullptr );
            fViewAction->setCheckable( true );
            fViewAction->setChecked( NPreferences::NCore::CPreferences::instance()->getPageVisible( fName ) );

            QObject::connect( fViewAction, &QAction::triggered, [ name ]( bool checked ) { NPreferences::NCore::CPreferences::instance()->setPageVisible( name, checked ); } );

            fTabIndex = fTabWidget->addTab( fTab, fIcon, fName );
            setVisiblePerPrefs();
        }

        void STabDef::setVisiblePerPrefs()
        {
            auto shouldBeVisible = NPreferences::NCore::CPreferences::instance()->getPageVisible( fName );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 15, 0 )
            fTabWidget->setTabVisible( fTabIndex, shouldBeVisible );
#else
            fTabWidget->setTabEnabled( fTabIndex, shouldBeVisible );
#endif
        }
    }
}
