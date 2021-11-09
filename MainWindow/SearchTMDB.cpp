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

#include "SearchTMDB.h"
#include "TitleInfo.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
//#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
//#include <QNetworkRequest>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QPixmap>

SSearchTMDBInfo::SSearchTMDBInfo( const QString & text, std::shared_ptr< STitleInfo > titleInfo )
{
    fTitleInfo = titleInfo;
    fInitSearchString = text;
    updateSearchCriteria( true );
}

QString stripKnownData( const QString & string )
{
    QString retVal = string;
    auto separators = QStringList() << "1080p" << "720p" << "AMZN" << "WebRip" << "WEB" << "-RUMOUR" << "-PECULATE" << "x264" << "x265" << "h264" << "h265" << "rarbg" << "-";
    for ( auto &&separator : separators )
    {
        retVal.replace( "[" + separator + "]", "", Qt::CaseSensitivity::CaseInsensitive );
        retVal.replace( separator, "", Qt::CaseSensitivity::CaseInsensitive );
    }
    return retVal;
}

QString smartTrim( const QString & string, bool stripInnerPeriods =false )
{
    auto retVal = string;
    auto pos = retVal.indexOf( QRegularExpression( "[^\\.\\s\\-]" ) );
    if ( pos != -1 )
        retVal = retVal.mid( pos );

    pos = retVal.lastIndexOf( QRegularExpression( "[^\\.\\s\\-]" ) );
    if ( pos != -1 )
        retVal = retVal.left( pos + 1 );
    if ( stripInnerPeriods )
        retVal.replace( ".", " " );
    return retVal;
}

void SSearchTMDBInfo::updateSearchCriteria( bool updateSearchBy )
{
    fSearchName = fInitSearchString;
    QString seasonStr;
    QString episodeStr;

    fSearchName = smartTrim( stripKnownData( fSearchName ) );

    if ( fIsMovie )
    {
        auto regExp = QRegularExpression( "\\((?<releaseDate>\\d{2,4})\\)" );
        auto match = regExp.match( fSearchName );
        if ( match.hasMatch() )
        {
            fReleaseDate = smartTrim( match.captured( "releaseDate" ) );
            fSearchName.replace( regExp, "" );
        }
        else
        {
            auto regExp = QRegularExpression( "\\.(?<releaseDate>\\d{2,4})\\." );
            match = regExp.match( fSearchName );
            if ( match.hasMatch() )
            {
                fReleaseDate = smartTrim( match.captured( "releaseDate" ) );
                fSearchName.replace( regExp, "" );
            }
            else
            {
                auto regExp = QRegularExpression( "\\.(?<releaseDate>\\d{2,4})$" );
                match = regExp.match( fSearchName );
                if ( match.hasMatch() )
                {
                    fReleaseDate = smartTrim( match.captured( "releaseDate" ) );
                    fSearchName.replace( regExp, "" );
                }
            }
        }
        regExp = QRegularExpression( "\\[tmdbid=(?<tmdbid>\\d+)\\]" );
        match = regExp.match( fSearchName );
        if ( match.hasMatch() )
        {
            fTMDBID = smartTrim( match.captured( "tmdbid" ) );
            fSearchName.replace( regExp, "" );
        }

        fSearchName = smartTrim( fSearchName, true );
    }
    else
    {
        QString dataBeforeSeason;
        auto regExp = QRegularExpression( "S(?<season>\\d+)" );
        auto match = regExp.match( fSearchName );
        if ( match.hasMatch() )
        {
            seasonStr = smartTrim( match.captured( "season" ) );
            dataBeforeSeason = smartTrim( fSearchName.left( match.capturedStart( "season" ) - 1 ) );
            fSearchName = fSearchName.mid( match.capturedStart( "season" ) - 1 );
            fSearchName.replace( regExp, "" );
        }

        regExp = QRegularExpression( "E(?<episode>\\d+)" );
        match = regExp.match( fSearchName );
        if ( match.hasMatch() )
        {
            episodeStr = smartTrim( match.captured( "episode" ) );
            fSearchName.replace( regExp, "" );
        }

        if ( !dataBeforeSeason.isEmpty() )
            fSearchName = dataBeforeSeason;

        fSearchName = smartTrim( fSearchName, true );
    }

    if ( fTitleInfo )
    {
        fSearchName = fTitleInfo->fTitle;
        episodeStr = fTitleInfo->fEpisode;
        seasonStr = fTitleInfo->fSeason;
        fReleaseDate = fTitleInfo->fReleaseDate;
        fTMDBID = fTitleInfo->fTMDBID;
        fEpisodeTitle = fTitleInfo->fEpisodeTitle;
    }

    if ( !episodeStr.isEmpty() )
    {
        bool aOK;
        fEpisode = episodeStr.toInt( &aOK );
        if ( !aOK )
            fEpisode = -1;
    }

    if ( !seasonStr.isEmpty() )
    {
        bool aOK;
        fSeason = seasonStr.toInt( &aOK );
        if ( !aOK )
            fSeason = -1;
    }

    if ( updateSearchBy )
        fSearchByName = fTMDBID.isEmpty();
}

