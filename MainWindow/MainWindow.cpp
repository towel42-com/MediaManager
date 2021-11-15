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

    fImpl->tvInPattern->setDelay( 1000 );
    fImpl->tvOutFilePattern->setDelay( 1000 );
    fImpl->movieInPattern->setDelay( 1000 );
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
    connect( fImpl->btnSaveM3U, &QPushButton::clicked, this, &CMainWindow::slotSaveM3U );
    connect( fImpl->treatAsMovie, &QCheckBox::clicked, this, &CMainWindow::slotToggleTreatAsMovie );
    connect( fImpl->treatAsMovie, &QCheckBox::pressed, this, &CMainWindow::slotAboutToToggle );

    connect( fImpl->tvInPattern, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotTVInputPatternChanged );
    connect( fImpl->movieInPattern, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotMovieInputPatternChanged );
    connect( fImpl->files, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

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
    

QString CMainWindow::getDefaultOutDirPattern() const
{
    return "<title> (<year>)( [tmdbid=<tmdbid>]):<tmdbid>( - <extra_info>):<extra_info>";
}

QString CMainWindow::getDefaultOutFilePattern( bool forTV ) const
{
    if ( forTV )
        return "<title> - S<season>E<episode>( - <episode_title>):<episode_title>";
    else
        return "<title>";
}

void CMainWindow::loadSettings()
{
    QSettings settings;

    fImpl->directory->setText( settings.value( "Directory", QString() ).toString() );
    fImpl->extensions->setText( settings.value( "Extensions", QString( "*.mkv;*.mp4;*.avi;*.mov;*.wmv;*.mpg;*.mpg2;*.idx;*.sub;*.srt" ) ).toString() );

    fImpl->treatAsMovie->setChecked( settings.value( "TreatAsMovie", true ).toBool() );
    fImpl->exactMatchesOnly->setChecked( settings.value( "ExactMatchesOnly", true ).toBool() );

    loadPatterns();
    slotToggleTreatAsMovie();
}

void CMainWindow::loadPatterns()
{
    QSettings settings;

    settings.beginGroup( "ForTV" );

    auto currText = settings.value( "InPattern", getDefaultInPattern( true ) ).toString();
    fImpl->tvInPattern->setText( currText );

    currText = settings.value( "OutFilePattern", getDefaultOutFilePattern( true ) ).toString();
    fImpl->tvOutFilePattern->setText( currText );

    settings.endGroup();

    settings.beginGroup( "ForMovies" );
    currText = settings.value( "InPattern", getDefaultInPattern( false ) ).toString();
    fImpl->movieInPattern->setText( currText );

    currText = settings.value( "OutFilePattern", getDefaultOutFilePattern( false ) ).toString();
    fImpl->movieOutFilePattern->setText( currText );

    currText = settings.value( "OutDirPattern", getDefaultOutDirPattern() ).toString();
    fImpl->movieOutDirPattern->setText( currText );
    settings.endGroup();
}

void CMainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue( "Directory", fImpl->directory->text() );
    settings.setValue( "Extensions", fImpl->extensions->text() );
    settings.setValue( "TreatAsMovie", fImpl->treatAsMovie->isChecked() );
    settings.setValue( "ExactMatchesOnly", fImpl->exactMatchesOnly->isChecked() );
    

    settings.beginGroup( "ForMovies" );

    settings.setValue( "InPattern", fImpl->movieInPattern->text() );
    settings.setValue( "OutFilePattern", fImpl->movieOutFilePattern->text() );
    settings.setValue( "OutDirPattern", fImpl->movieOutDirPattern->text() );

    settings.endGroup();

    settings.beginGroup( "ForTV" );
    settings.setValue( "InPattern", fImpl->tvInPattern->text() );
    settings.setValue( "OutFilePattern", fImpl->tvOutFilePattern->text() );
}

