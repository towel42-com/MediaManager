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

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/QtUtils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPushButton>

CSelectTMDB::CSelectTMDB( const QString & text, std::shared_ptr< STitleInfo > titleInfo, QWidget* parent )
    : QDialog( parent ),
    fImpl( new Ui::CSelectTMDB )
{
    fImpl->setupUi( this );

    //                                              "\\(?<releaseDate>\\d{2,4})\\)\\s?(\\[tmdbid=(?<tmdbid>\\d+)\\])?"
    QRegularExpression regExp( "(?<searchText>.*)\\s?\\((?<releaseDate>\\d{2,4})\\)\\s?(\\[tmdbid=(?<tmdbid>\\d+)\\])?" );

    QString searchName;
    QString releaseDate;
    QString tmdbid;

    auto match = regExp.match( text );
    if ( match.hasMatch() )
    {
        searchName = match.captured( "searchText" ).trimmed();
        releaseDate = match.captured( "releaseDate" ).trimmed();
        tmdbid = match.captured( "tmdbid" ).trimmed();
    }
    else
        searchName = text;

    if ( titleInfo )
    {
        searchName = titleInfo->fTitle;
        releaseDate = titleInfo->fReleaseDate;
        tmdbid = titleInfo->fTMDBID;
        fImpl->resultExtraInfo->setText( titleInfo->fExtraInfo );
    }

    fImpl->searchText->setText( searchName, false );
    fImpl->releaseYear->setText( releaseDate, false );
    fImpl->tmdbid->setText( tmdbid, false );

    fImpl->searchText->setDelay( 1000 );
    fImpl->releaseYear->setDelay( 1000 );
    fImpl->tmdbid->setDelay( 1000 );

    fImpl->byName->setChecked( tmdbid.isEmpty() );
    fImpl->byTMDBID->setChecked( !tmdbid.isEmpty() );

    reset();

    connect( fImpl->searchText, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->releaseYear, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->tmdbid, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    QTimer::singleShot( 0, this, &CSelectTMDB::slotSearchTextChanged );
    fManager = new QNetworkAccessManager( this );
    connect( fManager, &QNetworkAccessManager::finished, this, &CSelectTMDB::slotRequestFinished );

    connect( fImpl->results->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CSelectTMDB::slotSelectionChanged );
}

void CSelectTMDB::reset()
{
    for( auto && ii : fImageInfoReplies )
    {
        ii.second = nullptr;
    }
    fImageInfoReplies.clear();
    fImpl->results->clear();
    fImpl->results->setHeaderLabels( QStringList() << "Title" << "TMDB ID" << "Release Date" << "Desc" );
}

CSelectTMDB::~CSelectTMDB()
{
}

const QString apiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
const QString apiKeyV4 = "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";

void CSelectTMDB::slotRequestFinished( QNetworkReply *reply )
{
    if ( reply->error() != QNetworkReply::NoError )
    {
        auto msg = reply->errorString();
        return;
    }
    if ( reply == fConfigReply )
    {
        loadConfig();
        return;
    }

    if ( reply == fSearchReply )
    {
        loadSearchResult();
        return;
    }

    if ( reply == fGetMovieReply )
    {
        loadMovieResult();
        return;
    }
    
    loadImageResults( reply );
}

void CSelectTMDB::slotGetConfig()
{
    QUrl url;
    url.setScheme( "https" );
    url.setHost( "api.themoviedb.org" );
    url.setPath( "/3/configuration" );

    QUrlQuery query;
    query.addQueryItem( "api_key", apiKeyV3 );
    url.setQuery( query );
    fConfigReply = fManager->get( QNetworkRequest( url ) );
}

bool CSelectTMDB::hasConfiguration() const
{
    return fConfiguration.has_value() && !fConfiguration.value().isEmpty();
}

void CSelectTMDB::slotSearchTextChanged()
{
    if ( !hasConfiguration() )
    {
        QTimer::singleShot( 0, this, &CSelectTMDB::slotGetConfig );
        return;
    }
    QUrl url;
    url.setScheme( "https" );
    url.setHost( "api.themoviedb.org" );
    if ( fImpl->byName->isChecked() )
    {
        url.setPath( "/3/search/movie" );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        query.addQueryItem( "include_adult", "true" );
        if ( !fImpl->releaseYear->text().isEmpty() )
            query.addQueryItem( "year", fImpl->releaseYear->text() );
        auto searchStrings =
            fImpl->searchText->text().split( " ", Qt::SplitBehaviorFlags::SkipEmptyParts );

        query.addQueryItem( "query", searchStrings.join( "+" ) );
        url.setQuery( query );

        //qDebug() << url.toString();
        fSearchReply = fManager->get( QNetworkRequest( url ) );
    }
    else // by tmdbid
    {
        url.setPath( QString( "/3/movie/%1" ).arg( fImpl->tmdbid->text() ) );
        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        url.setQuery( query );
        qDebug() << url.toString();
        fGetMovieReply = fManager->get( QNetworkRequest( url ) );
    }
}

void CSelectTMDB::loadConfig()
{
    if ( !fConfigReply )
        return;

    auto data = fConfigReply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );
    if ( !doc.object().contains( "images" ) )
        return;
    auto images = doc.object()["images"].toObject();
    if ( !images.contains( "poster_sizes" ) || !images[ "poster_sizes" ].isArray() )
        return;
    auto posterSizes = images["poster_sizes"].toArray();

    QString posterSize = "original";
    for( int ii = 0; ii < posterSizes.size(); ++ii )
    {
        auto curr = posterSizes[ii].toString();
        //if ( curr == "w500" )
        //{
        //    posterSize = curr;
        //    break;
        //}
    }
    auto posterURL = images.contains( "secure_base_url" ) ? images["secure_base_url"].toString() : QString();
    if ( !posterURL.isEmpty() )
        fConfiguration = posterURL + posterSize;
    QTimer::singleShot( 0, this, &CSelectTMDB::slotSearchTextChanged );
    fConfigReply = nullptr;
}