CSearchTMDB::CSearchTMDB( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::optional< QString > &configuration, QObject* parent )
    : QObject( parent ),
    fSearchInfo( searchInfo ),
    fConfiguration( configuration )
{
    fManager = new QNetworkAccessManager( this );
    connect( fManager, &QNetworkAccessManager::finished, this, &CSearchTMDB::slotRequestFinished );

    QTimer::singleShot( 0, this, &CSearchTMDB::slotGetConfig );
}


CSearchTMDB::~CSearchTMDB()
{
}

void CSearchTMDB::resetResults()
{
    fStopSearching = true;
    fErrorMessage.reset();
    fImageInfoReplies.clear();
    fTVInfoReplies.clear();
    fSeasonInfoReplies.clear();
    fTopLevelResults.clear();
}

const QString apiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
const QString apiKeyV4 = "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";

void CSearchTMDB::slotRequestFinished( QNetworkReply *reply )
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
                QTimer::singleShot( 0, this, &CSearchTMDB::slotGetConfig );
                return;
            }
            title = tr( "Could not download configuration" );
            fConfigReply = nullptr;
        }
        else if ( reply == fSearchReply )
        {
            fSearchReply = nullptr;
            title = tr( "Could not search for %1(s)" ).arg( fSearchInfo->fIsMovie ? tr( "Movie" ) : tr( "TV Show" ) );
            if ( errorMsg == "Not Found" )
            {
                fErrorMessage = tr( "Could not find %1" ).arg( fSearchInfo->fIsMovie ? tr( "Movie" ) : tr( "TV Show" ) );
                emit sigSearchFinished();
                return;
            }
        }
        else if ( reply == fGetMovieReply )
        {
            auto url = fGetMovieReply->url();
            auto tmdbid = url.path().mid( url.path().lastIndexOf( '/' ) + 1 );
            fErrorMessage = tr( "Could not find Movie with TMDBID: <b>%1</b>" ).arg( tmdbid );
            fGetMovieReply = nullptr;
            emit sigSearchFinished();
            return;
        }
        else if ( reply == fGetTVReply )
        {
            auto url = fGetTVReply->url();
            auto tmdbid = url.path().mid( url.path().lastIndexOf( '/' ) + 1 );
            fErrorMessage = tr( "Could not find TV Show with TMDBID: <b>%1</b>" ).arg( tmdbid );
            fGetTVReply = nullptr;
            emit sigSearchFinished();
            return;
        }
        else
        {
            if ( fImageInfoReplies.find( reply ) != fImageInfoReplies.end() )
            {
                title = tr( "Could not get image(s)" );
                auto pos = fImageInfoReplies.find( reply );
                fImageInfoReplies.erase( pos );
            }
            else if ( fTVInfoReplies.find( reply ) != fTVInfoReplies.end() )
            {
                title = tr( "Could not find TV Details information" );
                auto pos = fTVInfoReplies.find( reply );
                fTVInfoReplies.erase( pos );
            }
            else if ( fSeasonInfoReplies.find( reply ) != fSeasonInfoReplies.end() )
            {
                title = tr( "Could not find Season Details information" );
                auto pos = fSeasonInfoReplies.find( reply );
                fSeasonInfoReplies.erase( pos );
            }
            if ( errorMsg == "Not Found" )
            {
                fErrorMessage = title;
                return;
            }
        }

        fErrorMessage = prefix + "-" + errorMsg;
        emit sigSearchFinished();
        return;
    }
    if ( reply == fConfigReply )
    {
        loadConfig();
        fConfigReply = nullptr;
        checkIfStillSearching();
        return;
    }

    if ( reply == fSearchReply )
    {
        loadSearchResult();
        fSearchReply = nullptr;
        checkIfStillSearching();
        return;
    }

    if ( reply == fGetMovieReply )
    {
        loadMovieResult();
        fGetMovieReply = nullptr;
        checkIfStillSearching();
        return;
    }
    
    if ( reply == fGetTVReply )
    {
        loadTVResult();
        fGetTVReply = nullptr;
        checkIfStillSearching();
        return;
    }

    bool handled = loadImageResults( reply );
    handled = loadTVDetails( reply ) || handled;
    handled = loadSeasonDetails( reply ) || handled;

    if ( !handled )
        qDebug() << "Reply not handled:" << reply->url().toString();
    checkIfStillSearching();
}