void CMainWindow::slotDirectoryChanged()
{
    fImpl->btnLoad->setEnabled( false );
    fImpl->btnTransform->setEnabled( false );
    fImpl->btnSaveM3U->setEnabled( false );
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

void CMainWindow::slotAboutToToggle()
{
    saveSettings();
}

bool CMainWindow::isDir( const QModelIndex &idx ) const
{
    return fDirModel->isDir( idx );
}

void CMainWindow::slotAutoSearch()
{
    if ( !fDirModel )
        return;

    if ( fDirModel->rowCount() != 1 )
        return;

    //if ( !fSearchTMDB )
    //{
    //    fSearchTMDB = new CSearchTMDB( nullptr, std::optional<QString>(), this );
    //    connect( fSearchTMDB, &CSearchTMDB::sigSearchFinished, this, &CMainWindow::slotSearchFinished );
    //}

    auto rootIdx = fDirModel->index( 0, 0 );
    for ( int ii = 0; ii < fDirModel->rowCount( rootIdx ); ++ii )
    {
        auto childIndex = fDirModel->index( ii, 0, rootIdx );
        auto name = fDirModel->getSearchName( childIndex );
        auto titleInfo = fDirModel->getTitleInfo( childIndex );
        auto searchInfo = std::make_shared< SSearchTMDBInfo >( name, titleInfo );

        auto persistentModelIndex = QPersistentModelIndex( childIndex );
        //fSearchTMDB->addSearch( searchInfo );
        //fSearchResults[searchInfo] = { persistentModelIndex, {} };
    }
}

void CMainWindow::slotSearchFinished()
{
    //for( auto && ii : fSearchResults )
    //{
    //    auto result = fSearchTMDB->getResult( ii.first, true );
    //    if ( result.has_value() )
    //    {
    //        ii.second.second.insert( ii.second.second.end(), result.value().begin(), result.value().end() );
    //        qDebug() << QDateTime::currentDateTime().toString() << "Found results for" << ii.second.first.data();
    //    }
    //}
}

void CMainWindow::slotDoubleClicked( const QModelIndex &idx )
{
    auto dirModel = dynamic_cast<CDirModel *>( const_cast<QAbstractItemModel *>( idx.model() ) );

    auto baseIdx = dirModel->index( idx.row(), CDirModel::EColumns::eFSName, idx.parent() );
    auto titleInfo = dirModel->getTitleInfo( idx );
    
    auto isDir = baseIdx.data( CDirModel::ECustomRoles::eIsDir ).toBool();
    auto fullPath = baseIdx.data( CDirModel::ECustomRoles::eFullPathRole ).toString();
    auto nm = fDirModel->getSearchName( idx );

    CSelectTMDB dlg( nm, titleInfo, this );
    dlg.setSearchForMovies( fDirModel->treatAsMovie( QFileInfo( fullPath ) ), true );
    dlg.setExactMatchOnly( fImpl->exactMatchesOnly->isChecked(), true );

    if ( dlg.exec() == QDialog::Accepted )
    {
        auto titleInfo = dlg.getTitleInfo();
        dirModel->setTitleInfo( idx, titleInfo, isDir );
    }
}

void CMainWindow::slotToggleTreatAsMovie()
{
    if ( fDirModel )
        fDirModel->slotTreatAsMovieChanged( fImpl->treatAsMovie->isChecked() );
    fImpl->tabWidget->setCurrentWidget( fImpl->treatAsMovie->isChecked() ? fImpl->moviePatternsPage : fImpl->tvPatternsPage );
}

void CMainWindow::slotSaveM3U()
{
    fDirModel->saveM3U( this );
}

void CMainWindow::slotTVInputPatternChanged( const QString& inPattern )
{
    if ( fDirModel )
    {
        fDirModel->slotTVInputPatternChanged( inPattern );
    }
}

void CMainWindow::slotMovieInputPatternChanged( const QString &inPattern )
{
    if ( fDirModel )
    {
        fDirModel->slotMovieInputPatternChanged( inPattern );
    }
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
    connect( fImpl->movieOutFilePattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotMovieOutputFilePatternChanged );
    connect( fImpl->movieOutDirPattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotMovieOutputDirPatternChanged );
    connect( fDirModel.get(), &CDirModel::sigDirReloaded, this, &CMainWindow::slotAutoSearch );
    fDirModel->slotTreatAsMovieChanged( fImpl->treatAsMovie->isChecked() );
    fDirModel->slotTVInputPatternChanged( fImpl->tvInPattern->text() );
    fDirModel->slotTVOutputFilePatternChanged( fImpl->tvOutFilePattern->text() );
    fDirModel->slotMovieInputPatternChanged( fImpl->movieInPattern->text() );
    fDirModel->slotMovieOutputFilePatternChanged( fImpl->movieOutFilePattern->text() );
    fDirModel->slotMovieOutputDirPatternChanged( fImpl->movieOutDirPattern->text() );
    fDirModel->setNameFilters( fImpl->extensions->text().split( ";" ), fImpl->files  );
    fDirModel->setRootPath( fImpl->directory->text(), fImpl->files );
    fImpl->btnTransform->setEnabled( true );
    fImpl->btnSaveM3U->setEnabled( true );
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



