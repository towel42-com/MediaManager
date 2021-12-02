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

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QTimer>

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

            fImpl->directory->setDelay( 1000 );

            auto completer = new QCompleter( this );
            auto fsModel = new QFileSystemModel( completer );
            fsModel->setRootPath( "/" );
            completer->setModel( fsModel );
            completer->setCompletionMode( QCompleter::PopupCompletion );
            completer->setCaseSensitivity( Qt::CaseInsensitive );

            fImpl->directory->setCompleter( completer );
            connect( fImpl->directory, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotDirectoryChanged );
            connect( fImpl->directory, &CDelayLineEdit::textChanged, this, &CMainWindow::slotDirectoryChangedImmediate );


            fImpl->mediaNamerFiles->setExpandsOnDoubleClick( false );
            connect( fImpl->mediaNamerFiles, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

            fImpl->mergeSRTFiles->setExpandsOnDoubleClick( false );
            connect( fImpl->mergeSRTFiles, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

            connect( fImpl->actionSelectDir, &QAction::triggered, this, &CMainWindow::slotSelectDirectory );
            connect( fImpl->actionLoad, &QAction::triggered, this, &CMainWindow::slotLoadDirectory );
            connect( fImpl->actionRun, &QAction::triggered, this, &CMainWindow::slotRun );

            connect( fImpl->actionTreatAsTVShowByDefault, &QAction::triggered, this, &CMainWindow::slotToggleTreatAsTVShowByDefault );
            connect( fImpl->actionPreferences, &QAction::triggered, this, &CMainWindow::slotPreferences );

            fSearchTMDB = new NCore::CSearchTMDB( nullptr, std::optional<QString>(), this );
            fSearchTMDB->setSkipImages( true );
            connect( fSearchTMDB, &NCore::CSearchTMDB::sigAutoSearchFinished, this, &CMainWindow::slotAutoSearchFinished );

            loadSettings();

            QSettings settings;
            fImpl->tabWidget->setCurrentIndex( settings.value( "LastFunctionalityPage", 0 ).toInt() );

            QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChanged );
        }

        CMainWindow::~CMainWindow()
        {
            saveSettings();
            QSettings settings;
            settings.setValue( "LastFunctionalityPage", fImpl->tabWidget->currentIndex() );
        }

        void CMainWindow::loadSettings()
        {
            fImpl->directory->setText( NCore::CPreferences::instance()->getMediaDirectory() );
            fImpl->actionTreatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->actionExactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );

            slotToggleTreatAsTVShowByDefault();
        }

        //settings.setValue( "Extensions", fImpl->extensions->text() );
        void CMainWindow::saveSettings()
        {
            NCore::CPreferences::instance()->setMediaDirectory( fImpl->directory->text() );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->actionTreatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->actionExactMatchesOnly->isChecked() );
        }

        void CMainWindow::slotDirectoryChangedImmediate()
        {
            fImpl->actionLoad->setEnabled( false );
            fImpl->actionRun->setEnabled( false );
        }

        void CMainWindow::slotDirectoryChanged()
        {
            slotDirectoryChangedImmediate();
            CAutoWaitCursor awc;

            auto dirName = fImpl->directory->text();

            QFileInfo fi( dirName );
            fImpl->actionLoad->setEnabled( !dirName.isEmpty() && fi.exists() && fi.isDir() );
        }

        void CMainWindow::slotSelectDirectory()
        {
            auto dir = QFileDialog::getExistingDirectory( this, tr( "Select Directory:" ), fImpl->directory->text() );
            if ( !dir.isEmpty() )
                fImpl->directory->setText( dir );
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

        void CMainWindow::slotMergeSRTDirectoryLoaded( bool /*canceled*/ )
        {
            clearProgressDlg();
        }

        void CMainWindow::slotAutoSearchForNewNames( bool canceled )
        {
            clearProgressDlg();
            if ( canceled )
                return;

            if ( !fXformModel )
                return;

            if ( fXformModel->rowCount() != 1 )
                return;

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
                        fProgressDlg->setLabelText( tr( "Adding Background Search for '%1'" ).arg( QDir( fImpl->directory->text() ).relativeFilePath( path ) ) );
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

            qDebug().noquote().nospace() << "Search results for path " << path << " Has Result? " << ( ( results.size() == 1 ) ? "Yes" : "No" );
            if ( searchesRemaining )
            {
                if ( fProgressDlg )
                {
                    fProgressDlg->setValue( fProgressDlg->value() + 1 );
                    fSearchesCompleted++;
                    fProgressDlg->setLabelText( tr( "Search Complete for '%1'" ).arg( QDir( fImpl->directory->text() ).relativeFilePath( path ) ) );
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
            qDebug() << result->toString();

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
            fImpl->actionSelectDir->setEnabled( true );
        }

        void CMainWindow::setupProgressDlg( const QString &title, const QString &cancelButtonText, int max )
        {
            fImpl->actionSelectDir->setEnabled( false );
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
        }

        void CMainWindow::slotDoubleClicked( const QModelIndex &idx )
        {
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

        void CMainWindow::slotLoadDirectory()
        {
            bool aOK = true;
            if ( isTransformActive() )
            {
                fXformModel.reset( new NCore::CDirModel( NCore::CDirModel::eTransform ) );
                fImpl->mediaNamerFiles->setModel( fXformModel.get() );
                connect( fXformModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMainWindow::slotAutoSearchForNewNames );
                fXformModel->slotTreatAsTVByDefaultChanged( fImpl->actionTreatAsTVShowByDefault->isChecked() );
                fXformModel->slotTVOutputFilePatternChanged( NCore::CPreferences::instance()->getTVOutFilePattern() );
                fXformModel->slotTVOutputDirPatternChanged( NCore::CPreferences::instance()->getTVOutDirPattern() );
                fXformModel->slotMovieOutputFilePatternChanged( NCore::CPreferences::instance()->getMovieOutFilePattern() );
                fXformModel->slotMovieOutputDirPatternChanged( NCore::CPreferences::instance()->getMovieOutDirPattern() );
                fXformModel->setNameFilters( NCore::CPreferences::instance()->getMediaExtensions() << NCore::CPreferences::instance()->getSubtitleExtensions(), fImpl->mediaNamerFiles );
                setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
                fXformModel->setRootPath( fImpl->directory->text(), fImpl->mediaNamerFiles, fProgressDlg );
            }
            else if ( isMergeSRTActive() )
            {
                fMergeSRTModel.reset( new NCore::CDirModel( NCore::CDirModel::eMergeSRT ) );
                fImpl->mergeSRTFiles->setModel( fMergeSRTModel.get() );
                connect( fMergeSRTModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMainWindow::slotMergeSRTDirectoryLoaded );
                fMergeSRTModel->setNameFilters( QStringList() << "*.srt", fImpl->mergeSRTFiles );
                setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
                fMergeSRTModel->setRootPath( fImpl->directory->text(), fImpl->mergeSRTFiles, fProgressDlg );
                fImpl->actionRun->setEnabled( true );
            }
            else
                aOK = false;

            fImpl->actionRun->setEnabled( aOK );
        }

        void CMainWindow::slotRun()
        {
            if ( isTransformActive() )
            {
                if ( fXformModel->process(
                    [this]( int count ) { setupProgressDlg( tr( "Renaming Files..." ), tr( "Abort Rename" ), count ); return fProgressDlg; },
                    [this]( QProgressDialog *dlg ) { (void)dlg; clearProgressDlg(); },
                    this ) )
                    slotLoadDirectory();
                ;
            }
            else if ( isMergeSRTActive() )
            {
                if ( fMergeSRTModel->process(
                    [this]( int count ) { setupProgressDlg( tr( "Merging SRT Files into MKV..." ), tr( "Abort Merge" ), count ); return fProgressDlg; },
                    [this]( QProgressDialog *dlg ) { (void)dlg; clearProgressDlg(); },
                    this ) )
                    slotLoadDirectory();
                ;
            }
        }
    }
}

