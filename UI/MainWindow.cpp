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
#include "SABUtils/BIFFile.h"
#include "SABUtils/BIFModel.h"
#include "SABUtils/BIFWidget.h"
#include "SABUtils/DelayLineEdit.h"

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


#include <QProgressDialog>

namespace NMediaManager
{
    namespace NUi
    {
        CMainWindow::CMainWindow( QWidget *parent )
            : QMainWindow( parent ),
            fImpl( new Ui::CMainWindow )
        {
            fImpl->setupUi( this );

            auto widgets = fImpl->bifViewerPage->getFrameIntervalWidgets();
            fImpl->bifToolbar->addWidget( widgets.first );
            fImpl->bifToolbar->addWidget( widgets.second );
            fImpl->bifToolbar->addSeparator();

            widgets = fImpl->bifViewerPage->getFrameSkipWidgets();
            fImpl->bifToolbar->addWidget( widgets.first );
            fImpl->bifToolbar->addWidget( widgets.second );
            fImpl->bifToolbar->addSeparator();

            fImpl->directory->setDelay( 1000 );
            fImpl->directory->setIsOKFunction( []( const QString &dirName )
                                               {
                                                   auto fi = QFileInfo( dirName );
                                                   return dirName.isEmpty() || ( fi.exists() && fi.isDir() && fi.isExecutable() );
                                               }, tr( "Directory '%1' does not Exist or is not a Directory" ) );

            auto completer = new QCompleter( this );
            auto fsModel = new QFileSystemModel( completer );
            fsModel->setRootPath( "/" );
            completer->setModel( fsModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->directory->setCompleter( completer );
            connect( fImpl->directory, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotDirectoryChanged );
            connect( fImpl->directory, &CDelayComboBox::editTextChanged, this, &CMainWindow::slotDirectoryChangedImmediate );
            connect( fImpl->directory->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotLoad );

            completer = new QCompleter( this );
            fsModel = new QFileSystemModel( completer );
            fsModel->setRootPath( "/" );
            completer->setModel( fsModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->fileName->setCompleter( completer );

            fImpl->fileName->setDelay( 1000 );
            fImpl->fileName->setIsOKFunction( []( const QString &fileName )
                                             {
                                                 auto fi = QFileInfo( fileName );
                                                 return fileName.isEmpty() || ( fi.exists() && fi.isFile() && fi.isReadable() );
                                             }, tr( "File '%1' does not Exist or is not Readable" ) );
            connect( fImpl->fileName, &CDelayComboBox::sigEditTextChangedAfterDelay, this, &CMainWindow::slotFileChanged );
            connect( fImpl->fileName->lineEdit(), &CDelayLineEdit::sigFinishedEditingAfterDelay, this, &CMainWindow::slotFileFinishedEditing );

            fImpl->menuBIFPlayer->addAction( fImpl->bifViewerPage->actionSkipBackward() );
            fImpl->menuBIFPlayer->addAction( fImpl->bifViewerPage->actionPrev() );
            fImpl->menuBIFPlayer->addSeparator();
            fImpl->menuBIFPlayer->addAction( fImpl->bifViewerPage->actionTogglePlayPause() );
            fImpl->menuBIFPlayer->addSeparator();
            fImpl->menuBIFPlayer->addAction( fImpl->bifViewerPage->actionPause() );
            fImpl->menuBIFPlayer->addAction( fImpl->bifViewerPage->actionPlay() );
            fImpl->menuBIFPlayer->addSeparator();
            fImpl->menuBIFPlayer->addAction( fImpl->bifViewerPage->actionNext() );
            fImpl->menuBIFPlayer->addAction( fImpl->bifViewerPage->actionSkipForward() );

            fImpl->bifToolbar->addAction( fImpl->bifViewerPage->actionSkipBackward() );
            fImpl->bifToolbar->addAction( fImpl->bifViewerPage->actionPrev() );
            fImpl->bifToolbar->addSeparator();
            fImpl->bifToolbar->addAction( fImpl->bifViewerPage->actionTogglePlayPause() );
            fImpl->bifToolbar->addSeparator();
            fImpl->bifToolbar->addAction( fImpl->bifViewerPage->actionPause() );
            fImpl->bifToolbar->addAction( fImpl->bifViewerPage->actionPlay() );
            fImpl->bifToolbar->addSeparator();
            fImpl->bifToolbar->addAction( fImpl->bifViewerPage->actionNext() );
            fImpl->bifToolbar->addAction( fImpl->bifViewerPage->actionSkipForward() );

            fImpl->mediaNamerFiles->setExpandsOnDoubleClick( false );
            connect( fImpl->mediaNamerFiles, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

            fImpl->mergeSRTFiles->setExpandsOnDoubleClick( false );
            connect( fImpl->mergeSRTFiles, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

            connect( fImpl->actionOpen, &QAction::triggered, this, &CMainWindow::slotOpen );
            connect( fImpl->actionLoad, &QAction::triggered, this, &CMainWindow::slotLoad );
            connect( fImpl->actionRun, &QAction::triggered, this, &CMainWindow::slotRun );

            connect( fImpl->actionTreatAsTVShowByDefault, &QAction::triggered, this, &CMainWindow::slotToggleTreatAsTVShowByDefault );
            connect( fImpl->actionPreferences, &QAction::triggered, this, &CMainWindow::slotPreferences );

            fSearchTMDB = new NCore::CSearchTMDB( nullptr, std::optional<QString>(), this );
            fSearchTMDB->setSkipImages( true );
            connect( fSearchTMDB, &NCore::CSearchTMDB::sigAutoSearchFinished, this, &CMainWindow::slotAutoSearchFinished );

            connect( fImpl->tabWidget, &QTabWidget::currentChanged, this, &CMainWindow::slotWindowChanged );

            loadSettings();

            QSettings settings;
            fImpl->tabWidget->setCurrentIndex( settings.value( "LastFunctionalityPage", 0 ).toInt() );
            if ( settings.contains( "mergeSRTSplitter" ) )
                fImpl->mergeSRTSplitter->restoreState( settings.value( "mergeSRTSplitter" ).toByteArray() );
            else
                fImpl->mergeSRTSplitter->setSizes( QList< int >() << 100 << 0 );

            setBIFPlayerButtonsLayout( fImpl->bifViewerPage->buttonsLayout() );

            connect( fImpl->actionBIFPlayerButtonDiscrete, &QAction::triggered, this, &CMainWindow::slotBIFPlayerButtonDiscrete );
            connect( fImpl->actionBIFPlayerButtonToggle, &QAction::triggered, this, &CMainWindow::slotBIFPlayerButtonToggle );
            connect( fImpl->actionBIFPlayerButtonNone, &QAction::triggered, this, &CMainWindow::slotBIFPlayerButtonNone );

            QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChangedImmediate );
            QTimer::singleShot( 10, this, &CMainWindow::slotDirectoryChanged );

            QTimer::singleShot( 0, this, &CMainWindow::slotWindowChanged );
        }


        CMainWindow::~CMainWindow()
        {
            saveSettings();
            QSettings settings;
            settings.setValue( "LastFunctionalityPage", fImpl->tabWidget->currentIndex() );
            settings.setValue( "mergeSRTSplitter", fImpl->mergeSRTSplitter->saveState() );
        }

        void CMainWindow::loadSettings()
        {
            fImpl->directory->addItems( NCore::CPreferences::instance()->getDirectories(), true );
            fImpl->fileName->addItems( NCore::CPreferences::instance()->getFileNames(), true );
            fImpl->actionTreatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->actionExactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );

            slotToggleTreatAsTVShowByDefault();
        }

        //settings.setValue( "Extensions", fImpl->extensions->text() );
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

            fImpl->bifToolbar->setVisible( isBIFViewerActive() );

            validateRunAction();
            validateLoadAction();

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
            fImpl->actionRun->setEnabled( !isBIFViewerActive() && getActiveModel() && getActiveModel()->rowCount() );
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
            else if ( isMergeSRTActive() || isTransformActive() )
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
                slotToggleTreatAsTVShowByDefault();

                if ( fXformModel )
                {
                    fXformModel->slotTVOutputDirPatternChanged( NCore::CPreferences::instance()->getTVOutDirPattern() );
                    fXformModel->slotTVOutputFilePatternChanged( NCore::CPreferences::instance()->getTVOutFilePattern() );

                    fXformModel->slotMovieOutputDirPatternChanged( NCore::CPreferences::instance()->getMovieOutDirPattern() );
                    fXformModel->slotMovieOutputFilePatternChanged( NCore::CPreferences::instance()->getMovieOutFilePattern() );
                }
            }
        }

