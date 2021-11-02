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

#include "SelectTMDB.h"
#include "DirModel.h"
#include "ui_SelectTMDB.h"
#include "TitleInfo.h"
#include "SearchTMDB.h"

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/QtUtils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QPushButton>
#include <QMessageBox>

CSelectTMDB::CSelectTMDB( const QString & text, std::shared_ptr< STitleInfo > titleInfo, QWidget* parent )
    : QDialog( parent ),
    fImpl( new Ui::CSelectTMDB )
{
    fImpl->setupUi( this );

    fSearchInfo = std::make_shared< SSearchTMDBInfo >( text, titleInfo );
    fSearchTMDB = new CSearchTMDB( fSearchInfo, std::optional<QString>(), this );

    connect( this, &CSelectTMDB::sigStartSearch, fSearchTMDB, &CSearchTMDB::slotSearch );
    connect( fSearchTMDB, &CSearchTMDB::sigSearchFinished, this, &CSelectTMDB::slotSearchFinished );

    fImpl->resultExtraInfo->setText( titleInfo ? titleInfo->fExtraInfo : QString() );

    fImpl->resultEpisodeTitle->setText( fSearchInfo->fEpisodeTitle );

    fImpl->searchName->setDelay( 1000 );
    //fImpl->searchSeason->setDelay( 1000 );
    //fImpl->searchEpisode->setDelay( 1000 );
    fImpl->searchReleaseYear->setDelay( 1000 );
    fImpl->searchTMDBID->setDelay( 1000 );

    updateFromSearchInfo( fSearchInfo );
    slotReset();

    connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
    connect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForMovieChanged );
    connect( fImpl->searchForMovies, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForMovieChanged );

    connect( fImpl->searchName, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchSeason, &CDelaySpinBox::sigValueChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchEpisode, &CDelaySpinBox::sigValueChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchReleaseYear, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchTMDBID, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->results->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CSelectTMDB::slotSelectionChanged );

    fImpl->results->setIconSize( QSize( 128, 128 ) );

    updateByName( true );

    QTimer::singleShot( 0, this, &CSelectTMDB::slotSearchTextChanged );
}