void CSearchTMDB::checkIfStillSearching()
{
    if ( isActive() )
        return;
    emit sigSearchFinished();
}

bool CSearchTMDB::isActive() const
{
    if ( fConfigReply )
        return true;
    if ( fSearchReply )
        return true;
    if ( fGetMovieReply )
        return true;
    if ( fGetTVReply )
        return true;
    if ( !fImageInfoReplies.empty() )
        return true;
    if ( !fTVInfoReplies.empty() )
        return true;
    if ( !fSeasonInfoReplies.empty() )
        return true;
    return false;
}

void CSearchTMDB::slotGetConfig()
{
    if ( fConfigErrorCount >= 5 )
        return;

    if ( fConfiguration.has_value() )
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

bool CSearchTMDB::hasConfiguration() const
{
    return fConfiguration.has_value() && !fConfiguration.value().isEmpty();
}


void CSearchTMDB::loadConfig()
{
    if ( !fConfigReply )
        return;

    auto data = fConfigReply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );
    if ( !doc.object().contains( "images" ) )
        return;
    auto images = doc.object()["images"].toObject();
    if ( !images.contains( "poster_sizes" ) || !images["poster_sizes"].isArray() )
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
    fConfigReply = nullptr;
}

std::optional< std::pair< QUrl, ESearchType > > SSearchTMDBInfo::getSearchURL() const
{
    QUrl url;
    url.setScheme( "https" );
    url.setHost( "api.themoviedb.org" );
    if ( fSearchByName )
    {
        if ( fIsMovie )
            url.setPath( "/3/search/movie" );
        else
            url.setPath( "/3/search/tv" );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        query.addQueryItem( "include_adult", "true" );
        if ( !fReleaseDate.isEmpty() )
            query.addQueryItem( "year", fReleaseDate );
        auto searchStrings = fSearchName.split( QRegularExpression( "[\\s\\.]" ), TSkipEmptyParts );

        if ( searchStrings.isEmpty() )
            return {};

        query.addQueryItem( "query", searchStrings.join( "+" ) );
        url.setQuery( query );

        //qDebug() << url.toString();
        return std::make_pair( url, fIsMovie ? ESearchType::eSearchMovie : ESearchType::eSearchTV );
    }
    else if ( fIsMovie ) // by tmdbid
    {
        if ( fTMDBID.isEmpty() )
            return {};

        url.setPath( QString( "/3/movie/%1" ).arg( fTMDBID ) );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        url.setQuery( query );
        //qDebug() << url.toString();
        return std::make_pair( url, ESearchType::eGetMovie );
    }
    else
    {
        if ( fTMDBID.isEmpty() )
            return {};

        url.setPath( QString( "/3/tv/%1" ).arg( fTMDBID ) );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        url.setQuery( query );
        qDebug() << url.toString();
        return std::make_pair( url, ESearchType::eGetTVShow );
    }
}

