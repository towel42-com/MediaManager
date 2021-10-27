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

#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "ui_MainWindow.h"

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMediaPlaylist>
#include <QMessageBox>

CMainWindow::CMainWindow( QWidget* parent )
    : QMainWindow( parent ),
    fImpl( new Ui::CMainWindow )
{
    fImpl->setupUi( this );
    connect( fImpl->directory, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotDirectoryChanged );

    connect( fImpl->btnSelectDir, &QPushButton::clicked, this, &CMainWindow::slotSelectDirectory );
    connect( fImpl->btnLoad, &QPushButton::clicked, this, &CMainWindow::slotLoad );
    connect( fImpl->btnSaveM3U, &QPushButton::clicked, this, &CMainWindow::slotSaveM3U );
    connect( fImpl->treatAsMovie, &QCheckBox::clicked, this, &CMainWindow::slotToggleTreatAsMovie );
    connect( fImpl->treatAsMovie, &QCheckBox::pressed, this, &CMainWindow::slotAboutToToggle );

    connect( fImpl->inPattern, &CDelayLineEdit::sigTextChanged, this, &CMainWindow::slotInputPatternChanged );
    connect( fImpl->files, &QTreeView::doubleClicked, this, &CMainWindow::slotDoubleClicked );

    fImpl->inPattern->setDelay( 1000 );
    fImpl->directory->setDelay( 1000 );
    fImpl->outFilePattern->setDelay( 1000 );
    fImpl->outDirPattern->setDelay( 1000 );

    auto completer = new QCompleter( this );
    auto fsModel = new QFileSystemModel( completer );
    fsModel->setRootPath( "" );
    completer->setModel( fsModel );
    completer->setCompletionMode( QCompleter::PopupCompletion );
    completer->setCaseSensitivity( Qt::CaseInsensitive );

    fImpl->directory->setCompleter( completer );
    fImpl->files->setExpandsOnDoubleClick( false );

    loadSettings();
    slotDirectoryChanged();
}

CMainWindow::~CMainWindow()
{
    saveSettings();
}

QString CMainWindow::getDefaultInPattern( bool forMovies ) const
{
    if ( forMovies )
        return "(?<title>.+)\\.(?<year>\\d{2,4})\\..*";
    else
        return "(?<title>.+)\\.([Ss](?<season>\\d+))([Ee](?<episode>\\d+))(\\.(?<episode_title>.*))?\\.1080.*";
}
    

QString CMainWindow::getDefaultOutDirPattern( bool forMovies ) const
{
    if ( forMovies )
        return "<title> (<year>)( [tmdbid=<tmdbid>]):<tmdbid>( - <extra_info>):<extra_info>";
    else
        return "";
}

QString CMainWindow::getDefaultOutFilePattern( bool forMovies ) const
{
    if ( forMovies )
        return "<title>";
    else
        return "<title> - S<season>E<episode>( - <episode_title>):<episode_title>";
}

void CMainWindow::loadSettings()
{
    QSettings settings;

    fImpl->directory->setText( settings.value( "Directory", QString() ).toString() );
    fImpl->extensions->setText( settings.value( "Extensions", QString( "*.mkv;*.mp4;*.avi;*.idx;*.sub;*.srt" ) ).toString() );

    fImpl->treatAsMovie->setChecked( settings.value( "TreatAsMovie", true ).toBool() );

    loadPatterns();
}

void CMainWindow::loadPatterns()
{
    QSettings settings;
    fImpl->outDirPattern->setEnabled( fImpl->treatAsMovie->isChecked() );

    if ( fImpl->treatAsMovie->isChecked() )
        settings.beginGroup( "ForMovies" );
    else
        settings.beginGroup( "ForTV" );

    auto currText = settings.value( "InPattern", getDefaultInPattern( fImpl->treatAsMovie->isChecked() ) ).toString();
    fImpl->inPattern->setText( currText );

    currText = settings.value( "OutFilePattern", getDefaultOutFilePattern( fImpl->treatAsMovie->isChecked() ) ).toString();
    fImpl->outFilePattern->setText( currText );

    currText = settings.value( "OutDirPattern", getDefaultOutDirPattern( fImpl->treatAsMovie->isChecked() ) ).toString();
    fImpl->outDirPattern->setText( currText );
}

void CMainWindow::saveSettings()
{
    QSettings settings;

    settings.setValue( "Directory", fImpl->directory->text() );
    settings.setValue( "Extensions", fImpl->extensions->text() );
    settings.setValue( "TreatAsMovie", fImpl->treatAsMovie->isChecked() );

    if ( fImpl->treatAsMovie->isChecked() )
        settings.beginGroup( "ForMovies" );
    else
        settings.beginGroup( "ForTV" );

    settings.setValue( "InPattern", fImpl->inPattern->text() );
    settings.setValue( "OutFilePattern", fImpl->outFilePattern->text() );
    settings.setValue( "OutDirPattern", fImpl->outDirPattern->text() );
}

