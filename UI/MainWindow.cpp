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

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QTimer>
#include <QPixmap>
#include <QLabel>
#include <QMovie>


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
            formatBIFTable();

            fImpl->directory->setDelay( 1000 );
            fImpl->directory->setIsOKFunction( []( const QString &dirName )
                                               {
                                                   auto fi = QFileInfo( dirName );
                                                   return dirName.isEmpty() || ( fi.exists() && fi.isDir() );
                                               } );
            fImpl->bifFile->setDelay( 1000 );
            fImpl->bifFile->setIsOKFunction( []( const QString &fileName )
                                             {
                                                 auto fi = QFileInfo( fileName );
                                                 return fileName.isEmpty() || ( fi.exists() && fi.isFile() && fi.isReadable() );
                                             } );

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

            connect( fImpl->openBIFBtn, &QToolButton::clicked, this, &CMainWindow::slotSelectBIFFile );
            connect( fImpl->bifFile, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotBIFFileChanged );
            connect( fImpl->bifStepBack, &QToolButton::clicked, this, &CMainWindow::slotBIFBack );
            connect( fImpl->bifStepForward, &QToolButton::clicked, this, &CMainWindow::slotBIFForward );
            connect( fImpl->bifPlayPause, &QToolButton::clicked, this, &CMainWindow::slotBIFPlayPause );
            connect( fImpl->bifTS, qOverload< int >( &QSpinBox::valueChanged ), this, &CMainWindow::slotBIFTSChanged );

            connect( fImpl->actionSelectDir, &QAction::triggered, this, &CMainWindow::slotSelectDirectory );
            connect( fImpl->actionLoad, &QAction::triggered, this, &CMainWindow::slotLoadDirectory );
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

            if ( settings.contains( "bifViewerVSplitter" ) )
                fImpl->bifViewerVSplitter->restoreState( settings.value( "bifViewerVSplitter" ).toByteArray() );
            else
                fImpl->bifViewerVSplitter->setSizes( QList< int >() << 100 << 100 );

            if ( settings.contains( "bifViewerHSplitter" ) )
                fImpl->bifViewerHSplitter->restoreState( settings.value( "bifViewerHSplitter" ).toByteArray() );
            else
                fImpl->bifViewerHSplitter->setSizes( QList< int >() << 100 << 0 );
#ifdef _DEBUG
            fImpl->bifFile->setText( settings.value( "LastBIFFile", "//mediabox/video/Movies/The Last Duel (2021) [tmdbid=617653]/The Last Duel-320-10.bif" ).toString() );