void CSearchTMDB::slotSearch()
{
    if ( !hasConfiguration() )
    {
        QTimer::singleShot( 100, this, &CSearchTMDB::slotSearch );
        return;
    }

    fStopSearching = false;
    auto searchInfo = fSearchInfo->getSearchURL();
    if ( !searchInfo.has_value() )
        return;

    if ( fStopSearching )
        return;

    qDebug() << searchInfo.value().first.toString();
    switch ( searchInfo.value().second )
    {
        case ESearchType::eSearchTV:
        case ESearchType::eSearchMovie:
            fSearchReply = fManager->get( QNetworkRequest( searchInfo.value().first ) );
            break;
        case ESearchType::eGetMovie:
            fGetMovieReply = fManager->get( QNetworkRequest( searchInfo.value().first ) );
            break;
        case ESearchType::eGetTVShow:
            fGetTVReply = fManager->get( QNetworkRequest( searchInfo.value().first ) );
            break;
    }
}


void CSearchTMDB::loadSearchResult()
{
    if ( !fSearchReply )
        return;

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
            fErrorMessage = tr( "Could not find %1 - No results found that match search criteria" ).arg( fSearchInfo->fIsMovie ? tr( "Movie" ) : tr( "TV Show" ) );
            return;
        }
    }
}

void CSearchTMDB::loadMovieResult()
{
    if ( !fGetMovieReply )
        return;

    auto data = fGetMovieReply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );
    if ( !loadSearchResult( doc.object(), false ) )
    {
        auto url = fGetMovieReply->url();
        auto tmdbid = url.path().mid( url.path().lastIndexOf( '/' ) + 1 );
        auto foundTMDBID = doc.object().contains( "id" ) ? doc.object()["id"].toInt() : -1;

        fErrorMessage = tr( "Found TMDBID <b>%1</b> did not match searched TMDBID of <b>%2</b>" ).arg( foundTMDBID ).arg( tmdbid );
    }
}

