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
#include "DirModel.h"
#include "SelectTMDB.h"
#include "TitleInfo.h"
#include "TransformConfirm.h"
#include "SearchTMDB.h"
#include "SearchTMDBInfo.h"

#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/AutoWaitCursor.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QTimer>

CMainWindow::CMainWindow( QWidget* parent )
    : QMainWindow( parent ),
    fImpl( new Ui::CMainWindow )
{
    fImpl->setupUi( this );

    fImpl->tvOutFilePattern->setDelay( 1000 );
    fImpl->tvOutDirPattern->setDelay( 1000 );
    fImpl->movieOutFilePattern->setDelay( 1000 );
    fImpl->movieOutDirPattern->setDelay( 1000 );
    fImpl->directory->setDelay( 1000 );

    auto completer = new QCompleter( this );
    auto fsModel = new QFileSystemModel( completer );
    fsModel->setRootPath( "/" );
    completer->setModel( fsModel );
    completer->setCompletionMode( QCompleter::PopupCompletion );
    //completer->setCaseSensitivity( Qt::CaseInsensitive );

    fImpl->directory->setCompleter( completer );
    fImpl->files->setExpandsOnDoubleClick( false );

    loadSettings();

    connect( fImpl->directory, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotDirectoryChanged );
    connect( fImpl->btnTransform, &QPushButton::clicked, this, &CMainWindow::slotTransform );

    connect( fImpl->btnSelectDir, &QPushButton::clicked, this, &CMainWindow::slotSelectDirectory );
    connect( fImpl->btnLoad, &QPushButton::clicked, this, &CMainWindow::slotLoad );
    connect( fImpl->treatAsTVShowByDefault, &QCheckBox::clicked, this, &CMainWindow::slotToggleTreatAsTVShowByDefault );

    connect( fImpl->files, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

    fSearchTMDB = new CSearchTMDB( nullptr, std::optional<QString>(), this );
    fSearchTMDB->setSkipImages( true );
    connect( fSearchTMDB, &CSearchTMDB::sigAutoSearchFinished, this, &CMainWindow::slotAutoSearchFinished );

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
    QSettings settings;

    fImpl->directory->setText( settings.value( "Directory", QString() ).toString() );
    fImpl->extensions->setText( settings.value( "Extensions", QString( "*.mkv;*.mp4;*.avi;*.mov;*.wmv;*.mpg;*.mpg2;*.idx;*.sub;*.srt" ) ).toString() );

    fImpl->treatAsTVShowByDefault->setChecked( settings.value( "TreatAsTVShowByDefault", false ).toBool() );
    fImpl->exactMatchesOnly->setChecked( settings.value( "ExactMatchesOnly", true ).toBool() );

    loadPatterns();
    slotToggleTreatAsTVShowByDefault();
}

void CMainWindow::loadPatterns()
{
    QSettings settings;

    settings.beginGroup( "ForTV" );

    auto currText = settings.value( "OutFilePattern", getDefaultOutFilePattern( true ) ).toString();
    fImpl->tvOutFilePattern->setText( currText );
    currText = settings.value( "OutDirPattern", getDefaultOutDirPattern( true ) ).toString();
    fImpl->tvOutDirPattern->setText( currText );

    settings.endGroup();

    settings.beginGroup( "ForMovies" );

    currText = settings.value( "OutFilePattern", getDefaultOutFilePattern( false ) ).toString();
    fImpl->movieOutFilePattern->setText( currText );

    currText = settings.value( "OutDirPattern", getDefaultOutDirPattern( false ) ).toString();
    fImpl->movieOutDirPattern->setText( currText );
    settings.endGroup();
}

void CMainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue( "Directory", fImpl->directory->text() );
    settings.setValue( "Extensions", fImpl->extensions->text() );
    settings.setValue( "TreatAsTVShowByDefault", fImpl->treatAsTVShowByDefault->isChecked() );
    settings.setValue( "ExactMatchesOnly", fImpl->exactMatchesOnly->isChecked() );
    

    settings.beginGroup( "ForMovies" );

    settings.setValue( "OutFilePattern", fImpl->movieOutFilePattern->text() );
    settings.setValue( "OutDirPattern", fImpl->movieOutDirPattern->text() );

    settings.endGroup();

    settings.beginGroup( "ForTV" );
    settings.setValue( "OutFilePattern", fImpl->tvOutFilePattern->text() );
    settings.setValue( "OutDirPattern", fImpl->tvOutDirPattern->text() );
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

void CMainWindow::autoSearch( QModelIndex parentIdx )
{
    auto rowCount = fDirModel->rowCount( parentIdx );
    for ( int ii = 0; ii < rowCount; ++ii )
    {
        auto childIndex = fDirModel->index( ii, 0, parentIdx );
        auto name = fDirModel->getSearchName( childIndex );
        auto path = fDirModel->filePath( childIndex );
        auto titleInfo = fDirModel->getTitleInfo( childIndex );
        auto searchInfo = std::make_shared< SSearchTMDBInfo >( name, titleInfo );
        searchInfo->setExactMatchOnly( true );

        if ( fDirModel->shouldAutoSearch( childIndex ) )
            fSearchTMDB->addSearch( path, searchInfo );

        autoSearch( childIndex );
    }
}

void CMainWindow::slotAutoSearch()
{
    if ( !fDirModel )
        return;

    if ( fDirModel->rowCount() != 1 )
        return;
    auto rootIdx = fDirModel->index( 0, 0 );
    autoSearch( rootIdx );
}

void CMainWindow::slotAutoSearchFinished( const QString & path )
{
    qDebug().noquote().nospace() << "Search results for path " << path;
    auto result = fSearchTMDB->getResult( path );
    auto item = fDirModel->getItemFromPath( path );
    if ( result )
        qDebug() << result->toString();
    if ( item && result )
    {
        fDirModel->setTitleInfo( item, result, false );
    }
}

void CMainWindow::slotDoubleClicked( const QModelIndex &idx )
{
    auto baseIdx = fDirModel->index( idx.row(), CDirModel::EColumns::eFSName, idx.parent() );
    auto titleInfo = fDirModel->getTitleInfo( idx );
    
    auto isDir = baseIdx.data( CDirModel::ECustomRoles::eIsDir ).toBool();
    auto fullPath = baseIdx.data( CDirModel::ECustomRoles::eFullPathRole ).toString();
    bool isTVShow = baseIdx.data( CDirModel::ECustomRoles::eIsTVShowRole ).toBool();
    auto nm = fDirModel->getSearchName( idx );

    CSelectTMDB dlg( nm, titleInfo, this );
    dlg.setSearchForTVShows( fDirModel->treatAsTVShow( QFileInfo( fullPath ), isTVShow ), true );
    dlg.setExactMatchOnly( fImpl->exactMatchesOnly->isChecked(), true );

    if ( dlg.exec() == QDialog::Accepted )
    {
        auto titleInfo = dlg.getTitleInfo();
        bool setChildren = true;
        if ( titleInfo->isTVShow() && titleInfo->isSeasonOnly() )
            setChildren = false;
        fDirModel->setTitleInfo( idx, titleInfo, setChildren );
    }
}

void CMainWindow::slotToggleTreatAsTVShowByDefault()
{
    if ( fDirModel )
        fDirModel->slotTreatAsTVByDefaultChanged( fImpl->treatAsTVShowByDefault->isChecked() );
    fImpl->tabWidget->setCurrentWidget( fImpl->treatAsTVShowByDefault->isChecked() ? fImpl->tvPatternsPage : fImpl->moviePatternsPage );
}

void CMainWindow::slotLoad()
{
    loadDirectory();
}

void CMainWindow::loadDirectory()
{
    CAutoWaitCursor awc;

    fDirModel.reset( new CDirModel );
    fImpl->files->setModel( fDirModel.get() );
    connect( fImpl->tvOutFilePattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotTVOutputFilePatternChanged );
    connect( fImpl->tvOutDirPattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotTVOutputDirPatternChanged );
    connect( fImpl->movieOutFilePattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotMovieOutputFilePatternChanged );
    connect( fImpl->movieOutDirPattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotMovieOutputDirPatternChanged );
    connect( fDirModel.get(), &CDirModel::sigDirReloaded, this, &CMainWindow::slotAutoSearch );
    fDirModel->slotTreatAsTVByDefaultChanged( fImpl->treatAsTVShowByDefault->isChecked() );
    fDirModel->slotTVOutputFilePatternChanged( fImpl->tvOutFilePattern->text() );
    fDirModel->slotTVOutputDirPatternChanged( fImpl->tvOutDirPattern->text() );
    fDirModel->slotMovieOutputFilePatternChanged( fImpl->movieOutFilePattern->text() );
    fDirModel->slotMovieOutputDirPatternChanged( fImpl->movieOutDirPattern->text() );
    fDirModel->setNameFilters( fImpl->extensions->text().split( ";" ), fImpl->files  );
    fDirModel->setRootPath( fImpl->directory->text(), fImpl->files );
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
    dlg.setModel( transformations.second );
    dlg.setIconLabel( QMessageBox::Information );
    dlg.setButtons( QDialogButtonBox::Yes | QDialogButtonBox::No );
    if ( dlg.exec() == QDialog::Accepted )
    {
        transformations = fDirModel->transform( false );
        if ( !transformations.first )
        {
            CTransformConfirm dlg( tr( "Error While Transforming:" ), tr( "Issues:" ), this );
            dlg.setModel( transformations.second );
            dlg.setIconLabel( QMessageBox::Critical );
            dlg.setButtons( QDialogButtonBox::Ok );
            dlg.exec();
        }
        loadDirectory();
    }
}