void CMainWindow::slotDirectoryChanged()
{
    auto dirName = fImpl->directory->text();
    QFileInfo fi( dirName );
    fImpl->btnLoad->setEnabled( !dirName.isEmpty() && fi.exists() && fi.isDir() );
    fImpl->btnTransform->setEnabled( false );
    fImpl->btnSaveM3U->setEnabled( false );
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
    if ( idx.model() == fDirModel.get() )
        return fDirModel->isDir( idx );
    else if ( idx.model() == fDirFilterModel.get() )
    {
        return isDir( dynamic_cast<const CDirFilterModel *>( idx.model() )->mapToSource( idx ) );
    }
    return false;
    
}
void CMainWindow::slotDoubleClicked( const QModelIndex &idx )
{
    if ( !isDir( idx ) )
        return;
    QModelIndex sourceIdx = idx;
    auto model = idx.model();
    if ( sourceIdx.model() == fDirFilterModel.get() )
    {
        sourceIdx = dynamic_cast<const CDirFilterModel *>( model )->mapToSource( idx );
        model = dynamic_cast<const CDirFilterModel *>( model )->sourceModel();
    }

    auto dirModel = dynamic_cast<CDirModel *>( const_cast<QAbstractItemModel *>( model ) );

    auto titleInfo = dirModel->getTitleInfo( sourceIdx );
    
    auto nm = dirModel->index( sourceIdx.row(), 4, idx.parent() ).data().toString();
    if ( nm == "<NOMATCH>" )
    {
        nm = dirModel->index( sourceIdx.row(), 0, idx.parent() ).data().toString();
    }
    CSelectTMDB dlg( nm, titleInfo, this );
    if ( dlg.exec() == QDialog::Accepted )
    {
        auto titleInfo = dlg.getTitleInfo();
        if ( titleInfo->getTitle().isEmpty() )
            return;

        dirModel->setTitleInfo( sourceIdx, titleInfo );
    }
}
void CMainWindow::slotToggleTreatAsMovie()
{
    loadPatterns();
}

void CMainWindow::slotSaveM3U()
{
    fDirModel->saveM3U( this );
}

void CMainWindow::slotInputPatternChanged( const QString& inPattern )
{
    if ( fDirModel )
        fDirModel->slotInputPatternChanged( inPattern );
}

void CMainWindow::slotLoad()
{
    //QRegularExpression regExp(inPattern);
    //if (!regExp.isValid())
    //{
    //	QMessageBox::critical(dynamic_cast<QWidget*>(parent()), tr("Invalid RegEx"), tr("Invalid regular expression: '%1").arg(inPattern));
    //	return;
    //}
    loadDirectory();
}

void CMainWindow::loadDirectory()
{
    fDirModel.reset( new CDirModel );
    fDirFilterModel.reset( new CDirFilterModel );

    fDirFilterModel->setSourceModel( fDirModel.get() );
    fImpl->files->setModel( fDirModel.get() );
    fDirModel->setReadOnly( true );
    fDirModel->setFilter( QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot );
    fDirModel->setNameFilterDisables( false );
    connect( fDirModel.get(), &QFileSystemModel::directoryLoaded, this, &CMainWindow::slotDirLoaded );
    connect( fImpl->outFilePattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotOutputFilePatternChanged );
    connect( fImpl->outDirPattern, &CDelayLineEdit::sigTextChanged, fDirModel.get(), &CDirModel::slotOutputDirPatternChanged );
    connect( fImpl->btnTransform, &QPushButton::clicked, this, &CMainWindow::slotTransform );

    fDirModel->slotInputPatternChanged( fImpl->inPattern->text() );
    fDirModel->slotOutputFilePatternChanged( fImpl->outFilePattern->text() );
    fDirModel->slotOutputDirPatternChanged( fImpl->outDirPattern->text() );
    fDirModel->slotTreatAsMovieChanged( fImpl->treatAsMovie->isChecked() );
    fDirModel->setNameFilters( fImpl->extensions->text().split( ";" ) );
    fDirModel->setRootPath( fImpl->directory->text() );
    fImpl->files->setRootIndex( fDirModel->index( fImpl->directory->text() ) );
    fImpl->btnTransform->setEnabled( true );
    fImpl->btnSaveM3U->setEnabled( true );
}

void CMainWindow::slotDirLoaded( const QString& dirName )
{
    auto idx = fDirModel->index( dirName );
    Q_ASSERT( idx.isValid() );
    auto numRows = fDirModel->rowCount( idx );
    for ( int ii = 0; ii < numRows; ++ii )
    {
        auto childIndex = fDirModel->index( ii, 0, idx );
        if ( childIndex.isValid() && fDirModel->isDir( childIndex ) )
        {
            fImpl->files->setExpanded( childIndex, true );
        }
    }
}

void CMainWindow::slotTransform()
{
    auto transformations = fDirModel->transform( true );
    CScrollMessageBox dlg( tr( "Transformations:" ), tr( "Proceed?" ), this );
    dlg.setPlainText( transformations.second.join( "\n" ) );
    dlg.setIconLabel( QMessageBox::Information );
    dlg.setButtons( QDialogButtonBox::Yes | QDialogButtonBox::No );
    if ( dlg.exec() == QDialog::Accepted )
    {
        transformations = fDirModel->transform( false );
        if ( !transformations.first )
        {
            CScrollMessageBox dlg( tr( "Error While Transforming:" ), tr( "Issues:" ), this );
            dlg.setPlainText( transformations.second.join( "\n" ) );
            dlg.setIconLabel( QMessageBox::Critical );
            dlg.setButtons( QDialogButtonBox::Ok );
            dlg.exec();
        }
    }
    loadDirectory();
}