        void CMainWindow::slotMergeSRTDirectoryLoaded()
        {
        }

        void CMainWindow::slotAutoSearchForNewNames()
        {
            if ( !fXformModel )
                return;

            if ( fXformModel->rowCount() != 1 )
                return;

            fSearchTMDB->resetResults();

            auto count = NQtUtils::itemCount( fXformModel.get(), true );

            setupProgressDlg( tr( "Finding Results" ), tr( "Cancel" ), count );

            auto rootIdx = fXformModel->index( 0, 0 );
            autoSearchForNewNames( rootIdx );
            fProgressDlg->setValue( fSearchesCompleted );
        }

        void CMainWindow::autoSearchForNewNames( QModelIndex parentIdx )
        {
            auto rowCount = fXformModel->rowCount( parentIdx );
            for ( int ii = 0; ii < rowCount; ++ii )
            {
                if ( fProgressDlg->wasCanceled() )
                {
                    fSearchTMDB->clearSearchCache();
                    break;
                }

                auto childIndex = fXformModel->index( ii, 0, parentIdx );
                auto name = fXformModel->getSearchName( childIndex );
                auto path = fXformModel->filePath( childIndex );
                auto titleInfo = fXformModel->getSearchResultInfo( childIndex );
                auto searchInfo = std::make_shared< NCore::SSearchTMDBInfo >( name, titleInfo );
                searchInfo->setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly() );

                if ( fXformModel->canAutoSearch( childIndex ) )
                {
                    if ( fProgressDlg )
                    {
                        fProgressDlg->setLabelText( tr( "Adding Background Search for '%1'" ).arg( QDir( fImpl->directory->currentText() ).relativeFilePath( path ) ) );
                        fProgressDlg->setValue( fProgressDlg->value() + 1 );
                        qApp->processEvents();
                    }
                    fSearchTMDB->addSearch( path, searchInfo );
                }

                autoSearchForNewNames( childIndex );
            }
        }