void CSelectTMDB::updateFromSearchInfo( std::shared_ptr< SSearchTMDBInfo > searchInfo )
{
    disconnect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
    disconnect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForMovieChanged );
    disconnect( fImpl->searchForMovies, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForMovieChanged );

    disconnect( fImpl->searchName, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    disconnect( fImpl->searchSeason, &CDelaySpinBox::sigValueChanged, this, &CSelectTMDB::slotSearchTextChanged );
    disconnect( fImpl->searchEpisode, &CDelaySpinBox::sigValueChanged, this, &CSelectTMDB::slotSearchTextChanged );
    disconnect( fImpl->searchReleaseYear, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    disconnect( fImpl->searchTMDBID, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );

    fImpl->searchName->setText( searchInfo->fSearchName );
    fImpl->searchSeason->setValue( searchInfo->fSeason );
    fImpl->searchEpisode->setValue( searchInfo->fEpisode );
    fImpl->searchReleaseYear->setText( searchInfo->fReleaseDate );
    fImpl->searchTMDBID->setText( searchInfo->fTMDBID );
    fImpl->byName->setChecked( fSearchTMDB->searchByName() );
    fImpl->byTMDBID->setChecked( !fSearchTMDB->searchByName() );
    fImpl->exactMatchesOnly->setChecked( searchInfo->fExactMatchOnly );
    fImpl->searchForMovies->setChecked( searchInfo->fIsMovie );

    connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
    connect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForMovieChanged );
    connect( fImpl->searchForMovies, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForMovieChanged );

    connect( fImpl->searchName, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchSeason, &CDelaySpinBox::sigValueChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchEpisode, &CDelaySpinBox::sigValueChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchReleaseYear, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchTMDBID, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
}

void CSelectTMDB::slotReset()
{
    if ( fLoading )
    {
        fStopLoading = true;
        fSearchTMDB->resetResults();
        QTimer::singleShot( 500, this, &CSelectTMDB::slotReset );
        return;
    }
    fStopLoading = false;
    fSearchTMDB->resetResults();
    fImpl->results->clear();
    resetHeader();
}

void CSelectTMDB::resetHeader()
{
    fImpl->results->setColumnCount( 0 );
    if ( fImpl->searchForMovies->isChecked() )
        fImpl->results->setHeaderLabels( QStringList() << "Title" << "TMDB ID" << "Release Date" << "Desc" );
    else
        fImpl->results->setHeaderLabels( QStringList() << "Title" << "TMDB ID" << "Season" << "Episode" << "Air Date" << "Episode Title" << "Desc" );
}

CSelectTMDB::~CSelectTMDB()
{
}

const QString apiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
const QString apiKeyV4 = "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";

void CSelectTMDB::deleteParent( QTreeWidgetItem *item )
{
    if ( !item )
        return;
    auto parent = item->parent();
    if ( item->type() == EItemType::eTVShow )
    {
        delete item;
        deleteParent( parent );
    }
    else if ( item->type() == EItemType::eSeason )
    {
        if ( item->childCount() == 0 )
        {
            delete item;
            deleteParent( parent );
        }
    }
    else if ( item->type() == EItemType::eTVShow )
    {
        if ( item->childCount() == 0 )
        {
            delete item;
        }
    }
}

void CSelectTMDB::slotSearchFinished()
{
    if ( fSearchTMDB->hasError() )
    {
        QMessageBox::information( this, tr( "Error searching themoviedb" ), fSearchTMDB->errorString() );
        return;
    }

    auto currResults = fSearchTMDB->getResults();
    for ( auto &&ii : currResults )
        fCurrentResults.push_back( ii );
    fBestMatch = fSearchTMDB->bestMatch();

    fLoading = true;
    qDebug() << QDateTime::currentDateTime().toString() << "launching slotLoadNextResult" << fCurrentResults.size();
    QTimer::singleShot( 0, this, &CSelectTMDB::slotLoadNextResult );
}

void CSelectTMDB::slotLoadNextResult()
{
    qDebug() << QDateTime::currentDateTime().toString() << "handling slotLoadNextResult" << fCurrentResults.size();
    if ( fStopLoading )
        fCurrentResults.clear();

    if ( fCurrentResults.empty() )
    {
        delete fButtonEnabler;
        fButtonEnabler = new CButtonEnabler( fImpl->results, fImpl->buttonBox->button( QDialogButtonBox::Ok ) );

        fLoading = false;
        fSearchTMDB->resetResults();

        if ( fQueuedSearchInfo )
        {
            fSearchInfo = fQueuedSearchInfo;
            fQueuedSearchInfo.reset();
            QTimer::singleShot( 500, this, &CSelectTMDB::slotSearchTextChanged );
        }
        return;
    }

    auto curr = fCurrentResults.front();
    fCurrentResults.pop_front();
    loadResults( curr, nullptr );

    qDebug() << QDateTime::currentDateTime().toString() << "Launching slotLoadNextResult" << fCurrentResults.size();
    QTimer::singleShot( 0, this, &CSelectTMDB::slotLoadNextResult );
}

void CSelectTMDB::loadResults( std::shared_ptr< STitleInfo > info, QTreeWidgetItem *parent )
{
    if ( fStopLoading )
        return;

    qDebug() << QDateTime::currentDateTime().toString() << "loading result" << fCurrentResults.size();
    QStringList data;
    EItemType itemType;
    int labelPos = -1;
    if ( fImpl->searchForMovies->isChecked() )
    {
        data = QStringList() << info->fTitle << info->fTMDBID << info->fReleaseDate << QString();
        itemType = EItemType::eMovie;
        labelPos = 3;
    }
    else
    {
        data = QStringList() << info->fTitle << info->fTMDBID << info->fSeason << info->fEpisode << info->fReleaseDate << info->fEpisodeTitle << QString();
        itemType = EItemType::eTVShow;
        labelPos = 6;
    }

    QTreeWidgetItem *item;
    if ( parent )
        item = new QTreeWidgetItem( parent, data, EItemType::eMovie );
    else
        item = new QTreeWidgetItem( fImpl->results, data, itemType );

    auto label = new QLabel( info->fDescription, this );
    label->setWordWrap( true );
    fImpl->results->setItemWidget( item, labelPos, label );
    item->setExpanded( true );

    if ( !info->fPixmap.isNull() )
    {
        item->setIcon( 0, QIcon( info->fPixmap ) );
    }

    fImpl->results->resizeColumnToContents( 0 );
    if ( fBestMatch && ( info.get() == fBestMatch.get() ) )
        item->setSelected( true );
    qApp->processEvents();

    for( auto && ii : info->fChildren )
    {
        loadResults( ii, item );
        if ( fStopLoading )
            break;
    }
}


std::shared_ptr< STitleInfo > CSelectTMDB::getTitleInfo() const
{
    auto retVal = std::make_shared< STitleInfo >();
    retVal->fTitle = fImpl->resultTitle->text();
    retVal->fTMDBID = fImpl->resultTMDBID->text();
    retVal->fReleaseDate = fImpl->resultReleaseDate->text();

    retVal->fSeason = fImpl->resultSeason->text();
    retVal->fEpisode = fImpl->resultEpisode->text();
    retVal->fEpisodeTitle = fImpl->resultEpisodeTitle->text();
    retVal->fExtraInfo = fImpl->resultExtraInfo->text();

    retVal->fIsMovie = fImpl->searchForMovies->isChecked();

    return retVal;
}


void CSelectTMDB::slotSelectionChanged()
{
    auto selected = fImpl->results->selectedItems();
    if ( selected.empty() )
        return;

    auto first = selected.front();
    if ( !first )
        return;

    fImpl->resultTitle->setText( first->text( 0 ) );
    fImpl->resultTMDBID->setText( first->text( 1 ) );
    if ( fImpl->searchForMovies->isChecked() )
    {
        fImpl->resultReleaseDate->setText( first->text( 2 ) );
    }
    else
    {
        fImpl->resultSeason->setText( first->text( 2 ) );
        fImpl->resultEpisode->setText( first->text( 3 ) );
        fImpl->resultReleaseDate->setText( first->text( 4 ) );
        fImpl->resultEpisodeTitle->setText( first->text( 5 ) );
    }
}

std::shared_ptr< SSearchTMDBInfo > CSelectTMDB::getSearchInfo()
{
    std::shared_ptr< SSearchTMDBInfo > searchInfo;
    if ( fSearchTMDB->isActive() )
    {
        if ( !fQueuedSearchInfo )
            fQueuedSearchInfo = std::make_shared< SSearchTMDBInfo >( *fSearchInfo );
        searchInfo = fQueuedSearchInfo;
    }
    else
        searchInfo = fSearchInfo;
    return searchInfo;
}

void CSelectTMDB::setSearchForMovies( bool value, bool init )
{
    auto searchInfo = getSearchInfo();

    searchInfo->fIsMovie = value;
    searchInfo->updateSearchCriteria( init );
    if ( init && !fSearchPending )
    {
        updateFromSearchInfo( searchInfo );
    }
}

void CSelectTMDB::setExactMatchOnly( bool value, bool /*init*/ )
{
    auto searchInfo = getSearchInfo();
    searchInfo->fExactMatchOnly = value;
}

void CSelectTMDB::slotByNameChanged()
{
    updateByName( false );
}

void CSelectTMDB::updateByName( bool init )
{
    fImpl->searchName->setEnabled( !fImpl->byTMDBID->isChecked() );
    fImpl->searchReleaseYear->setEnabled( !fImpl->byTMDBID->isChecked() );
    fImpl->searchSeason->setEnabled( !fImpl->searchForMovies->isChecked() && !fImpl->byTMDBID->isChecked() );
    fImpl->searchEpisode->setEnabled( !fImpl->searchForMovies->isChecked() && !fImpl->byTMDBID->isChecked() );
    fImpl->searchTMDBID->setEnabled( true );
    if ( !init )
        slotSearchTextChanged();
}

void CSelectTMDB::slotExactOrForMovieChanged()
{
    auto searchInfo = getSearchInfo();

    searchInfo->fExactMatchOnly = fImpl->exactMatchesOnly->isChecked();
    searchInfo->fIsMovie = fImpl->searchForMovies->isChecked();
    fImpl->searchSeason->setEnabled( !fImpl->searchForMovies->isChecked() && !fImpl->byTMDBID->isChecked() );
    fImpl->searchEpisode->setEnabled( !fImpl->searchForMovies->isChecked() && !fImpl->byTMDBID->isChecked() );
    fImpl->resultSeason->setEnabled( !fImpl->searchForMovies->isChecked() );
    fImpl->resultEpisode->setEnabled( !fImpl->searchForMovies->isChecked() );
    fImpl->resultEpisodeTitle->setEnabled( !fImpl->searchForMovies->isChecked() );
    searchInfo->updateSearchCriteria( false );
    updateFromSearchInfo( fSearchInfo );
    slotSearchTextChanged();
}

void CSelectTMDB::slotSearchTextChanged()
{
    if ( fSearchTMDB->isActive() )
    {
        QTimer::singleShot( 500, this, &CSelectTMDB::slotSearchTextChanged );
        return;
    }

    auto searchInfo = getSearchInfo();
    if ( fImpl->searchTMDBID->text().isEmpty() && !fImpl->byName->isChecked() )
    {
        disconnect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
        fImpl->byName->setChecked( true );
        connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
    }
    searchInfo->fSearchByName = fImpl->byName->isChecked();
    searchInfo->fReleaseDate = fImpl->searchReleaseYear->text();
    searchInfo->fSearchName = fImpl->searchName->text();
    searchInfo->fTMDBID = fImpl->searchTMDBID->text();

    slotReset();

    emit sigStartSearch();
}

