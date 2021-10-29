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
#include <QMessageBox>

CSelectTMDB::CSelectTMDB( const QString & text, std::shared_ptr< STitleInfo > titleInfo, bool isMovie, QWidget* parent )
    : QDialog( parent ),
    fImpl( new Ui::CSelectTMDB ),
    fIsMovie( isMovie )
{
    fImpl->setupUi( this );

    fImpl->releaseYear->setEnabled( isMovie );
    fImpl->searchSeason->setEnabled( !isMovie );
    fImpl->searchEpisode->setEnabled( !isMovie );
    fImpl->resultSeason->setEnabled( !isMovie );
    fImpl->resultEpisode->setEnabled( !isMovie );
    fImpl->resultEpisodeTitle->setEnabled( !isMovie );


    QString searchName = text;
    QString season;
    QString episode;
    QString episodeTitle;
    QString releaseDate;
    QString tmdbid;

    if ( isMovie )
    {
        auto regExpText = QString( "(?<searchName>.*)\\s?\\((?<releaseDate>\\d{2,4})\\)\\s?(\\[tmdbid=(?<tmdbid>\\d+)\\])?" );
        QRegularExpression regExp( regExpText );

        auto match = regExp.match( text );
        if ( match.hasMatch() )
        {
            searchName = match.captured( "searchName" ).trimmed();
            releaseDate = match.captured( "releaseDate" ).trimmed();
            tmdbid = match.captured( "tmdbid" ).trimmed();
        }
    }
    else
    {
        auto regExp = QRegularExpression( "S(?<season>\\d+)" );
        auto match = regExp.match( searchName );
        if ( match.hasMatch() )
        {
            season = match.captured( "season" ).trimmed();
            searchName.replace( regExp, "" );
        }

        regExp = QRegularExpression( "E(?<episode>\\d+)" );
        match = regExp.match( searchName );
        if ( match.hasMatch() )
        {
            episode = match.captured( "episode" ).trimmed();
            searchName.replace( regExp, "" );
        }

        auto pos = searchName.indexOf( "-" );
        if ( pos != -1 )
        {
            episodeTitle = searchName.mid( pos + 1 ).trimmed();
            if ( episodeTitle.startsWith( '-' ) )
                episodeTitle = episodeTitle.mid( 1 ).trimmed();
            searchName = searchName.left( pos ).trimmed();
        }
        searchName = searchName.trimmed();
        if ( searchName.endsWith( "-" ) )
        {
            searchName = searchName.mid( 0, searchName.length() - 1 ).trimmed();
        }

        if ( ( searchName == "Star Wars" ) && ( text.startsWith( "Star Wars - Rebels" ) ) )
             searchName = "Star Wars Rebels";
    }

    if ( titleInfo )
    {
        searchName = titleInfo->fTitle;
        episode = titleInfo->fEpisode;
        season = titleInfo->fSeason;
        releaseDate = titleInfo->fReleaseDate;
        tmdbid = titleInfo->fTMDBID;
        episodeTitle = titleInfo->fEpisodeTitle;
        fImpl->resultExtraInfo->setText( titleInfo->fExtraInfo );
    }

    if ( !episode.isEmpty() )
    {
        bool aOK;
        episode.toInt( &aOK );
        if ( !aOK )
            episode.clear();
    }

    if ( !season.isEmpty() )
    {
        bool aOK;
        season.toInt( &aOK );
        if ( !aOK )
            season.clear();
    }

    fImpl->searchName->setText( searchName );
    fImpl->searchSeason->setText( season );
    fImpl->searchEpisode->setText( episode );
    fImpl->releaseYear->setText( releaseDate );
    fImpl->tmdbid->setText( tmdbid );

    fImpl->resultEpisodeTitle->setText( episodeTitle );

    fImpl->searchName->setDelay( 1000 );
    fImpl->searchSeason->setDelay( 1000 );
    fImpl->searchEpisode->setDelay( 1000 );
    fImpl->releaseYear->setDelay( 1000 );
    fImpl->tmdbid->setDelay( 1000 );

    fImpl->byName->setChecked( tmdbid.isEmpty() );
    fImpl->byTMDBID->setChecked( !tmdbid.isEmpty() );
    slotByNameChanged();
    reset();

    connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
    connect( fImpl->searchName, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchSeason, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
    connect( fImpl->searchEpisode, &CDelayLineEdit::sigTextChanged, this, &CSelectTMDB::slotSearchTextChanged );
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
    if ( fIsMovie )
        fImpl->results->setHeaderLabels( QStringList() << "Title" << "TMDB ID" << "Release Date" << "Desc" );
    else
        fImpl->results->setHeaderLabels( QStringList() << "Title" << "TMDB ID" << "Season" << "Episode" << "Episode Title" << "Desc" );
}

CSelectTMDB::~CSelectTMDB()
{
}

const QString apiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
const QString apiKeyV4 = "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";

void CSelectTMDB::removeFromMap( std::map< QNetworkReply *, QTreeWidgetItem * > & map, QTreeWidgetItem *item )
{
    for ( auto &&ii = map.begin(); ii != map.end();  )
    {
        ii = ( ( *ii ).second == item ) ? map.erase( ii ) : std::next( ii );
    }
}

void CSelectTMDB::removeFromMaps( QTreeWidgetItem *item )
{
    removeFromMap( fImageInfoReplies, item );
    removeFromMap( fTVInfoReplies, item );
    removeFromMap( fSeasonInfoReplies, item );
}

void CSelectTMDB::deleteParent( QTreeWidgetItem *item )
{
    if ( !item )
        return;
    auto parent = item->parent();
    if ( item->type() == EItemType::eTVShow )
    {
        removeFromMaps( item );
        delete item;
        deleteParent( parent );
    }
    else if ( item->type() == EItemType::eSeason )
    {
        if ( item->childCount() == 0 )
        {
            removeFromMaps( item );
            delete item;
            deleteParent( parent );
        }
    }
    else if ( item->type() == EItemType::eTVShow )
    {
        if ( item->childCount() == 0 )
        {
            removeFromMaps( item );
            delete item;
        }
    }
}

void CSelectTMDB::slotRequestFinished( QNetworkReply *reply )
{
    if ( reply->error() != QNetworkReply::NoError )
    {
        QString title = "Unknown Issue";

        auto errorMsg = reply->errorString();
        auto pos = errorMsg.indexOf( " - server replied:" );
        QString prefix;
        if ( pos != -1 )
        {
            prefix = errorMsg.left( pos ).trimmed();
            errorMsg = errorMsg.mid( pos + 18 ).trimmed();
        }

        if ( reply == fConfigReply )
        {
            fConfigErrorCount++;
            if ( fConfigErrorCount < 5 )
            {
                QTimer::singleShot( 0, this, &CSelectTMDB::slotGetConfig );
                return;
            }
            title = tr( "Could not download configuration" );
        }
        else if ( reply == fSearchReply )
        {
            title = tr( "Could not search for movie(s)" );
            if ( errorMsg == "Not Found" )
            {
                auto msg = tr( "Could not find Movie" );
                QMessageBox::information( this, tr( "Could not find movie" ), msg );
                return;
            }
        }
        else if ( reply == fGetMovieReply )
        {
            auto url = fGetMovieReply->url();
            auto tmdbid = url.path().mid( url.path().lastIndexOf( '/' ) + 1 );
            auto msg = tr( "Could not find TMDBID: <b>%1</b>" ).arg( tmdbid );
            QMessageBox::information( this, tr( "Could not find TMDBID" ), msg );
            return;
        }
        else
        {
            if ( fImageInfoReplies.find( reply ) != fImageInfoReplies.end() )
            {
                title = tr( "Could not get image(s)" );
                auto pos = fImageInfoReplies.find( reply );
                delete ( *pos ).second;
                fImageInfoReplies.erase( pos );
            }
            else if ( fTVInfoReplies.find( reply ) != fTVInfoReplies.end() )
            {
                title = tr( "Could not find TV Details information" );
                auto pos = fTVInfoReplies.find( reply );
                delete ( *pos ).second;
                fTVInfoReplies.erase( pos );
            }
            else if ( fSeasonInfoReplies.find( reply ) != fSeasonInfoReplies.end() )
            {
                title = tr( "Could not find Season Details information" );
                auto pos = fSeasonInfoReplies.find( reply );
                deleteParent( ( *pos ).second );
            }
            if ( errorMsg == "Not Found" )
            {
                QMessageBox::information( this, title, title );
                return;
            }
        }

        QMessageBox::warning( this, title, QString( "%1<br>Error: <b>%2</b>" ).arg( prefix ).arg( errorMsg ) );
        return;
    }
    if ( reply == fConfigReply )
    {
        loadConfig();
        fConfigReply = nullptr;
        return;
    }

    if ( reply == fSearchReply )
    {
        loadSearchResult();
        fSearchReply = nullptr;
        return;
    }

    if ( reply == fGetMovieReply )
    {
        loadMovieResult();
        fGetMovieReply = nullptr;
        return;
    }
    
    loadImageResults( reply );
    loadTVDetails( reply );
    loadSeasonDetails( reply );
    autoAcceptOnFinished();
}

void CSelectTMDB::autoAcceptOnFinished()
{
    if ( fConfigReply )
        return;
    if ( fSearchReply )
        return;
    if ( fGetMovieReply )
        return;
    if ( fGetTVReply )
        return;
    if ( !fImageInfoReplies.empty() )
        return;
    if ( !fTVInfoReplies.empty() )
        return;
    if ( !fSeasonInfoReplies.empty() )
        return;

    auto selected = fImpl->results->selectedItems();
    if ( selected.count() == 1 )
        emit accept();
}

void CSelectTMDB::slotGetConfig()
{
    if ( fConfigErrorCount >= 5 )
        return;

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
        if ( fIsMovie )
            url.setPath( "/3/search/movie" );
        else
            url.setPath( "/3/search/tv" );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        query.addQueryItem( "include_adult", "true" );
        if ( !fImpl->releaseYear->text().isEmpty() )
            query.addQueryItem( "year", fImpl->releaseYear->text() );
        auto searchStrings =
            fImpl->searchName->text().split( " ", Qt::SplitBehaviorFlags::SkipEmptyParts );

        if ( searchStrings.isEmpty() )
            return;

        query.addQueryItem( "query", searchStrings.join( "+" ) );
        url.setQuery( query );

        //qDebug() << url.toString();
        fSearchReply = fManager->get( QNetworkRequest( url ) );
    }
    else if ( fIsMovie ) // by tmdbid
    {
        if ( fImpl->tmdbid->text().isEmpty() )
            return;

        url.setPath( QString( "/3/movie/%1" ).arg( fImpl->tmdbid->text() ) );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        url.setQuery( query );
        qDebug() << url.toString();
        fGetMovieReply = fManager->get( QNetworkRequest( url ) );
    }
    else
    {
        if ( fImpl->tmdbid->text().isEmpty() )
            return;

        url.setPath( QString( "/3/tv/%1" ).arg( fImpl->tmdbid->text() ) );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        url.setQuery( query );
        qDebug() << url.toString();
        fGetTVReply = fManager->get( QNetworkRequest( url ) );
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
    //for( int ii = 0; ii < posterSizes.size(); ++ii )
    //{
    //    auto curr = posterSizes[ii].toString();
    //}
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
    if ( !loadSearchResult( doc.object(), false ) )
    {
        auto url = fGetMovieReply->url();
        auto tmdbid = url.path().mid( url.path().lastIndexOf( '/' ) + 1 );
        auto foundTMDBID = doc.object().contains( "id" ) ? doc.object()["id"].toInt() : -1;

        QMessageBox::information( this, tr( "Could not find TMDBID" ), tr( "Found TMDBID <b>%1</b> did not match searched TMDBID of <b>%2</b>" ).arg( foundTMDBID ).arg( tmdbid ) );
    }

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
        bool found = false;
        for ( int ii = 0; ii < results.size(); ++ii )
        {
            found = loadSearchResult( results[ii].toObject(), results.count() > 1 ) || found;
        }
        if ( !found )
        {
            QMessageBox::information( this, tr( "Could not find Movie" ), tr( "No results found that match search criteria" ) );
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
bool CSelectTMDB::loadSearchResult( const QJsonObject &resultItem, bool multipleResults )
{
    qDebug().nospace().noquote() << QJsonDocument( resultItem ).toJson( QJsonDocument::Indented );

    auto tmdbid = resultItem.contains( "id" ) ? resultItem["id"].toInt() : -1;
    auto desc = resultItem.contains( "overview" ) ? resultItem["overview"].toString() : QString();
    QString title;
    if ( fIsMovie )
        title = resultItem.contains( "title" ) ? resultItem["title"].toString() : QString();
    else
        title = resultItem.contains( "name" ) ? resultItem["name"].toString() : QString();
    auto releaseDate = resultItem.contains( "release_date" ) ? resultItem["release_date"].toString() : QString();
    auto posterPath = resultItem.contains( "poster_path" ) ? resultItem["poster_path"].toString() : QString();

    bool aOK;
    int releaseYear = fImpl->releaseYear->text().toInt( &aOK );
    if ( aOK && fImpl->byName->isChecked() && !fImpl->releaseYear->text().isEmpty() && !releaseDate.isEmpty() )
    {
        auto dt = NQtUtils::findDate( releaseDate );

        if ( dt.isValid() && dt.year() != releaseYear )
        {
            return false;
        }
    }

    int searchTmdbid = fImpl->tmdbid->text().toInt( &aOK );
    bool canCheckTMDB = !fImpl->tmdbid->text().isEmpty() && ( tmdbid != -1 );
    if ( aOK && ( !fIsMovie && canCheckTMDB ) || ( fIsMovie && fImpl->byTMDBID->isChecked() && canCheckTMDB ) )
    {
        if ( tmdbid != searchTmdbid )
            return false;
    }
    if ( multipleResults && !canCheckTMDB && ( fImpl->searchName->text() != title ) )
        return false;

    auto label = new QLabel( desc, this );
    label->setWordWrap( true );
    QTreeWidgetItem *item = nullptr;
    if ( fIsMovie )
    {
        item = new QTreeWidgetItem( fImpl->results, QStringList() << title << QString::number( tmdbid ) << releaseDate << QString(), EItemType::eMovie );
        fImpl->results->setItemWidget( item, 3, label );
    }
    else
    {
        item = new QTreeWidgetItem( fImpl->results, QStringList() << title << QString::number( tmdbid ) << QString() << QString() << QString() << QString(), EItemType::eTVShow );
        item->setExpanded( true );
        fImpl->results->setItemWidget( item, 5, label );
    }

    fImpl->results->resizeColumnToContents( 0 );

    if ( !posterPath.isEmpty() && hasConfiguration() )
    {
        auto path = fConfiguration.value() + posterPath;
        QUrl url( path );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );
        url.setQuery( query );
        //qDebug() << url.toString();

        auto reply = fManager->get( QNetworkRequest( url ) );
        fImageInfoReplies[reply] = item;
    }

    if ( !fIsMovie && ( tmdbid != -1 ) )
    {
        getTVDetails( item, tmdbid, -1 );
    }

    return true;
}


//bool CSelectTMDB::loadEpisodeInfo( const QJsonObject &resultItem, QTreeWidgetItem *seasonItem )
//{
//    qDebug().nospace().noquote() << QJsonDocument( resultItem ).toJson( QJsonDocument::Indented );
//
//    auto releaseDate = resultItem.contains( "episode_number" ) ? resultItem["episode_number"].toInt() : 0;
//    auto posterPath = resultItem.contains( "poster_path" ) ? resultItem["poster_path"].toString() : QString();
//
//
//}

void CSelectTMDB::getTVDetails( QTreeWidgetItem * item, int tmdbid, int seasonNum )
{
    QUrl url;
    url.setScheme( "https" );
    url.setHost( "api.themoviedb.org" );

    auto path = QString( "/3/tv/%1" ).arg( tmdbid );
    auto season = ( seasonNum != -1 ) ? QString::number( seasonNum ) : fImpl->searchSeason->text();
    QTreeWidgetItem *seasonItem = nullptr;
    if ( !season.isEmpty() )
    {
        path += QString( "/season/%1" ).arg( season );
        seasonItem = new QTreeWidgetItem( item, QStringList() << QString() << QString() << season << QString() << QString() << QString(), EItemType::eSeason );
        seasonItem->setExpanded( true );
    }
    url.setPath( path );

    QUrlQuery query;
    query.addQueryItem( "api_key", apiKeyV3 );
    url.setQuery( query );

    auto reply = fManager->get( QNetworkRequest( url ) );
    if ( seasonItem )
        fSeasonInfoReplies[reply] = seasonItem;
    else
        fTVInfoReplies[reply] = item;
}

// from a /tv/tv_id request
void CSelectTMDB::loadTVDetails( QNetworkReply *reply )
{
    auto pos = fTVInfoReplies.find( reply );
    if ( pos == fTVInfoReplies.end() )
        return;
    auto showItem = ( *pos ).second;
    fTVInfoReplies.erase( pos );
    if ( !showItem )
        return;

    auto data = reply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    auto numSeasons = doc.object().contains( "number_of_seasons" ) ? doc.object()["number_of_seasons"].toInt() : -1;
    showItem->setText( 2, QString( "%1 Season%2" ).arg( numSeasons ).arg( numSeasons != 1 ? "s" : "" ) );
    
    auto seasons = doc.object()["seasons"].toArray();
    
    for ( auto &&ii : seasons )// int ii = 0; ii < seasons.count(); ++ii )
    {
        qDebug().nospace().noquote() << QJsonDocument( ii.toObject() ).toJson( QJsonDocument::Indented );
        getTVDetails( showItem, showItem->text( 1 ).toInt(), ii.toObject()["season_number"].toInt() );
    }
} 

void CSelectTMDB::loadSeasonDetails( QNetworkReply *reply )
{
    auto pos = fSeasonInfoReplies.find( reply );
    if ( pos == fSeasonInfoReplies.end() )
        return;
    auto seasonItem = ( *pos ).second;
    fSeasonInfoReplies.erase( pos );
    if ( !seasonItem )
        return;

    auto data = reply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    auto seasonName = doc.object().contains( "name" ) ? doc.object()["name"].toString() : QString();
    seasonItem->setText( 0, seasonName );

    auto episodes = doc.object()["episodes"].toArray();

    bool episodeFound = false;
    for ( auto && ii : episodes )
    {
        episodeFound = loadEpisodeDetails( ii.toObject(), seasonItem ) || episodeFound;
    }
    if ( !episodeFound )
    {
        deleteParent( seasonItem );
    }
}

bool CSelectTMDB::loadEpisodeDetails( const QJsonObject &episodeInfo, QTreeWidgetItem *seasonItem )
{
    auto episodeNumber = episodeInfo.contains( "episode_number" ) ? episodeInfo["episode_number"].toInt() : -1;
    if ( !fImpl->searchEpisode->text().isEmpty() )
    {
        if ( fImpl->searchEpisode->text().toInt() != episodeNumber )
            return false;
    }
    auto episodeName = episodeInfo.contains( "name" ) ? episodeInfo["name"].toString() : QString();
    auto overview = episodeInfo.contains( "overview" ) ? episodeInfo["overview"].toString() : QString();

    auto retVal = new QTreeWidgetItem( seasonItem, QStringList() << QString() << QString() << QString() << QString::number( episodeNumber ) << episodeName << overview, EItemType::eEpisode );
    if ( !fImpl->searchEpisode->text().isEmpty() )
        retVal->setSelected( true );
    return true;
}

void CSelectTMDB::loadImageResults( QNetworkReply * reply )
{
    auto pos = fImageInfoReplies.find( reply );
    if ( pos == fImageInfoReplies.end() )
        return;
    auto item = ( *pos ).second;
    fImageInfoReplies.erase( pos );
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
    retVal->fSeason = fImpl->resultSeason->text();
    retVal->fEpisode = fImpl->resultEpisode->text();
    retVal->fEpisodeTitle = fImpl->resultEpisodeTitle->text();
    retVal->fExtraInfo = fImpl->resultExtraInfo->text();
    retVal->fIsMovie = fIsMovie;

    return retVal;
}

QString CSelectTMDB::getText( QTreeWidgetItem * item, int column, bool forceTop ) const
{
    if ( !item )
        return QString();
    auto text = forceTop ? ( ( item->parent() == nullptr ) ? item->text( column ) : QString() ) : item->text( column );
    if ( text.isEmpty() )
        text = getText( item->parent(), column, forceTop );
    if ( text.startsWith( "Star Wars Rebels" ) )
        text = "Star Wars - Rebels";
    return text;
}

void CSelectTMDB::slotSelectionChanged()
{
    auto selected = fImpl->results->selectedItems();
    if ( selected.empty() )
        return;

    auto first = selected.front();
    if ( !first )
        return;


    fImpl->resultTitle->setText( getText( first, 0, true ) );
    fImpl->resultTMDBID->setText( getText( first, 1 ) );
    if ( fIsMovie )
    {
        fImpl->resultReleaseDate->setText( getText( first, 2 ) );
    }
    else
    {
        fImpl->resultSeason->setText( getText( first, 2 ) );
        fImpl->resultEpisode->setText( getText( first, 3 ) );
        fImpl->resultEpisodeTitle->setText( getText( first, 4 ) );
    }
}

void CSelectTMDB::slotByNameChanged()
{
    fImpl->byNameGroupBox->setEnabled( fImpl->byName->isChecked() );
    fImpl->tmdbid->setEnabled( !fImpl->byName->isChecked() );
    slotSearchTextChanged();
}