        void CMainWindow::slotAutoSearchFinished( const QString &path, bool searchesRemaining )
        {
            auto results = fSearchTMDB->getResults( path );

            //qDebug().noquote().nospace() << "Search results for path " << path << " Has Result? " << ( ( results.size() == 1 ) ? "Yes" : "No" );
            if ( searchesRemaining )
            {
                if ( fProgressDlg )
                {
                    fProgressDlg->setValue( fProgressDlg->value() + 1 );
                    fSearchesCompleted++;
                    fProgressDlg->setLabelText( tr( "Search Complete for '%1'" ).arg( QDir( fImpl->directory->currentText() ).relativeFilePath( path ) ) );
                }
            }
            else
            {
                clearProgressDlg();
            }

            if ( fProgressDlg && fProgressDlg->wasCanceled() )
                fSearchTMDB->clearSearchCache();

            if ( results.size() != 1 )
                return;
            auto result = results.front();
            //qDebug() << result->toString();

            auto item = fXformModel->getItemFromPath( path );
            if ( item && result )
            {
                fXformModel->setSearchResult( item, result, false );
            }
        }

        void CMainWindow::clearProgressDlg()
        {
            delete fProgressDlg;
            fProgressDlg = nullptr;
            fImpl->actionOpen->setEnabled( true );
        }

