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

#include "FileRenamerLib/DirModel.h"
#include "FileRenamerLib/SearchResult.h"
#include "FileRenamerLib/SearchTMDB.h"
#include "FileRenamerLib/SearchTMDBInfo.h"

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

CMainWindow::CMainWindow( QWidget* parent )
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
    //completer->setCaseSensitivity( Qt::CaseInsensitive );

    fImpl->directory->setCompleter( completer );
    fImpl->files->setExpandsOnDoubleClick( false );

    connect( fImpl->directory, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotDirectoryChanged );
    connect( fImpl->btnTransform, &QPushButton::clicked, this, &CMainWindow::slotTransform );

    connect( fImpl->btnSelectDir, &QPushButton::clicked, this, &CMainWindow::slotSelectDirectory );
    connect( fImpl->btnLoad, &QPushButton::clicked, this, &CMainWindow::slotLoadDirectory );
    connect( fImpl->actionTreatAsTVShowByDefault, &QAction::triggered, this, &CMainWindow::slotToggleTreatAsTVShowByDefault );
    connect( fImpl->actionPreferences, &QAction::triggered, this, &CMainWindow::slotPreferences );

    connect( fImpl->files, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

    fSearchTMDB = new NFileRenamerLib::CSearchTMDB( nullptr, std::optional<QString>(), this );
    fSearchTMDB->setSkipImages( true );
    connect( fSearchTMDB, &NFileRenamerLib::CSearchTMDB::sigAutoSearchFinished, this, &CMainWindow::slotAutoSearchFinished );

    loadSettings();

    QTimer::singleShot( 0, this, &CMainWindow::slotDirectoryChanged );
}

CMainWindow::~CMainWindow()
{
    saveSettings();
}

QString CMainWindow::getDefaultInPattern( bool forTV ) const
{
    if ( forTV )
        return "(?<title>.+)\\.([Ss](?<season>\\d+))([Ee](?<episode>\\d+))(\\.(?<episode_title>.*))?\\.(1080|720|2160)(p|i)?.*";
    else
        return "(?<title>.+)\\.(?<year>\\d{2,4})\\..*";
}
    

QString CMainWindow::getDefaultOutDirPattern( bool forTV ) const
{
    if ( forTV )
        return "<title> - Season <season>";
    else
        return "<title> (<year>)( [tmdbid=<tmdbid>]):<tmdbid>( - <extra_info>):<extra_info>";
}

QString CMainWindow::getDefaultOutFilePattern( bool forTV ) const
{
    if ( forTV )
        return "<title> - S<season>E<episode>( - <episode_title>):<episode_title>( - <extra_info>):<extra_info>";
    else
        return "<title>";
}

void CMainWindow::loadSettings()
{
    fImpl->directory->setText( CPreferences::getMediaDirectory() );
    fImpl->actionTreatAsTVShowByDefault->setChecked( CPreferences::getTreatAsTVShowByDefault() );
    fImpl->actionExactMatchesOnly->setChecked( CPreferences::getExactMatchesOnly() );

    slotToggleTreatAsTVShowByDefault();
}

//settings.setValue( "Extensions", fImpl->extensions->text() );
void CMainWindow::saveSettings()
{
    CPreferences::setMediaDirectory( fImpl->directory->text() );
    CPreferences::setTreatAsTVShowByDefault( fImpl->actionTreatAsTVShowByDefault->isChecked() );
    CPreferences::setExactMatchesOnly( fImpl->actionExactMatchesOnly->isChecked() );
}

void CMainWindow::slotDirectoryChanged()
{
    fImpl->btnLoad->setEnabled( false );
    fImpl->btnTransform->setEnabled( false );
    CAutoWaitCursor awc;

    auto dirName = fImpl->directory->text();

    QFileInfo fi( dirName );
    fImpl->btnLoad->setEnabled( !dirName.isEmpty() && fi.exists() && fi.isDir() );
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
        fImpl->actionTreatAsTVShowByDefault->setChecked( CPreferences::getTreatAsTVShowByDefault() );
        fImpl->actionExactMatchesOnly->setChecked( CPreferences::getExactMatchesOnly() );
        slotToggleTreatAsTVShowByDefault();
        
        if ( fDirModel )
        {
            fDirModel->slotTVOutputDirPatternChanged( CPreferences::getTVOutDirPattern() );
            fDirModel->slotTVOutputFilePatternChanged( CPreferences::getTVOutFilePattern() );

            fDirModel->slotMovieOutputDirPatternChanged( CPreferences::getMovieOutDirPattern() );
            fDirModel->slotMovieOutputFilePatternChanged( CPreferences::getMovieOutFilePattern() );
        }
    }
}

void CMainWindow::slotAutoSearch()
{
    if ( !fDirModel )
        return;

    if ( fDirModel->rowCount() != 1 )
        return;

    bool wasCanceled = fProgressDlg && fProgressDlg->wasCanceled();
    clearProgressDlg();
    if ( wasCanceled )
        return;

    auto count = NQtUtils::itemCount( fDirModel.get(), true );

    setupProgressDlg( tr( "Finding Results" ), tr( "Cancel" ), count );

    auto rootIdx = fDirModel->index( 0, 0 );
    autoSearch( rootIdx );
    fProgressDlg->setValue( fSearchesCompleted );
}