void CSelectTMDB::loadMovieResult()
{
    if ( !fGetMovieReply )
        return;

    reset();

    auto data = fGetMovieReply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );
    loadSearchResult( doc.object() );

    delete fButtonEnabler;
    fButtonEnabler = new CButtonEnabler( fImpl->results, fImpl->buttonBox->button( QDialogButtonBox::Ok ) );
}

void CSelectTMDB::loadSearchResult()
{
    if ( !fSearchReply )
        return;

    reset();

    auto data = fSearchReply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    if ( doc.object().contains( "results" ) )
    {
        auto results = doc.object()["results"].toArray();
        for ( int ii = 0; ii < results.size(); ++ii )
        {
            loadSearchResult( results[ii].toObject() );
        }
    }

    delete fButtonEnabler;
    fButtonEnabler = new CButtonEnabler( fImpl->results, fImpl->buttonBox->button( QDialogButtonBox::Ok ) );
}


/*
 *{
    "adult": false,
    "backdrop_path": "/aknvFyJUQQoZFtmFnYzKi4vGv4J.jpg",
    "genre_ids": [
        28,
        12,
        18,
        878
    ],
    "id": 438631,
    "original_language": "en",
    "original_title": "Dune",
    "overview": "Paul Atreides, a brilliant and gifted young man born into a great destiny beyond his understanding, must travel to the most dangerous planet in the universe to ensure the future of his family and his people. As malevolent forces explode into conflict over the planet's exclusive supply of the most precious resource in existence-a commodity capable of unlocking humanity's greatest potential-only those who can conquer their fear will survive.",
    "popularity": 723.694,
    "poster_path": "/lr3cYNDlJcpT1EWzFH42aSIvkab.jpg",
    "release_date": "2021-09-15",
    "title": "Dune",
    "video": false,
    "vote_average": 8.1,
    "vote_count": 1591
}
*/
void CSelectTMDB::loadSearchResult( const QJsonObject &resultItem )
{
    qDebug().nospace().noquote() << QJsonDocument( resultItem ).toJson( QJsonDocument::Indented );

    auto tmdbid = resultItem.contains( "id" ) ? resultItem["id"].toInt() : -1;
    auto desc = resultItem.contains( "overview" ) ? resultItem["overview"].toString() : QString();
    auto title = resultItem.contains( "title" ) ? resultItem["title"].toString() : QString();
    auto releaseDate = resultItem.contains( "release_date" ) ? resultItem["release_date"].toString() : QString();
    auto posterPath = resultItem.contains( "poster_path" ) ? resultItem["poster_path"].toString() : QString();

    bool aOK;
    int releaseYear = fImpl->releaseYear->text().toInt( &aOK );
    if ( aOK && !fImpl->releaseYear->text().isEmpty() && !releaseDate.isEmpty() )
    {
        auto dt = NQtUtils::findDate( releaseDate );

        if ( dt.isValid() && dt.year() != releaseYear )
        {
            return;
        }
    }

    int searchTmdbid = fImpl->tmdbid->text().toInt( &aOK );
    if ( aOK && !fImpl->tmdbid->text().isEmpty() && ( tmdbid != -1 ) )
    {
        if ( tmdbid != searchTmdbid )
            return;
    }
    auto label = new QLabel( desc, this );
    label->setWordWrap( true );
    auto item = new QTreeWidgetItem( fImpl->results, QStringList() << title << QString::number( tmdbid ) << releaseDate << QString() );
    fImpl->results->setItemWidget( item, 3, label );
    fImpl->results->resizeColumnToContents( 0 );

    if ( !posterPath.isEmpty() && hasConfiguration() )
    {
        auto path = fConfiguration.value() + posterPath;
        QUrl url( path );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );
        url.setQuery( query );
        qDebug() << url.toString();

        auto reply = fManager->get( QNetworkRequest( url ) );
        fImageInfoReplies[reply] = item;
    }
}

void CSelectTMDB::loadImageResults( QNetworkReply * reply )
{
    auto pos = fImageInfoReplies.find( reply );
    if ( pos == fImageInfoReplies.end() )
        return;
    auto item = ( *pos ).second;
    if ( !item )
        return;

    QImage img;
    img.loadFromData( reply->readAll() );
    QPixmap pm = QPixmap::fromImage( img );
    qDebug() << pm.size();

    QIcon icn( pm );
    item->setIcon( 0, icn );
    fImpl->results->setIconSize( QSize( 128, 128 ) );
    fImpl->results->resizeColumnToContents( 0 );
}

std::shared_ptr< STitleInfo > CSelectTMDB::getTitleInfo() const
{
    auto retVal = std::make_shared< STitleInfo >();
    retVal->fTitle = fImpl->resultTitle->text();
    retVal->fReleaseDate = fImpl->resultReleaseDate->text();
    retVal->fTMDBID = fImpl->resultTMDBID->text();
    retVal->fExtraInfo = fImpl->resultExtraInfo->text();
    retVal->fIsMovie = true;

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
    fImpl->resultReleaseDate->setText( first->text( 2 ) );
    fImpl->resultTMDBID->setText( first->text( 1 ) );
}