        void CMainWindow::setupProgressDlg( const QString &title, const QString &cancelButtonText, int max )
        {
            fImpl->actionOpen->setEnabled( false );
            if ( fProgressDlg )
                fProgressDlg->reset();

            if ( !fProgressDlg )
            {
                fProgressDlg = new QProgressDialog( this );
            }
            fProgressDlg->setWindowModality( Qt::WindowModal );
            fProgressDlg->setMinimumDuration( 0 );
            fProgressDlg->setAutoClose( false );
            fProgressDlg->setAutoReset( false );

            fProgressDlg->setWindowTitle( title );
            fProgressDlg->setCancelButtonText( cancelButtonText );
            fProgressDlg->setRange( 0, max );
            fProgressDlg->show();

            connect( fProgressDlg, &QProgressDialog::canceled, 
                     [this]()
                     {
                         fImpl->actionOpen->setEnabled( true );
                     } );
        }

        void CMainWindow::slotDoubleClicked( const QModelIndex &idx )
        {
            if ( !isTransformActive() )
                return;

            auto baseIdx = fXformModel->index( idx.row(), NCore::EColumns::eFSName, idx.parent() );
            auto titleInfo = fXformModel->getSearchResultInfo( idx );

            auto isDir = baseIdx.data( NCore::ECustomRoles::eIsDir ).toBool();
            auto fullPath = baseIdx.data( NCore::ECustomRoles::eFullPathRole ).toString();
            bool isTVShow = baseIdx.data( NCore::ECustomRoles::eIsTVShowRole ).toBool();
            auto nm = fXformModel->getSearchName( idx );

            CSelectTMDB dlg( nm, titleInfo, this );
            dlg.setSearchForTVShows( fXformModel->treatAsTVShow( QFileInfo( fullPath ), isTVShow ), true );
            dlg.setExactMatchOnly( fImpl->actionExactMatchesOnly->isChecked(), true );

            if ( dlg.exec() == QDialog::Accepted )
            {
                auto titleInfo = dlg.getSearchResult();
                bool setChildren = true;
                if ( titleInfo->isTVShow() && titleInfo->isSeasonOnly() )
                    setChildren = false;
                fXformModel->setSearchResult( idx, titleInfo, setChildren );
            }
        }

        void CMainWindow::slotToggleTreatAsTVShowByDefault()
        {
            if ( fXformModel )
                fXformModel->slotTreatAsTVByDefaultChanged( fImpl->actionTreatAsTVShowByDefault->isChecked() );
        }