void CMainWindow::autoSearch( QModelIndex parentIdx )
{
    auto rowCount = fDirModel->rowCount( parentIdx );
    for ( int ii = 0; ii < rowCount; ++ii )
    {
        if ( fProgressDlg->wasCanceled() )
        {
            fSearchTMDB->clearSearchCache();
            break;
        }

        auto childIndex = fDirModel->index( ii, 0, parentIdx );
        auto name = fDirModel->getSearchName( childIndex );
        auto path = fDirModel->filePath( childIndex );
        auto titleInfo = fDirModel->getSearchResultInfo( childIndex );
        auto searchInfo = std::make_shared< NFileRenamerLib::SSearchTMDBInfo >( name, titleInfo );
        searchInfo->setExactMatchOnly( CPreferences::getExactMatchesOnly() );

        if ( fDirModel->shouldAutoSearch( childIndex ) )
        {
            if ( fProgressDlg )
            {
                fProgressDlg->setLabelText( tr( "Adding Background Search for '%1'" ).arg( QDir( fImpl->directory->text() ).relativeFilePath( path ) ) );
                fProgressDlg->setValue( fProgressDlg->value() + 1 );
                qApp->processEvents();
            }
            fSearchTMDB->addSearch( path, searchInfo );
        }

        autoSearch( childIndex );
    }
}

void CMainWindow::slotAutoSearchFinished( const QString & path, bool searchesRemaining )
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

    auto item = fDirModel->getItemFromPath( path );
    if ( item && result )
    {
        fDirModel->setSearchResult( item, result, false );
    }
}

void CMainWindow::clearProgressDlg()
{
    delete fProgressDlg; 
    fProgressDlg = nullptr;
}

void CMainWindow::setupProgressDlg( const QString &title, const QString &cancelButtonText, int max )
{
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
    auto baseIdx = fDirModel->index( idx.row(), NFileRenamerLib::CDirModel::EColumns::eFSName, idx.parent() );
    auto titleInfo = fDirModel->getSearchResultInfo( idx );
    
    auto isDir = baseIdx.data( NFileRenamerLib::CDirModel::ECustomRoles::eIsDir ).toBool();
    auto fullPath = baseIdx.data( NFileRenamerLib::CDirModel::ECustomRoles::eFullPathRole ).toString();
    bool isTVShow = baseIdx.data( NFileRenamerLib::CDirModel::ECustomRoles::eIsTVShowRole ).toBool();
    auto nm = fDirModel->getSearchName( idx );

    CSelectTMDB dlg( nm, titleInfo, this );
    dlg.setSearchForTVShows( fDirModel->treatAsTVShow( QFileInfo( fullPath ), isTVShow ), true );
    dlg.setExactMatchOnly( fImpl->actionExactMatchesOnly->isChecked(), true );

    if ( dlg.exec() == QDialog::Accepted )
    {
        auto titleInfo = dlg.getSearchResult();
        bool setChildren = true;
        if ( titleInfo->isTVShow() && titleInfo->isSeasonOnly() )
            setChildren = false;
        fDirModel->setSearchResult( idx, titleInfo, setChildren );
    }
}

void CMainWindow::slotToggleTreatAsTVShowByDefault()
{
    if ( fDirModel )
        fDirModel->slotTreatAsTVByDefaultChanged( fImpl->actionTreatAsTVShowByDefault->isChecked() );
}

void CMainWindow::slotLoadDirectory()
{
    fDirModel.reset( new NFileRenamerLib::CDirModel );
    fImpl->files->setModel( fDirModel.get() );
    connect( fDirModel.get(), &NFileRenamerLib::CDirModel::sigDirReloaded, this, &CMainWindow::slotAutoSearch );
    fDirModel->slotTreatAsTVByDefaultChanged( fImpl->actionTreatAsTVShowByDefault->isChecked() );
    fDirModel->slotTVOutputFilePatternChanged( CPreferences::getTVOutFilePattern() );
    fDirModel->slotTVOutputDirPatternChanged( CPreferences::getTVOutDirPattern() );
    fDirModel->slotMovieOutputFilePatternChanged( CPreferences::getMovieOutFilePattern() );
    fDirModel->slotMovieOutputDirPatternChanged( CPreferences::getMovieOutDirPattern() );
    fDirModel->setNameFilters( CPreferences::getSearchExtensions(), fImpl->files  );
    setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
    fDirModel->setRootPath( fImpl->directory->text(), fImpl->files, fProgressDlg );
    fImpl->btnTransform->setEnabled( true );
}

void CMainWindow::slotTransform()
{
    auto transformations = fDirModel->transform( true );
    if ( transformations.second->rowCount() == 0 )
    {
        QMessageBox::information( this, tr( "Nothing to change" ), tr( "No files or directories could be transformed" ) );
        return;
    }
    CTransformConfirm dlg( tr( "Transformations:" ), tr( "Proceed?" ), this );
    auto count = NQtUtils::itemCount( transformations.second, true );
    dlg.setModel( transformations.second );
    dlg.setIconLabel( QMessageBox::Information );
    dlg.setButtons( QDialogButtonBox::Yes | QDialogButtonBox::No );
    if ( dlg.exec() == QDialog::Accepted )
    {
        setupProgressDlg( tr( "Renaming Files..." ), tr( "Abort Copy" ), count * fDirModel->eventsPerPath() );
        transformations = fDirModel->transform( false, fProgressDlg );
        if ( !transformations.first )
        {
            CTransformConfirm dlg( tr( "Error While Transforming:" ), tr( "Issues:" ), this );
            dlg.setModel( transformations.second );
            dlg.setIconLabel( QMessageBox::Critical );
            dlg.setButtons( QDialogButtonBox::Ok );
            dlg.exec();
        }
        clearProgressDlg();

        slotLoadDirectory();
    }
}



