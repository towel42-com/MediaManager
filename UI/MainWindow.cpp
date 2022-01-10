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
#include "Preferences.h"

#include "ui_MainWindow.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/DelayLineEdit.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/StayAwake.h"
#include "SABUtils/BackgroundFileCheck.h"

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

#include <QProgressBar>
#include "SABUtils/FileUtils.h"

namespace NMediaManager
{
    namespace NUi
    {
        class CCompleterFileSystemModel : public QFileSystemModel
        {
        public:
            CCompleterFileSystemModel( QObject * parent = 0 ) :
                QFileSystemModel( parent )
            {}

            QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const override
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

        CMainWindow::CMainWindow( QWidget * parent )
            : QMainWindow( parent ),
            fImpl( new Ui::CMainWindow )
        {
            fImpl->setupUi( this );

            fFileChecker = new NSABUtils::CBackgroundFileCheck( this );
            connect( fFileChecker, &NSABUtils::CBackgroundFileCheck::sigFinished, this, &CMainWindow::slotFileCheckFinished );

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
            connect( fImpl->fileName, &NSABUtils::CDelayComboBox::sigEditTextChangedAfterDelay, fImpl->bifViewerPage, &CBIFViewerPage::slotFileChanged );
            connect( fImpl->fileName->lineEdit(), &NSABUtils::CDelayLineEdit::sigFinishedEditingAfterDelay, fImpl->bifViewerPage, &CBIFViewerPage::slotFileFinishedEditing );

            connect( fImpl->actionOpen, &QAction::triggered, this, &CMainWindow::slotOpen );
            connect( fImpl->actionLoad, &QAction::triggered, this, &CMainWindow::slotLoad );
            connect( fImpl->actionRun, &QAction::triggered, this, &CMainWindow::slotRun );

            connect( fImpl->actionPreferences, &QAction::triggered, this, &CMainWindow::slotPreferences );

            connect( fImpl->tabWidget, &QTabWidget::currentChanged, this, &CMainWindow::slotWindowChanged );

            loadSettings();
            
            addUIComponents( fImpl->bifViewerTab, fImpl->bifViewerPage );
            addUIComponents( fImpl->transformMediaFileNamesTab, fImpl->transformMediaFileNamesPage );
            addUIComponents( fImpl->makeMKVTab, fImpl->makeMKVPage );
            addUIComponents( fImpl->mergeSRTTab, fImpl->mergeSRTPage );

            QSettings settings;
            fImpl->tabWidget->setCurrentIndex( settings.value( "LastFunctionalityPage", 0 ).toInt() );

            QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChangedImmediate );
            //QTimer::singleShot( 10, this, &CMainWindow::slotDirectoryChanged );

            QTimer::singleShot( 0, this, &CMainWindow::slotWindowChanged );
        }


        CMainWindow::~CMainWindow()
        {
            saveSettings();
            QSettings settings;
            settings.setValue( "LastFunctionalityPage", fImpl->tabWidget->currentIndex() );
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

        void CMainWindow::connectBasePage( CBasePage * basePage )
        {
            connect( basePage, &CBasePage::sigLoadFinished, this, &CMainWindow::slotLoadFinished );
            connect( basePage, &CBasePage::sigStartStayAwake, this, &CMainWindow::slotStartStayAwake );
            connect( basePage, &CBasePage::sigStopStayAwake, this, &CMainWindow::slotStopStayAwake );
        }

        void CMainWindow::addUIComponents( QWidget * tab, CBasePage * page )
        {
            auto menu = page->menu();
            auto toolBar = page->toolBar();

            QAction * menuAction = nullptr;
            if ( menu )
                menuAction = menuBar()->addMenu( menu );
            if ( toolBar )
                addToolBar( toolBar );
            fUIComponentMap[tab] = std::make_tuple( page, menuAction, toolBar );
            connectBasePage( page );
        }

        bool CMainWindow::setBIFFileName( const QString & name )
        {
            fImpl->tabWidget->setCurrentWidget( fImpl->bifViewerTab );
            return fImpl->bifViewerPage->setFileName( fImpl->fileName, name, true );
        }

        void CMainWindow::loadSettings()
        {
            fImpl->directory->addItems( NCore::CPreferences::instance()->getDirectories(), true );
            fImpl->fileName->addItems( NCore::CPreferences::instance()->getFileNames(), true );
        }

        void CMainWindow::saveSettings()
        {
            NCore::CPreferences::instance()->setDirectories( fImpl->directory->getAllText() );
            NCore::CPreferences::instance()->setFileNames( fImpl->fileName->getAllText() );
        }

        void CMainWindow::slotWindowChanged()
        {
            fImpl->dirLabel->setVisible( isActivePageDirBased() );
            fImpl->directory->setVisible( isActivePageDirBased() );

            fImpl->fileNameLabel->setVisible( isActivePageFileBased() );
            fImpl->fileName->setVisible( isActivePageFileBased() );

            if ( isActivePageFileBased() )
                fImpl->actionOpen->setText( tr( "Open File..." ) );
            else if ( isActivePageDirBased() )
                fImpl->actionOpen->setText( tr( "Set Directory..." ) );

            validateRunAction();
            validateLoadAction();

            auto tmp = fImpl->bifViewerPage->parentWidget();
            auto index = fImpl->tabWidget->indexOf( fImpl->bifViewerPage );
            auto activePage = fImpl->tabWidget->currentWidget();
            for ( auto && ii : fUIComponentMap )
            {
                if ( std::get< 1 >( ii.second ) )
                    std::get< 1 >( ii.second )->setVisible( false );
                if ( std::get< 2 >( ii.second ) )
                    std::get< 2 >( ii.second )->setVisible( false );

                auto tabWidget = ii.first;
                auto currBasePage = dynamic_cast<CBasePage *>(std::get< 0 >( ii.second ));

                bool isActive = (activePage == tabWidget) ;
                currBasePage->setActive( isActive );
            }

            auto pos = fUIComponentMap.find( activePage );
            if ( pos != fUIComponentMap.end() )
            {
                if ( std::get< 1 >( (*pos).second ) )
                    std::get< 1 >( (*pos).second )->setVisible( true );
                if ( std::get< 2 >( (*pos).second ) )
                    std::get< 2 >( (*pos).second )->setVisible( true );
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

        void CMainWindow::slotFileCheckFinished( bool aOK, const QString & msg )
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
            if ( isActivePageFileBased() )
            {
                auto fileName = QFileDialog::getOpenFileName( this, tr( "Select File:" ), fImpl->fileName->currentText(), getCurrentBasePage()->selectFileFilter() );
                if ( !fileName.isEmpty() )
                    fImpl->fileName->setCurrentText( fileName );
            }
            else if ( isActivePageDirBased() )
            {
                auto dir = QFileDialog::getExistingDirectory( this, tr( "Select Directory:" ), fImpl->directory->currentText() );
                if ( !dir.isEmpty() )
                    fImpl->directory->setCurrentText( dir );
            }
        }

        void CMainWindow::slotPreferences()
        {
            CPreferences dlg;
            if ( dlg.exec() == QDialog::Accepted )
            {
            }
        }

        bool CMainWindow::isMergeSRTActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->mergeSRTTab;
        }

        bool CMainWindow::isMakeMKVActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->makeMKVTab;
        }

        bool CMainWindow::isTransformActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->transformMediaFileNamesTab;
        }

        bool CMainWindow::isBIFViewerActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->bifViewerTab;
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
                basePage->load( fImpl->directory->currentText() );
                fImpl->actionRun->setEnabled( false );
            }
        }

        CBasePage * CMainWindow::getCurrentBasePage() const
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
    }
}