        bool CMainWindow::isTransformActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->mediaNamerTab;
        }

        bool CMainWindow::isMergeSRTActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->mergeSRTTab;
        }


        bool CMainWindow::isBIFViewerActive() const
        {
            return fImpl->tabWidget->currentWidget() == fImpl->bifViewerTab;
        }

        NCore::CDirModel * CMainWindow::getActiveModel() const
        {
            if ( isTransformActive() )
                return fXformModel.get();
            else if ( isMergeSRTActive() )
                return fMergeSRTModel.get();
            return nullptr;
        }

        void CMainWindow::slotLoadFinished( bool canceled )
        {
            validateRunAction();
            clearProgressDlg();

            if ( canceled )
                return;

            if ( isTransformActive() )
                QTimer::singleShot( 0, this, &CMainWindow::slotAutoSearchForNewNames );
        }

        void CMainWindow::slotLoad()
        {
            bool aOK = true;
            fImpl->directory->addCurrentItem();
            if ( isTransformActive() )
            {
                fXformModel.reset( new NCore::CDirModel( NCore::CDirModel::eTransform ) );
                fImpl->mediaNamerFiles->setModel( fXformModel.get() );
                connect( fXformModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMainWindow::slotLoadFinished );
                fXformModel->slotTreatAsTVByDefaultChanged( fImpl->actionTreatAsTVShowByDefault->isChecked() );
                fXformModel->slotTVOutputFilePatternChanged( NCore::CPreferences::instance()->getTVOutFilePattern() );
                fXformModel->slotTVOutputDirPatternChanged( NCore::CPreferences::instance()->getTVOutDirPattern() );
                fXformModel->slotMovieOutputFilePatternChanged( NCore::CPreferences::instance()->getMovieOutFilePattern() );
                fXformModel->slotMovieOutputDirPatternChanged( NCore::CPreferences::instance()->getMovieOutDirPattern() );
                fXformModel->setNameFilters( NCore::CPreferences::instance()->getMediaExtensions() << NCore::CPreferences::instance()->getSubtitleExtensions(), fImpl->mediaNamerFiles );
                setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
                fXformModel->setRootPath( fImpl->directory->currentText(), fImpl->mediaNamerFiles, nullptr, fProgressDlg );
            }
            else if ( isMergeSRTActive() )
            {
                fMergeSRTModel.reset( new NCore::CDirModel( NCore::CDirModel::eMergeSRT ) );
                fImpl->mergeSRTFiles->setModel( fMergeSRTModel.get() );
                connect( fMergeSRTModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMainWindow::slotLoadFinished );
                fMergeSRTModel->setNameFilters( QStringList() << "*.mkv", fImpl->mergeSRTFiles );
                setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
                fMergeSRTModel->setRootPath( fImpl->directory->currentText(), fImpl->mergeSRTFiles, fImpl->mergeSRTResults, fProgressDlg );
            }
            fImpl->actionRun->setEnabled( false );
        }

        void CMainWindow::slotRun()
        {
            bool processOK = false;
            QString actionName;
            QString cancelName;

            NCore::CDirModel *model = nullptr;
            if ( isTransformActive() )
            {
                actionName = tr( "Renaming Files..." );
                cancelName = tr( "Abort Rename" );
                model = fXformModel.get();
            }
            else if ( isMergeSRTActive() )
            {
                auto sizes = fImpl->mergeSRTSplitter->sizes();
                if ( sizes.back() == 0 )
                {
                    sizes.front() -= 30;
                    sizes.back() = 30;

                    fImpl->mergeSRTSplitter->setSizes( sizes );
                }

                actionName = tr( "Merging SRT Files into MKV..." );
                cancelName = tr( "Abort Merge" );
                model = fMergeSRTModel.get();
            }
            else 
                return;

            if( model && model->process(
                    [actionName, cancelName, this]( int count ) { setupProgressDlg( actionName, cancelName, count ); return fProgressDlg; },
                    [this]( QProgressDialog *dlg ) { (void)dlg; clearProgressDlg(); },
                    this ) )
                    slotLoad();
                ;
        }

        void CMainWindow::setBIFPlayerButtonsLayout( NBIF::EButtonsLayout layout )
        {
            fImpl->actionBIFPlayerButtonDiscrete->setChecked( false );
            fImpl->actionBIFPlayerButtonToggle->setChecked( false );
            fImpl->actionBIFPlayerButtonNone->setChecked( false );

            if ( layout == NBIF::EButtonsLayout::eDiscretePlayPause )
                fImpl->actionBIFPlayerButtonDiscrete->setChecked( true );
            else if ( layout == NBIF::EButtonsLayout::eTogglePlayPause )
                fImpl->actionBIFPlayerButtonToggle->setChecked( true );
            else if ( layout == NBIF::EButtonsLayout::eNoButtons )
                fImpl->actionBIFPlayerButtonNone->setChecked( true );

            fImpl->bifViewerPage->setButtonsLayout( layout );
        }

        void CMainWindow::slotBIFPlayerButtonDiscrete()
        {
            setBIFPlayerButtonsLayout( NBIF::EButtonsLayout::eDiscretePlayPause );
        }

        void CMainWindow::slotBIFPlayerButtonToggle()
        {
            setBIFPlayerButtonsLayout( NBIF::EButtonsLayout::eTogglePlayPause );
        }

        void CMainWindow::slotBIFPlayerButtonNone()
        {
            setBIFPlayerButtonsLayout( NBIF::EButtonsLayout::eNoButtons );
        }
    }
}

