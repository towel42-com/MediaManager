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

#include "MainWindow.h"
#include "SelectTMDB.h"
#include "TransformConfirm.h"
#include "Preferences.h"

#include "ui_MainWindow.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "Core/SearchResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/DelayLineEdit.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/StayAwake.h"

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

            fImpl->directory->setDelay( 1000 );
            fImpl->directory->setIsOKFunction( [ ]( const QString & dirName )
            {
                auto fi = QFileInfo( dirName );
                return dirName.isEmpty() || (fi.exists() && fi.isDir() && fi.isExecutable());
            }, tr( "Directory '%1' does not Exist or is not a Directory" ) );

            auto completer = new QCompleter( this );
            fDirModel = new CCompleterFileSystemModel( completer );
            fDirModel->setRootPath( "/" );
            completer->setModel( fDirModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->directory->setCompleter( completer );
            connect( fImpl->directory, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotDirectoryChanged );
            connect( fImpl->directory, &CDelayComboBox::editTextChanged, this, &CMainWindow::slotDirectoryChangedImmediate );
            connect( fImpl->directory->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotLoad );

            completer = new QCompleter( this );
            fFileModel = new CCompleterFileSystemModel( completer );
            fFileModel->setRootPath( "/" );
            completer->setModel( fFileModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->fileName->setCompleter( completer );

            fImpl->fileName->setDelay( 1000 );
            fImpl->fileName->setIsOKFunction( [ ]( const QString & fileName )
            {
                auto fi = QFileInfo( fileName );
                return fileName.isEmpty() || (fi.exists() && fi.isFile() && fi.isReadable());
            }, tr( "File '%1' does not Exist or is not Readable" ) );
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );
            connect( fImpl->fileName->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotFileFinishedEditing );

            auto menu = fImpl->bifViewerPage->menu();
            if ( menu )
                menuBar()->addMenu( menu );

            auto toolBar = fImpl->bifViewerPage->toolBar();
            if ( toolBar )
                addToolBar( toolBar );

            connect( fImpl->actionOpen, &QAction::triggered, this, &CMainWindow::slotOpen );
            connect( fImpl->actionLoad, &QAction::triggered, this, &CMainWindow::slotLoad );
            connect( fImpl->actionRun, &QAction::triggered, this, &CMainWindow::slotRun );

            connect( fImpl->actionTreatAsTVShowByDefault, &QAction::triggered, this, &CMainWindow::slotTreatAsTVShowByDefault );
            connect( fImpl->actionExactMatchesOnly, &QAction::triggered, this, &CMainWindow::slotExactMatchesOnly );
            connect( fImpl->actionPreferences, &QAction::triggered, this, &CMainWindow::slotPreferences );

            connect( fImpl->tabWidget, &QTabWidget::currentChanged, this, &CMainWindow::slotWindowChanged );

            loadSettings();

            fImpl->transformMediaFileNamesPage->setSetupProgressDlgFunc(
                [ this ]( const QString & title, const QString & cancelButtonText, int max )
            {
                setupProgressDlg( title, cancelButtonText, max );
                return fProgressDlg;
            },
                [ this ]()
            {
                clearProgressDlg();
            }
            );
            connect( fImpl->transformMediaFileNamesPage, &CTransformMediaFileNamesPage::sigLoadFinished, this, &CMainWindow::slotLoadFinished );
            connect( fImpl->transformMediaFileNamesPage, &CTransformMediaFileNamesPage::sigStartStayAwake, this, &CMainWindow::slotStartStayAwake );
            connect( fImpl->transformMediaFileNamesPage, &CTransformMediaFileNamesPage::sigStopStayAwake, this, &CMainWindow::slotStopStayAwake );

            fImpl->makeMKVPage->setSetupProgressDlgFunc(
                [ this ]( const QString & title, const QString & cancelButtonText, int max )
            {
                setupProgressDlg( title, cancelButtonText, max );
                return fProgressDlg;
            },
                [ this ]()
            {
                clearProgressDlg();
            }
            );
            connect( fImpl->makeMKVPage, &CMakeMKVPage::sigLoadFinished, this, &CMainWindow::slotLoadFinished );
            connect( fImpl->makeMKVPage, &CMakeMKVPage::sigStartStayAwake, this, &CMainWindow::slotStartStayAwake );
            connect( fImpl->makeMKVPage, &CMakeMKVPage::sigStopStayAwake, this, &CMainWindow::slotStopStayAwake );

            fImpl->mergeSRTPage->setSetupProgressDlgFunc(
                [ this ]( const QString & title, const QString & cancelButtonText, int max )
            {
                setupProgressDlg( title, cancelButtonText, max );
                return fProgressDlg;
            },
                [ this ]()
            {
                clearProgressDlg();
            }
            );
            connect( fImpl->mergeSRTPage, &CMergeSRTPage::sigLoadFinished, this, &CMainWindow::slotLoadFinished );
            connect( fImpl->mergeSRTPage, &CMergeSRTPage::sigStartStayAwake, this, &CMainWindow::slotStartStayAwake );
            connect( fImpl->mergeSRTPage, &CMergeSRTPage::sigStopStayAwake, this, &CMainWindow::slotStopStayAwake );

            QSettings settings;
            fImpl->tabWidget->setCurrentIndex( settings.value( "LastFunctionalityPage", 0 ).toInt() );

            QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChangedImmediate );
            QTimer::singleShot( 10, this, &CMainWindow::slotDirectoryChanged );

            QTimer::singleShot( 0, this, &CMainWindow::slotWindowChanged );
        }


        CMainWindow::~CMainWindow()
        {
            saveSettings();
            QSettings settings;
            settings.setValue( "LastFunctionalityPage", fImpl->tabWidget->currentIndex() );
        }

        bool CMainWindow::setBIFFileName( const QString & name )
        {
            if ( name.isEmpty() )
                return true;
            if ( !QFileInfo( name ).exists() )
            {
                QMessageBox::critical( this, tr( "Could not View BIF File" ), tr( "'%1' does not exist" ).arg( name ) );
                return false;
            }

            fImpl->tabWidget->setCurrentWidget( fImpl->bifViewerTab );

            disconnect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );
            disconnect( fImpl->fileName->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotFileFinishedEditing );
            fImpl->fileName->setCurrentText( name );
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );
            connect( fImpl->fileName->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotFileFinishedEditing );

            slotFileFinishedEditing();
            return true;
        }

        void CMainWindow::loadSettings()
        {
            fImpl->directory->addItems( NCore::CPreferences::instance()->getDirectories(), true );
            fImpl->fileName->addItems( NCore::CPreferences::instance()->getFileNames(), true );
            fImpl->actionTreatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->actionExactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );

            slotTreatAsTVShowByDefault();
        }

        void CMainWindow::saveSettings()
        {
            NCore::CPreferences::instance()->setDirectories( fImpl->directory->getAllText() );
            NCore::CPreferences::instance()->setFileNames( fImpl->fileName->getAllText() );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->actionTreatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->actionExactMatchesOnly->isChecked() );
        }

        void CMainWindow::slotWindowChanged()
        {
            fImpl->dirLabel->setVisible( !isBIFViewerActive() );
            fImpl->directory->setVisible( !isBIFViewerActive() );

            fImpl->fileNameLabel->setVisible( isBIFViewerActive() );
            fImpl->fileName->setVisible( isBIFViewerActive() );

            validateRunAction();
            validateLoadAction();

            if ( isBIFViewerActive() )
                fImpl->actionOpen->setText( tr( "Open File..." ) );
            else
                fImpl->actionOpen->setText( tr( "Set Directory..." ) );

            fImpl->bifViewerPage->setActive( isBIFViewerActive() );
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

            CAutoWaitCursor awc;
            qApp->processEvents();

            validateLoadAction();
        }

        void CMainWindow::validateLoadAction()
        {
            CAutoWaitCursor awc;
            auto dirName = fImpl->directory->currentText();

            QFileInfo fi( dirName );
            bool aOK = !dirName.isEmpty() && fi.exists() && fi.isDir();
            fImpl->actionLoad->setEnabled( aOK && !isBIFViewerActive() );
        }

        void CMainWindow::validateRunAction()
        {
            fImpl->actionRun->setEnabled( !isBIFViewerActive() && canRun() );
        }

        void CMainWindow::slotFileFinishedEditing()
        {
            fileNameChanged( true );
        }

        void CMainWindow::slotFileChanged()
        {
            fileNameChanged( false );
        }

        void CMainWindow::fileNameChanged( bool andExecute )
        {
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );
            fImpl->fileName->addCurrentItem();
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );

            fImpl->bifViewerPage->setFileName( fImpl->fileName->currentText(), isBIFViewerActive() && andExecute );
        }

        void CMainWindow::slotOpen()
        {
            if ( isBIFViewerActive() )
            {
                auto fileName = QFileDialog::getOpenFileName( this, tr( "Select BIF File:" ), fImpl->fileName->currentText(), tr( "BIF Files (*.bif);;All Files (*.*)" ) );
                if ( !fileName.isEmpty() )
                    fImpl->fileName->setCurrentText( fileName );
            }
            else if ( isMergeSRTActive() || isTransformActive() || isMakeMKVActive() )
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
                fImpl->actionTreatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
                fImpl->actionExactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );
                slotTreatAsTVShowByDefault();
            }
        }

        void CMainWindow::clearProgressDlg()
        {
            fProgressDlg->close();
            fProgressDlg->reset();
            fImpl->actionOpen->setEnabled( true );
        }

        void CMainWindow::setupProgressDlg( const QString & title, const QString & cancelButtonText, int max )
        {
            fImpl->actionOpen->setEnabled( false );
            if ( fProgressDlg )
                fProgressDlg->reset();

            if ( !fProgressDlg )
            {
                fProgressDlg = std::make_shared< CDoubleProgressDlg >();
            }
            fProgressDlg->setWindowModality( Qt::WindowModal );
            fProgressDlg->setMinimumDuration( 0 );
            fProgressDlg->setAutoClose( false );
            fProgressDlg->setAutoReset( false );
            fProgressDlg->setSingleProgressBarMode( true );

            fProgressDlg->setValue( 0 );
            fProgressDlg->setWindowTitle( title );
            fProgressDlg->setCancelButtonText( cancelButtonText );
            fProgressDlg->setRange( 0, max );
            fProgressDlg->show();

            connect( fProgressDlg.get(), &CDoubleProgressDlg::canceled,
                     [ this ]()
            {
                fImpl->actionOpen->setEnabled( true );
            } );
        }

        void CMainWindow::slotExactMatchesOnly()
        {
            fImpl->transformMediaFileNamesPage->setExactMatchesOnly( fImpl->actionExactMatchesOnly->isChecked() );
        }

        void CMainWindow::slotTreatAsTVShowByDefault()
        {
            fImpl->transformMediaFileNamesPage->setTreatAsTVByDefault( fImpl->actionTreatAsTVShowByDefault->isChecked() );
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
            if ( isTransformActive() )
                return fImpl->transformMediaFileNamesPage->canRun();
            else if ( isMergeSRTActive() )
                return fImpl->mergeSRTPage->canRun();
            else if ( isMakeMKVActive() )
                return fImpl->makeMKVPage->canRun();
            return nullptr;
        }

        void CMainWindow::slotLoadFinished( bool canceled )
        {
            (void)canceled;
            validateRunAction();
            clearProgressDlg();
        }

        void CMainWindow::slotLoad()
        {
            bool aOK = true;
            fImpl->directory->addCurrentItem();
            if ( isTransformActive() )
            {
                fImpl->transformMediaFileNamesPage->load( fImpl->directory->currentText() );
            }
            else if ( isMergeSRTActive() )
            {
                fImpl->mergeSRTPage->load( fImpl->directory->currentText() );
            }
            else if ( isMakeMKVActive() )
            {
                fImpl->makeMKVPage->load( fImpl->directory->currentText() );
            }
            fImpl->actionRun->setEnabled( false );
        }

        void CMainWindow::slotRun()
        {
            if ( isTransformActive() )
            {
                fImpl->transformMediaFileNamesPage->run();
            }
            else if ( isMergeSRTActive() )
            {
                fImpl->mergeSRTPage->run();
            }
            else if ( isMakeMKVActive() )
            {
                fImpl->makeMKVPage->run();
            }
            else
                return;
        }

        void CMainWindow::slotStartStayAwake()
        {
            if ( !fStayAwake )
                fStayAwake = new NUtils::CStayAwake( true );

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