void CSearchTMDB::loadTVResult()
{
    if ( !fGetTVReply )
        return;

    auto data = fGetTVReply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    if ( !loadSearchResult( doc.object(), false ) )
    {
        auto url = fGetTVReply->url();
        auto tmdbid = url.path().mid( url.path().lastIndexOf( '/' ) + 1 );
        auto foundTMDBID = doc.object().contains( "id" ) ? doc.object()["id"].toInt() : -1;

        fErrorMessage = tr( "Found TMDBID <b>%1</b> did not match searched TMDBID of <b>%2</b>" ).arg( foundTMDBID ).arg( tmdbid );
    }
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
bool CSearchTMDB::loadSearchResult( const QJsonObject &resultItem, bool multipleResults )
{
    qDebug().nospace().noquote() << QJsonDocument( resultItem ).toJson( QJsonDocument::Indented );

    auto tmdbid = resultItem.contains( "id" ) ? resultItem["id"].toInt() : -1;
    auto desc = resultItem.contains( "overview" ) ? resultItem["overview"].toString() : QString();
    QString title;
    if ( fSearchInfo->fIsMovie )
        title = resultItem.contains( "title" ) ? resultItem["title"].toString() : QString();
    else
        title = resultItem.contains( "name" ) ? resultItem["name"].toString() : QString();
    auto releaseDate = resultItem.contains( "release_date" ) ? resultItem["release_date"].toString() : QString();
    auto posterPath = resultItem.contains( "poster_path" ) ? resultItem["poster_path"].toString() : QString();

    bool aOK;
    int releaseYear = fSearchInfo->fReleaseDate.toInt( &aOK );
    if ( aOK && fSearchInfo->fSearchByName && !fSearchInfo->fReleaseDate.isEmpty() && !releaseDate.isEmpty() )
    {
        auto dt = NQtUtils::findDate( releaseDate );

        if ( dt.isValid() && dt.year() != releaseYear )
        {
            return false;
        }
    }

    int searchTmdbid = fSearchInfo->fTMDBID.toInt( &aOK );
    bool canCheckTMDB = !fSearchInfo->fTMDBID.isEmpty() && ( tmdbid != -1 );
    if ( aOK && canCheckTMDB && fSearchInfo->fIsMovie ) // dont check for TV shows, as the TMDB could be the episode ID
    {
        if ( tmdbid != searchTmdbid )
            return false;
    }
    if ( multipleResults && !canCheckTMDB && fSearchInfo->fExactMatchOnly && ( fSearchInfo->fSearchName != title ) )
        return false;

    auto searchResults = std::make_shared< STitleInfo >();
    searchResults->fDescription = desc;
    searchResults->fReleaseDate = releaseDate;
    searchResults->fTitle = title;
    searchResults->fTMDBID = QString::number( tmdbid );

    if ( resultItem.contains( "number_of_seasons" ) )
    {
        searchResults->fSeason = QString( "%1 Season%2" ).arg( resultItem["number_of_seasons"].toInt() ).arg( resultItem["number_of_seasons"].toInt() == 1 ? "" : "s" );
    }
    if ( resultItem.contains( "number_of_episodes" ) )
    {
        searchResults->fEpisode = QString( "%1 Episode%2" ).arg( resultItem["number_of_episodes"].toInt() ).arg( resultItem["number_of_episodes"].toInt() == 1 ? "" : "s" );
    }
    if ( !posterPath.isEmpty() && hasConfiguration() )
    {
        auto path = fConfiguration.value() + posterPath;
        QUrl url( path );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );
        url.setQuery( query );
        //qDebug() << url.toString();

        if ( !fStopSearching )
        {
            auto reply = fManager->get( QNetworkRequest( url ) );
            fImageInfoReplies[reply] = searchResults;
        }
        else
            int xyx = 0;
    }

    if ( !fSearchInfo->fIsMovie && ( tmdbid != -1 ) )
    {
        searchTVDetails( searchResults, tmdbid, -1 );
    }

    fTopLevelResults.push_back( searchResults );
    return true;
}

void CSearchTMDB::searchTVDetails( std::shared_ptr< STitleInfo > info, int tmdbid, int seasonNum )
{
    QUrl url;
    url.setScheme( "https" );
    url.setHost( "api.themoviedb.org" );

    auto path = QString( "/3/tv/%1" ).arg( tmdbid );
    if ( seasonNum == -1 )
        seasonNum = fSearchInfo->fSeason;
    std::shared_ptr< STitleInfo > seasonInfo;
    if ( seasonNum != -1 )
    {
        path += QString( "/season/%1" ).arg( seasonNum );
        seasonInfo = std::make_shared< STitleInfo >();
        seasonInfo->fTitle = info->fTitle;
        seasonInfo->fTMDBID = info->fTMDBID;
        seasonInfo->fIsMovie = false;
        seasonInfo->fSeason = QString::number( seasonNum );
        info->fChildren.push_back( seasonInfo );
        seasonInfo->fParent = info;
    }
    url.setPath( path );

    QUrlQuery query;
    query.addQueryItem( "api_key", apiKeyV3 );
    url.setQuery( query );

    if ( !fStopSearching )
    {
        auto reply = fManager->get( QNetworkRequest( url ) );
        if ( seasonInfo )
            fSeasonInfoReplies[reply] = seasonInfo;
        else
            fTVInfoReplies[reply] = info;
    }
    else
        int xyz = 0;
}