#endif

            fBIFFrameTimer = new QTimer( this );
            fBIFFrameTimer->setSingleShot( false );
            fBIFFrameTimer->setInterval( NCore::CPreferences::instance()->bifTSInterval() );
            fCurrentFrame = 0;
            connect( fBIFFrameTimer, &QTimer::timeout, this, &CMainWindow::slotNextFrame );

            fResizeTimer = new QTimer( this );
            fResizeTimer->setSingleShot( true );
            fResizeTimer->setInterval( 250 );
            connect( fResizeTimer, &QTimer::timeout, this, &CMainWindow::slotResize );

            QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChangedImmediate );
            QTimer::singleShot( 10, this, &CMainWindow::slotDirectoryChanged );

            QTimer::singleShot( 0, this, &CMainWindow::slotWindowChanged );
            QTimer::singleShot( 10, this, &CMainWindow::slotBIFFileChanged );
        }

        void CMainWindow::formatBIFTable()
        {
            fImpl->bifFileValues->resizeColumnToContents( 0 );
            fImpl->bifFileValues->resizeColumnToContents( 1 );
            for ( int ii = 0; ii < fImpl->bifFileValues->topLevelItemCount(); ++ii )
            {
                fImpl->bifFileValues->topLevelItem( ii )->setTextAlignment( 1, Qt::AlignRight | Qt::AlignVCenter );
            }
        }

        void CMainWindow::showCurrentBIFFrame()
        {
            auto &&curr = fBIF->bifs()[fCurrentFrame];
            fImpl->bifImageLabel->setPixmap( QPixmap::fromImage( curr.fImage.second ) );
            fImpl->bifLabel->setText( tr( "BIF #: %1" ).arg( fCurrentFrame ) );
        }

        void CMainWindow::pauseBIF()
        {
            fBIFFrameTimer->stop();
            if ( fBIF )
            {
                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/play.png" ), QSize(), QIcon::Normal, QIcon::Off );
                fImpl->bifPlayPause->setIcon( icon );
            }
        }

        void CMainWindow::playBIF()
        {
            if ( !fBIF )
                return;

            fBIFFrameTimer->start();
            if ( fBIF )
            {
                QIcon icon;
                icon.addFile( QString::fromUtf8( ":/resources/pause.png" ), QSize(), QIcon::Normal, QIcon::Off );
                fImpl->bifPlayPause->setIcon( icon );
            }
        }

        CMainWindow::~CMainWindow()
        {
            saveSettings();
            QSettings settings;
            settings.setValue( "LastFunctionalityPage", fImpl->tabWidget->currentIndex() );
            settings.setValue( "mergeSRTSplitter", fImpl->mergeSRTSplitter->saveState() );
            settings.setValue( "bifViewerVSplitter", fImpl->bifViewerVSplitter->saveState() );
            settings.setValue( "bifViewerHSplitter", fImpl->bifViewerHSplitter->saveState() );
        }

        void CMainWindow::loadSettings()
        {
            fImpl->directory->setText( NCore::CPreferences::instance()->getMediaDirectory() );
            fImpl->actionTreatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->actionExactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );
            fImpl->bifTS->setValue( NCore::CPreferences::instance()->bifTSInterval() );

            slotToggleTreatAsTVShowByDefault();
        }

        //settings.setValue( "Extensions", fImpl->extensions->text() );
        void CMainWindow::saveSettings()
        {
            NCore::CPreferences::instance()->setMediaDirectory( fImpl->directory->text() );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->actionTreatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->actionExactMatchesOnly->isChecked() );
            NCore::CPreferences::instance()->setBIFTSInterval( fImpl->bifTS->value() );
        }

        void CMainWindow::slotWindowChanged()
        {
            fImpl->dirLabel->setVisible( !isBIFViewerActive() );
            fImpl->directory->setVisible( !isBIFViewerActive() );
            fImpl->dirLabel->setVisible( !isBIFViewerActive() );
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

            auto dirName = fImpl->directory->text();

            QFileInfo fi( dirName );
            bool aOK = !dirName.isEmpty() && fi.exists() && fi.isDir();
            fImpl->actionLoad->setEnabled( aOK );
        }


        void CMainWindow::resizeEvent( QResizeEvent * /*event*/ )
        {
            fResizeTimer->stop();
            fResizeTimer->start();
        }

        void CMainWindow::slotResize()
        {
            auto windowSize = 1.0 * fImpl->bifImages->size().width();
            auto itemSize = 1.0 * fImpl->bifImages->iconSize().width() + 16.0; // default size of each image;

            auto num = std::floor( windowSize / itemSize );

            auto numPerRow = 4;
            if ( num != numPerRow )
            {
                itemSize = std::ceil( windowSize / numPerRow ) - 16;
            }

            fImpl->bifImages->setIconSize( QSize( itemSize, itemSize ) );
        }

        void CMainWindow::slotSelectBIFFile()
        {
            auto bifFile = QFileDialog::getOpenFileName( this, tr( "Select BIF File:" ), fImpl->bifFile->text(), tr( "BIF Files (*.bif);;All Files (*.*)" ) );
            if ( !bifFile.isEmpty() )
                fImpl->bifFile->setText( bifFile );
        }
        

        void CMainWindow::slotBIFFileChanged()
        {
            auto bifFile = fImpl->bifFile->text();
            auto fi = QFileInfo( bifFile );
            bool aOK = !bifFile.isEmpty() && fi.exists() && fi.isFile() && fi.isReadable();

            fImpl->bifFileValues->clear();
            fImpl->bifImages->clear();
            fImpl->bifLabel->setPixmap( QPixmap() );
            pauseBIF();
            if ( !aOK )
                return;

            if ( fBIF )
                delete fBIF;

            fBIF = new CBIFFile( bifFile );
            if ( !fBIF->isValid() )
            {
                QMessageBox::warning( this, tr( "Could not Load" ), tr( "Could not load BIF File: %1" ).arg( fBIF->errorString() ) );
                delete fBIF;
                fBIF = nullptr;
                return;
            }
            loadBIF();
            playBIF();
            QTimer::singleShot( 0, this, &CMainWindow::slotNextFrame );
        }

        void CMainWindow::slotBIFBack()
        {
            pauseBIF();
            setCurrentFrame( fCurrentFrame - 1 );
            showCurrentBIFFrame();
        }

        void CMainWindow::slotBIFForward()
        {
            pauseBIF();
            setCurrentFrame( fCurrentFrame + 1 );
            showCurrentBIFFrame();
        }

        void CMainWindow::slotBIFPlayPause()
        {
            if ( fBIFFrameTimer->isActive() )
                pauseBIF();
            else
                playBIF();
        }

        void CMainWindow::slotBIFTSChanged()
        {
            if ( !fBIFFrameTimer )
                return;
            bool isActive = fBIFFrameTimer->isActive();
            if ( isActive )
                fBIFFrameTimer->stop();
            fBIFFrameTimer->setInterval( fImpl->bifTS->value() );
            if ( isActive )
                fBIFFrameTimer->start();
        }

        void CMainWindow::slotNextFrame()
        {
            if ( !fBIF )
            {
                pauseBIF();
                return;
            }
            
            showCurrentBIFFrame();
            setCurrentFrame( fCurrentFrame + 1 );
        }

        void CMainWindow::setCurrentFrame( int frame )
        {
            if ( frame < 0 )
                frame = static_cast<int>( fBIF->bifs().size() ) - 1;
            else if ( frame >= fBIF->bifs().size() )
                frame = 0;
            fCurrentFrame = frame;

        }

        void CMainWindow::loadBIF()
        {
            fImpl->bifFileValues->clear();
            fImpl->bifImages->clear();
            fImpl->bifLabel->setPixmap( QPixmap() );
            if ( !fBIF )
            {
                return;
            }

            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Magic Number" ) << tr( "00-07" ) << QString() << fBIF->magicNumber() );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Version" ) << tr( "08-11" ) << QString::number( std::get< 2 >( fBIF->version() ) ) << std::get< 1 >( fBIF->version() ) );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Number of BIF Images" ) << tr( "12-15" ) << QString::number( std::get< 2 >( fBIF->numImages() ) ) << std::get< 1 >( fBIF->numImages() ) );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Timestamp Multiplier" ) << tr( "16-19" ) << QString::number( std::get< 2 >( fBIF->tsMultiplier() ) ) << std::get< 1 >( fBIF->tsMultiplier() ) );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Reserved" ) << tr( "20-64" ) << QString() << fBIF->reserved() );

            auto pos = 0;
            for( auto && ii : fBIF->bifs() )
            {
                auto item = new QListWidgetItem( QIcon( QPixmap::fromImage( ii.fImage.second ) ), QString( "BIF #%1" ).arg( pos ) );
                fImpl->bifImages->addItem( item );
                pos++;
            }

            formatBIFTable();
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

            //qDebug().noquote().nospace() << "Search results for path " << path << " Has Result? " << ( ( results.size() == 1 ) ? "Yes" : "No" );
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
            fImpl->actionSelectDir->setEnabled( !isBIFViewerActive() );
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

            connect( fProgressDlg, &QProgressDialog::canceled, 
                     [this]()
                     {
                         fImpl->actionSelectDir->setEnabled( !isBIFViewerActive() );
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
            fImpl->actionRun->setEnabled( !isBIFViewerActive() && getActiveModel() && getActiveModel()->rowCount() );
            clearProgressDlg();

            if ( canceled )
                return;

            if ( isTransformActive() )
                QTimer::singleShot( 0, this, &CMainWindow::slotAutoSearchForNewNames );
        }

        void CMainWindow::slotLoadDirectory()
        {
            bool aOK = true;
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
                fXformModel->setRootPath( fImpl->directory->text(), fImpl->mediaNamerFiles, nullptr, fProgressDlg );
                fImpl->actionRun->setEnabled( true );
            }
            else if ( isMergeSRTActive() )
            {
                fMergeSRTModel.reset( new NCore::CDirModel( NCore::CDirModel::eMergeSRT ) );
                fImpl->mergeSRTFiles->setModel( fMergeSRTModel.get() );
                connect( fMergeSRTModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMainWindow::slotLoadFinished );
                fMergeSRTModel->setNameFilters( QStringList() << "*.mkv", fImpl->mergeSRTFiles );
                setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
                fMergeSRTModel->setRootPath( fImpl->directory->text(), fImpl->mergeSRTFiles, fImpl->mergeSRTResults, fProgressDlg );
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

            if( model->process(
                    [actionName, cancelName, this]( int count ) { setupProgressDlg( actionName, cancelName, count ); return fProgressDlg; },
                    [this]( QProgressDialog *dlg ) { (void)dlg; clearProgressDlg(); },
                    this ) )
                    slotLoadDirectory();
                ;
        }
    }
}