// from a /tv/tv_id request
bool CSearchTMDB::loadTVDetails( QNetworkReply *reply )
{
    auto pos = fTVInfoReplies.find( reply );
    if ( pos == fTVInfoReplies.end() )
        return false;
    auto showInfo = ( *pos ).second;
    fTVInfoReplies.erase( pos );
    if ( !showInfo )
        return false;

    auto data = reply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    auto numSeasons = doc.object().contains( "number_of_seasons" ) ? doc.object()["number_of_seasons"].toInt() : -1;
    //showInfo->fEpisodeTitle = QString( "%1 Season%2" ).arg( numSeasons ).arg( numSeasons != 1 ? "s" : "" );
    
    auto seasons = doc.object()["seasons"].toArray();
    
    for ( auto &&ii : seasons )// int ii = 0; ii < seasons.count(); ++ii )
    {
        qDebug().nospace().noquote() << QJsonDocument( ii.toObject() ).toJson( QJsonDocument::Indented );
        searchTVDetails( showInfo, showInfo->fTMDBID.toInt(), ii.toObject()["season_number"].toInt() );
    }
    return true;
} 

bool CSearchTMDB::loadSeasonDetails( QNetworkReply *reply )
{
    auto pos = fSeasonInfoReplies.find( reply );
    if ( pos == fSeasonInfoReplies.end() )
        return false;
    auto seasonInfo = ( *pos ).second;
    fSeasonInfoReplies.erase( pos );
    if ( !seasonInfo )
        return false;

    auto data = reply->readAll();
    auto doc = QJsonDocument::fromJson( data );
    qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    seasonInfo->fEpisodeTitle = doc.object().contains( "name" ) ? doc.object()["name"].toString() : QString();;
    seasonInfo->fSeason = doc.object().contains( "season_number" ) ? QString::number( doc.object()["season_number"].toInt() ) : QString();

    auto episodes = doc.object()["episodes"].toArray();
    seasonInfo->fEpisode = QString( "%1 Episode%2" ).arg( episodes.count() ).arg( episodes.count() == 1 ? "" : "s" );
    seasonInfo->fTMDBID = doc.object().contains( "id" ) ? QString::number( doc.object()["id"].toInt() ) : QString();

    bool episodeFound = false;
    for ( auto && ii : episodes )
    {
        episodeFound = loadEpisodeDetails( ii.toObject(), seasonInfo ) || episodeFound;
    }
    //(void)episodeFound;
    return true;
}

bool CSearchTMDB::loadEpisodeDetails( const QJsonObject & episodeObj, std::shared_ptr< STitleInfo > seasonInfo )
{
    auto episodeNumber = episodeObj.contains( "episode_number" ) ? episodeObj["episode_number"].toInt() : -1;
    if ( fSearchInfo->fEpisode != -1 )
    {
        if ( fSearchInfo->fEpisode != episodeNumber )
            return false;
    }
    auto episodeName = episodeObj.contains( "name" ) ? episodeObj["name"].toString() : QString();
    auto overview = episodeObj.contains( "overview" ) ? episodeObj["overview"].toString() : QString();

    auto episodeInfo = std::make_shared< STitleInfo >();
    episodeInfo->fEpisode = episodeNumber == -1 ? QString() : QString::number( episodeNumber );
    episodeInfo->fSeason = episodeObj.contains( "season_number" ) ? QString::number( episodeObj["season_number"].toInt() ) : QString();
    episodeInfo->fEpisodeTitle = episodeName;
    episodeInfo->fDescription = overview;
    episodeInfo->fTMDBID = episodeObj.contains( "id" ) ? QString::number( episodeObj["id"].toInt() ) : QString();
    episodeInfo->fTitle = seasonInfo->fTitle;
    episodeInfo->fReleaseDate = episodeObj.contains( "air_date" ) ? episodeObj["air_date"].toString() : QString();;

    seasonInfo->fChildren.push_back( episodeInfo );
    if ( fSearchInfo->fEpisode != -1 )
        fBestMatch = episodeInfo;
    return true;
}

bool CSearchTMDB::loadImageResults( QNetworkReply * reply )
{
    auto pos = fImageInfoReplies.find( reply );
    if ( pos == fImageInfoReplies.end() )
        return false;
    auto info = ( *pos ).second;
    fImageInfoReplies.erase( pos );
    if ( !info )
        return false;

    QImage img;
    img.loadFromData( reply->readAll() );
    QPixmap pm = QPixmap::fromImage( img );
    qDebug() << pm.size();

    info->fPixmap = pm;
    return true;
}
