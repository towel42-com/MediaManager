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
#include "SearchTMDBInfo.h"
#include "NetworkReply.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QPixmap>

QDebug operator<<( QDebug debug, const CSearchTMDB &searchTMDB )
{
    debug.nospace().noquote() << "(" << searchTMDB.toString() << ")";
    return debug;
}

CSearchTMDB::CSearchTMDB( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::optional< QString > &configuration, QObject* parent )
    : QObject( parent ),
    fSearchInfo( searchInfo ),
    fConfiguration( configuration )
{
    fManager = new QNetworkAccessManager( this );
    connect( fManager, &QNetworkAccessManager::authenticationRequired, this, &CSearchTMDB::slotAuthenticationRequired );
    connect( fManager, &QNetworkAccessManager::encrypted, this, &CSearchTMDB::slotEncrypted );
    connect( fManager, &QNetworkAccessManager::finished, this, &CSearchTMDB::slotRequestFinished );
    connect( fManager, &QNetworkAccessManager::preSharedKeyAuthenticationRequired, this, &CSearchTMDB::slotPreSharedKeyAuthenticationRequired );
    connect( fManager, &QNetworkAccessManager::proxyAuthenticationRequired, this, &CSearchTMDB::slotProxyAuthenticationRequired );
    connect( fManager, &QNetworkAccessManager::sslErrors, this, &CSearchTMDB::slotSSlErrors );

    connect( this, &CSearchTMDB::sigFakeRequestFinished, this, &CSearchTMDB::slotFakeRequestFinished );
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
    fSeasonInfoReplies.first.clear();
    fSeasonInfoReplies.second.reset();
    fTopLevelResults.clear();
    fCurrentQueuedSearch = {};
    fSearchQueue.clear();
    fQueuedResults.clear();
}

void CSearchTMDB::addSearch( const QString &filePath, std::shared_ptr< SSearchTMDBInfo > searchInfo )
{
    qDebug() << "AddSearch: " << filePath << searchInfo->searchName();

    fSearchQueue.push_back( std::make_pair( filePath, searchInfo ) );
    startAutoSearchTimer();

    //qDebug() << "AddSearch After: SearchTMBD" << *this;
}

QString CSearchTMDB::toString() const
{
    QString retVal =
        QString( "CSearchTMDB(Manager: 0x%1 " ).arg( reinterpret_cast<uintptr_t>( fManager ), 0, 16 )
        //+ QString( "ConfigReply: 0x%1 " ).arg( reinterpret_cast<uintptr_t>( fConfigReply.ifrst ), 0, 16 )
        //+ QString( "SearchReply: 0x%1 " ).arg( reinterpret_cast<uintptr_t>( fSearchReply ), 0, 16 )
        //+ QString( "GetMovieReply: 0x%1 " ).arg( reinterpret_cast<uintptr_t>( fGetMovieReply ), 0, 16 )
        //+ QString( "GetTVReply: 0x%1 " ).arg( reinterpret_cast<uintptr_t>( fGetTVReply ), 0, 16 )
        ;

    //retVal += QString( "ImageInfoReplies( %1 -" ).arg( fImageInfoReplies.size() );
    //for ( auto &&ii : fImageInfoReplies )
    //    retVal += QString( "Reply: 0x%1 - Title Info: %3" ).arg( reinterpret_cast<uintptr_t>( ii.first ), 0, 16 ).arg( ii.second->toString() );
    //retVal += ") ";

    //retVal += QString( "TVInfoReplies( %1 -" ).arg( fTVInfoReplies.size() );
    //for ( auto &&ii : fTVInfoReplies )
    //    retVal += QString( "Reply: 0x%1 - Title Info: %3" ).arg( reinterpret_cast<uintptr_t>( ii.first ), 0, 16 ).arg( ii.second->toString() );
    //retVal += ") ";

    //retVal += QString( "SeasonInfoReplies( %1 -" ).arg( fSeasonInfoReplies.first.size() );
    //for ( auto &&ii : fSeasonInfoReplies.first )
    //    retVal += QString( "Reply: 0x%1 - Title Info: %3" ).arg( reinterpret_cast<uintptr_t>( ii.first ), 0, 16 ).arg( ii.second->toString() );
    //retVal += ") Episode Found: " + QString( "%1" ).arg( fSeasonInfoReplies.second.has_value() ? fSeasonInfoReplies.second.value() : "No Value" ) + " ";

    retVal += ( fSearchInfo ? fSearchInfo->toString() : "nullptr" ) + " ";
    retVal += "CurrentQueuedSearch(";
    if ( fCurrentQueuedSearch.has_value() )
    {
        retVal += fCurrentQueuedSearch.value().first + " " + fCurrentQueuedSearch.value().second->toString();
    }
    else
        retVal += "nullptr";
    retVal += ") ";

    retVal += QString( "QueuedResults( %1 -" ).arg( fQueuedResults.size() );
    for( auto && ii : fQueuedResults )
    {
        retVal += QString( "(%1 - %2)" ).arg( ii.first ).arg( ii.second->toString() );
    }
    retVal += ") ";

    retVal += QString( "SearchQueue( %1 -" ).arg( fSearchQueue.size() );
    for ( auto &&ii : fSearchQueue )
    {
        retVal += QString( "(%1 - %2)" ).arg( ii.first ).arg( ii.second->toString() );
    }
    retVal += ") ";
    retVal += QString( "AutoSearchTimer isActive? %1 " ).arg( fAutoSearchTimer && fAutoSearchTimer->isActive() );

    retVal += QString( "Error Message: %1 " ).arg( fErrorMessage.has_value() ? fErrorMessage.value() : QString() );
    retVal += QString( "Configuration: %1 ErrorCount: %2 " ).arg( fConfiguration.has_value() ? fConfiguration.value() : QString( "<notset>") ).arg( fConfigErrorCount );;

    retVal += QString( "StopSearching: %1 SkipImages: %2 " ).arg( fStopSearching ).arg( fSkipImages );;
    retVal += QString( "BestMatch: %1" ).arg( fBestMatch ? fBestMatch->toString() : QString() );
    retVal += QString( "TopLevelResults( %1 -" ).arg( fTopLevelResults.size() );
    for ( auto &&ii : fTopLevelResults )
    {
        retVal += QString( "(%1) " ).arg( ii->toString() );
    }
    retVal += ") ";
    retVal += ")";
    return retVal;
}

void CSearchTMDB::startAutoSearchTimer()
{
    if ( !fAutoSearchTimer )
    {
        fAutoSearchTimer = new QTimer( this );
        fAutoSearchTimer->setInterval( 100 );
        fAutoSearchTimer->setSingleShot( true );
        connect( fAutoSearchTimer, &QTimer::timeout, this, &CSearchTMDB::slotAutoSearch );
    }
    fAutoSearchTimer->stop();
    fAutoSearchTimer->start();
}

const QString apiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
const QString apiKeyV4 = "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";

void CSearchTMDB::slotAuthenticationRequired( QNetworkReply * /*reply*/, QAuthenticator * /*authenticator*/ )
{
    //qDebug() << "slotAuthenticationRequired:" << reply << reply->url().toString() << authenticator;
}

void CSearchTMDB::slotEncrypted( QNetworkReply * /*reply*/ )
{
    //qDebug() << "slotEncrypted:" << reply << reply->url().toString();
}

std::shared_ptr< CNetworkReply > CSearchTMDB::sendRequest( const QNetworkRequest & request, ERequestType requestType )
{
    auto path = CNetworkReply::key( request.url() );
    auto pos = fURLResultsCache.find( path );
    if ( pos != fURLResultsCache.end() )
    {
        qDebug().noquote().nospace() << "Cached Result " << ::toString( requestType ) << " request:" << ( *pos ).first;
        emit sigFakeRequestFinished( requestType, (*pos).first, (*pos).second );
        return std::make_shared< CNetworkReply >(requestType, (*pos).first, (*pos).second );
    }
    auto retVal = fManager->get( request );
    qDebug().noquote().nospace() << "Sending " << ::toString( requestType ) << " request:" << request.url() << "Reply: 0x" << Qt::hex << reinterpret_cast< uint64_t >( retVal );
    //qDebug() << *this;
    addRequestType( retVal, requestType );
    return std::make_shared< CNetworkReply >( requestType, retVal );
}

void CSearchTMDB::addRequestType( QNetworkReply * retVal, ERequestType requestType )
{
    fRequestTypeMap[retVal] = requestType;
}

void CSearchTMDB::removeRequestType( std::shared_ptr<CNetworkReply> reply )
{
    if ( !reply )
        return;

    auto pos = fRequestTypeMap.find( reply->getNetworkReply() );
    fRequestTypeMap.erase( pos );
}

ERequestType CSearchTMDB::getRequestType( QNetworkReply * reply ) const
{
    auto pos = fRequestTypeMap.find( reply );
    Q_ASSERT( pos != fRequestTypeMap.end() );
    auto requestType = ERequestType::eUnknownRequest;
    if ( pos != fRequestTypeMap.end() )
        requestType = ( *pos ).second;
    return requestType;
}

void CSearchTMDB::slotFakeRequestFinished( ERequestType requestType, const QString & url, const QByteArray & cachedReply )
{
    handleRequestFinished( std::make_shared< CNetworkReply >( requestType, url, cachedReply ) );
}

void CSearchTMDB::slotRequestFinished( QNetworkReply * reply )
{
    handleRequestFinished( std::make_shared< CNetworkReply >( getRequestType( reply ), reply ) );
}

void CSearchTMDB::handleRequestFinished( std::shared_ptr< CNetworkReply > reply )
{
    Q_ASSERT( reply && reply->isValid() );
    qDebug() << "Reply Finished:" << Qt::hex << *reply;
    //qDebug() << "Before" << *this;
    if ( reply && reply->hasError() ) // replys with an error do not get cached
    {
        bool careAboutError = true;
        QString title = "Unknown Issue";

        auto errorMsg = reply->errorString();
        auto pos = errorMsg.indexOf( " - server replied:" );
        QString prefix;
        if ( pos != -1 )
        {
            prefix = errorMsg.left( pos ).trimmed();
            errorMsg = errorMsg.mid( pos + 18 ).trimmed();
        }

        if ( reply->isReply( fConfigReply ) )
        {
            fConfigErrorCount++;
            if ( fConfigErrorCount < 5 )
            {
                QTimer::singleShot( 0, this, &CSearchTMDB::slotGetConfig );
                return;
            }
            title = tr( "Could not download configuration" );
            fConfigReply.reset();
        }
        else if ( reply->isReply( fSearchReply ) )
        {
            fSearchReply.reset();
            title = tr( "Could not search for %1(s)" ).arg( getSearchName() );
            if ( errorMsg == "Not Found" )
            {
                fErrorMessage = tr( "Could not find %1" ).arg( getSearchName() );
                emitSigFinished();
                return;
            }
        }
        else if ( reply->isReply( fGetMovieReply ) )
        {
            fErrorMessage = tr( "Could not find Movie with TMDBID: <b>%1</b>" ).arg( fGetMovieReply->getTMDBIDFromURL() );
            fGetMovieReply.reset();
            emitSigFinished();
            return;
        }
        else if ( reply->isReply( fGetTVReply ) )
        {
            fErrorMessage = tr( "Could not find TV Show with TMDBID: <b>%1</b>" ).arg( fGetTVReply->getTMDBIDFromURL() );
            fGetTVReply.reset();
            emitSigFinished();
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
            else if ( fSeasonInfoReplies.first.find( reply ) != fSeasonInfoReplies.first.end() )
            {
                title = tr( "Could not find Season/Episode information" );
                auto pos = fSeasonInfoReplies.first.find( reply );
                auto info = ( *pos ).second;
                fSeasonInfoReplies.first.erase( pos );
                auto parent = info->fParent.lock();
                if ( parent )
                    parent->removeChild( info );
                careAboutError = fSeasonInfoReplies.first.empty() && !fSeasonInfoReplies.second.has_value();
            }
            if ( careAboutError && ( errorMsg == "Not Found" ) )
            {
                fErrorMessage = title;
                return;
            }
        }

        if ( careAboutError )
        {
            fErrorMessage = prefix + "-" + errorMsg;
            emitSigFinished();
            return;
        }
        else
            return;
    }
    bool handled = loadConfig( reply );
    handled = handled || loadSearchResult( reply );
    handled = handled || loadMovieResult( reply );
    handled = handled || loadTVResult( reply );
    handled = handled || loadImageResults( reply );
    handled = handled || loadTVDetails( reply );
    handled = handled || loadSeasonDetails( reply );

    if ( !handled )
    {
        reply->reportUnhandled();
    }

    // cache it for nexttime
    if ( !reply->isCached() )
    {
        fURLResultsCache[ reply->urlPathKey() ] = reply->getData();
        removeRequestType( reply );
    }


    checkIfStillSearching();
}

void CSearchTMDB::slotPreSharedKeyAuthenticationRequired( QNetworkReply * /*reply*/, QSslPreSharedKeyAuthenticator * /*authenticator*/ )
{
    //qDebug() << "slotPreSharedKeyAuthenticationRequired: 0x" << Qt::hex << reply << reply->url().toString() << authenticator;
}

void CSearchTMDB::slotProxyAuthenticationRequired( const QNetworkProxy & /*proxy*/, QAuthenticator * /*authenticator*/ )
{
    //qDebug() << "slotProxyAuthenticationRequired: 0x" << Qt::hex << &proxy << authenticator;
}

void CSearchTMDB::slotSSlErrors( QNetworkReply * /*reply*/, const QList<QSslError> & /*errors*/ )
{
    //qDebug() << "slotSSlErrors: 0x" << Qt::hex << reply << errors;
}

QString CSearchTMDB::getSearchName() const
{
    return fSearchInfo->isTVShow() ? tr( "TV Show" ) : tr( "Movie" );
}

void CSearchTMDB::checkIfStillSearching()
{
    if ( isActive() )
        return;
    if ( fSeasonInfoReplies.second.has_value() && !fSeasonInfoReplies.second.value() )
    {
        fErrorMessage = QString( "Could not find episode '%1' for TV show '%2'" ).arg( fSearchInfo->episode() ).arg( fSearchInfo->searchName() );
    }
    //qDebug() << "After" << *this;
    emitSigFinished();
}

void CSearchTMDB::emitSigFinished()
{
    if ( fCurrentQueuedSearch.has_value() )
    {
        auto path = fCurrentQueuedSearch.value().first;
        //qDebug() << "Sending autoSearchFinished " << path;
        emit sigAutoSearchFinished( path );
        fCurrentQueuedSearch.reset();
        fSearchInfo.reset();
        startAutoSearchTimer();
    }
    else
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
    if ( !fSeasonInfoReplies.first.empty() )
        return true;
    return false;
}

std::shared_ptr< STitleInfo > CSearchTMDB::getResult( const QString &path ) const
{
    auto pos = fQueuedResults.find( path );
    if ( pos == fQueuedResults.end() )
        return {};
    return ( *pos ).second;
}

std::list< std::shared_ptr< STitleInfo > > CSearchTMDB::getResults() const
{
    return fTopLevelResults;
}

bool CSearchTMDB::searchByName()
{
    return fSearchInfo->searchByName();
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
    fConfigReply = sendRequest( QNetworkRequest( url ), ERequestType::eConfig );
}

bool CSearchTMDB::hasConfiguration() const
{
    return fConfiguration.has_value() && !fConfiguration.value().isEmpty();
}


bool CSearchTMDB::loadConfig( std::shared_ptr< CNetworkReply > reply )
{
    if ( !fConfigReply || !reply->isType( ERequestType::eConfig ) )
        return false;

    auto data = reply->getData();
    fConfigReply.reset();

    auto doc = QJsonDocument::fromJson( data );
    //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );
    if ( !doc.object().contains( "images" ) )
        return false;
    auto images = doc.object()["images"].toObject();
    if ( !images.contains( "poster_sizes" ) || !images["poster_sizes"].isArray() )
        return false;
    auto posterSizes = images["poster_sizes"].toArray();

    QString posterSize = "original";
    //for( int ii = 0; ii < posterSizes.size(); ++ii )
    //{
    //    auto curr = posterSizes[ii].toString();
    //}
    auto posterURL = images.contains( "secure_base_url" ) ? images["secure_base_url"].toString() : QString();
    if ( !posterURL.isEmpty() )
        fConfiguration = posterURL + posterSize;
    return true;
}

std::optional< std::pair< QUrl, ESearchType > > SSearchTMDBInfo::getSearchURL() const
{
    QUrl url;
    url.setScheme( "https" );
    url.setHost( "api.themoviedb.org" );
    if ( fSearchByName )
    {
        if ( fIsTVShow )
            url.setPath( "/3/search/tv" );
        else
            url.setPath( "/3/search/movie" );
        
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
        return std::make_pair( url, fIsTVShow ? ESearchType::eSearchTV : ESearchType::eSearchMovie );
    }
    else if ( fIsTVShow ) // by tmdbid
    {
        if ( fTMDBID.isEmpty() )
            return {};

        url.setPath( QString( "/3/tv/%1" ).arg( fTMDBID ) );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );

        url.setQuery( query );
        //qDebug() << url.toString();
        return std::make_pair( url, ESearchType::eGetTVShow );
    }
    else
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
}

void CSearchTMDB::slotAutoSearch()
{
    //qDebug() << "slotAutoSearch";
    slotSearch();
}

void CSearchTMDB::slotSearch()
{
    //qDebug() << "slotSearch";
    if ( !hasConfiguration() )
    {
        QTimer::singleShot( 100, this, &CSearchTMDB::slotSearch );
        return;
    }

    fStopSearching = false;
    if ( !fSearchInfo && fSearchQueue.empty() )
        return;

    if ( !fSearchInfo )
    {
        fCurrentQueuedSearch = fSearchQueue.front();
        fSearchQueue.pop_front();
        fSearchInfo = fCurrentQueuedSearch.value().second;
    }

    auto searchInfo = fSearchInfo->getSearchURL();
    if ( !searchInfo.has_value() )
        return;

    if ( fStopSearching )
        return;

    //qDebug() << searchInfo.value().first.toString();
    switch ( searchInfo.value().second )
    {
        case ESearchType::eSearchTV:
        case ESearchType::eSearchMovie:
            fSearchReply = sendRequest( QNetworkRequest( searchInfo.value().first ), ( searchInfo.value().second == ESearchType::eSearchMovie ) ? ERequestType::eMovieSearch : ERequestType::eTVSearch );
            break;
        case ESearchType::eGetMovie:
            fGetMovieReply = sendRequest( QNetworkRequest( searchInfo.value().first ), ERequestType::eGetMovie );
            break;
        case ESearchType::eGetTVShow:
            fGetTVReply = sendRequest( QNetworkRequest( searchInfo.value().first ), ERequestType::eGetTVShow );
            break;
    }
}

bool CSearchTMDB::loadSearchResult( std::shared_ptr< CNetworkReply > reply )
{
    if ( !fSearchReply || ( !reply->isType( ERequestType::eMovieSearch ) && !reply->isType( ERequestType::eTVSearch ) ) )
        return false;

    auto data = reply->getData();
    fSearchReply.reset();

    auto doc = QJsonDocument::fromJson( data );
    //qDebug().noquote().nospace() << doc.toJson( QJsonDocument::Indented );
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
            fErrorMessage = tr( "Could not find %1 - No results found that match search criteria" ).arg( getSearchName() );
            return false;
        }
    }
    return true;
}

bool CSearchTMDB::loadMovieResult( std::shared_ptr< CNetworkReply > reply )
{
    if ( !fGetMovieReply || !reply->isType( ERequestType::eGetMovie ) )
        return false;

    auto data = fGetMovieReply->getData();
    auto tmdbid = fGetMovieReply->getTMDBIDFromURL();
    fGetMovieReply.reset();

    auto doc = QJsonDocument::fromJson( data );
    //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );
    if ( !loadSearchResult( doc.object(), false ) )
    {
        auto foundTMDBID = doc.object().contains( "id" ) ? doc.object()["id"].toInt() : -1;

        fErrorMessage = tr( "Found TMDBID <b>%1</b> did not match searched TMDBID of <b>%2</b>" ).arg( foundTMDBID ).arg( tmdbid );
        return false;
    }
    return true;
}

bool CSearchTMDB::loadTVResult( std::shared_ptr< CNetworkReply > reply )
{
    if ( !fGetTVReply || !reply->isType( ERequestType::eGetTVShow ) )
        return false;

    auto data = reply->getData();
    auto tmdbid = reply->getTMDBIDFromURL();
    fGetTVReply.reset();

    auto doc = QJsonDocument::fromJson( data );
    //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    if ( !loadSearchResult( doc.object(), false ) )
    {
        auto foundTMDBID = doc.object().contains( "id" ) ? doc.object()["id"].toInt() : -1;

        fErrorMessage = tr( "Found TMDBID <b>%1</b> did not match searched TMDBID of <b>%2</b>" ).arg( foundTMDBID ).arg( tmdbid );
    }
    return true;
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
    //qDebug().nospace().noquote() << QJsonDocument( resultItem ).toJson( QJsonDocument::Indented );

    if ( !fSearchInfo )
        return false;

    auto tmdbid = resultItem.contains( "id" ) ? resultItem["id"].toInt() : -1;
    auto desc = resultItem.contains( "overview" ) ? resultItem["overview"].toString() : QString();
    QString title;
    if ( fSearchInfo->isTVShow() )
        title = resultItem.contains( "name" ) ? resultItem["name"].toString() : QString();
    else
        title = resultItem.contains( "title" ) ? resultItem["title"].toString() : QString();
    auto releaseDate = resultItem.contains( "release_date" ) ? resultItem["release_date"].toString() : QString();
    auto posterPath = resultItem.contains( "poster_path" ) ? resultItem["poster_path"].toString() : QString();

    bool aOK;
    int releaseYear = fSearchInfo->releaseDate( &aOK );
    if ( aOK && fSearchInfo->searchByName() && fSearchInfo->releaseDateSet() && !releaseDate.isEmpty() )
    {
        auto dt = NQtUtils::findDate( releaseDate );

        if ( dt.isValid() && dt.year() != releaseYear )
        {
            return false;
        }
    }

    int searchTmdbid = fSearchInfo->tmdbID( &aOK );
    bool canCheckTMDB = fSearchInfo->tmdbIDSet() && ( tmdbid != -1 );
    if ( aOK && canCheckTMDB && !fSearchInfo->isTVShow() ) // dont check for TV shows, as the TMDB could be the episode ID
    {
        if ( tmdbid != searchTmdbid )
            return false;
    }
    if ( multipleResults && !canCheckTMDB && fSearchInfo->exactMatchOnly() && ( fSearchInfo->searchName() != title ) )
        return false;

    auto searchResults = std::make_shared< STitleInfo >( fSearchInfo->isTVShow() ? ETitleInfoType::eTVShow : ETitleInfoType::eMovie ); // movie or TV show
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
    if ( !posterPath.isEmpty() && hasConfiguration() && !fSkipImages )
    {
        auto path = fConfiguration.value() + posterPath;
        QUrl url( path );

        QUrlQuery query;
        query.addQueryItem( "api_key", apiKeyV3 );
        url.setQuery( query );
        //qDebug() << url.toString();

        if ( !fStopSearching )
        {
            auto reply = sendRequest( QNetworkRequest( url ), ERequestType::eGetImage );
            fImageInfoReplies[reply] = searchResults;
        }
    }

    if ( fSearchInfo->isTVShow() )
    {
        if ( tmdbid != -1 )
        {
            searchTVDetails( searchResults, tmdbid, -1 );
        }
    }
    else
    {
        if ( !multipleResults )
        {
            setBestMatch( searchResults );
        }
    }

    fTopLevelResults.push_back( searchResults );
    return true;
}

void CSearchTMDB::searchTVDetails( std::shared_ptr< STitleInfo > showInfo, int tmdbid, int seasonNum )
{
    QUrl url;
    url.setScheme( "https" );
    url.setHost( "api.themoviedb.org" );

    auto path = QString( "/3/tv/%1" ).arg( tmdbid );
    if ( seasonNum == -1 )
        seasonNum = fSearchInfo->season();
    std::shared_ptr< STitleInfo > seasonInfo;
    if ( seasonNum != -1 )
    {
        path += QString( "/season/%1" ).arg( seasonNum );
        seasonInfo = std::make_shared< STitleInfo >( ETitleInfoType::eTVSeason );
        seasonInfo->fTitle = showInfo->fTitle;
        seasonInfo->fTMDBID = showInfo->fTMDBID;
        seasonInfo->fSeason = QString::number( seasonNum );
        showInfo->fChildren.push_back( seasonInfo );
        seasonInfo->fParent = showInfo;
    }
    url.setPath( path );

    QUrlQuery query;
    query.addQueryItem( "api_key", apiKeyV3 );
    url.setQuery( query );

    if ( !fStopSearching )
    {
        auto reply = sendRequest( QNetworkRequest( url ), seasonInfo ? ERequestType::eSeasonInfo : ERequestType::eTVInfo );
        if ( seasonInfo )
            fSeasonInfoReplies.first[reply] = seasonInfo;
        else
            fTVInfoReplies[reply] = showInfo;
    }
}

// from a /tv/tv_id request
bool CSearchTMDB::loadTVDetails( std::shared_ptr< CNetworkReply > reply )
{
    auto pos = fTVInfoReplies.find( reply );
    if ( pos == fTVInfoReplies.end() )
        return false;

    auto showInfo = ( *pos ).second;
    fTVInfoReplies.erase( pos );
    if ( !showInfo )
        return false;

    auto data = reply->getData();
    auto doc = QJsonDocument::fromJson( data );
    //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    auto numSeasons = doc.object().contains( "number_of_seasons" ) ? doc.object()["number_of_seasons"].toInt() : -1;
    //showInfo->fEpisodeTitle = QString( "%1 Season%2" ).arg( numSeasons ).arg( numSeasons != 1 ? "s" : "" );
    
    auto seasons = doc.object()["seasons"].toArray();
    
    for ( auto &&ii : seasons )// int ii = 0; ii < seasons.count(); ++ii )
    {
        //qDebug().nospace().noquote() << QJsonDocument( ii.toObject() ).toJson( QJsonDocument::Indented );
        searchTVDetails( showInfo, showInfo->fTMDBID.toInt(), ii.toObject()["season_number"].toInt() );
    }
    return true;
} 

bool CSearchTMDB::loadSeasonDetails( std::shared_ptr< CNetworkReply > reply )
{
    auto pos = fSeasonInfoReplies.first.find( reply );
    if ( pos == fSeasonInfoReplies.first.end() )
        return false;
    auto seasonInfo = ( *pos ).second;
    fSeasonInfoReplies.first.erase( pos );
    if ( !seasonInfo )
        return false;

    auto data = reply->getData();
    auto doc = QJsonDocument::fromJson( data );
    //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

    seasonInfo->fEpisodeTitle = doc.object().contains( "name" ) ? doc.object()["name"].toString() : QString();;
    seasonInfo->fSeason = doc.object().contains( "season_number" ) ? QString::number( doc.object()["season_number"].toInt() ) : QString();

    auto episodes = doc.object()["episodes"].toArray();
    seasonInfo->fEpisode = QString( "%1 Episode%2" ).arg( episodes.count() ).arg( episodes.count() == 1 ? "" : "s" );
    seasonInfo->fSeasonTMDBID = doc.object().contains( "id" ) ? QString::number( doc.object()["id"].toInt() ) : QString();

    if ( ( fSearchInfo->episode() == -1 ) && ( fSearchInfo->season() != -1 ) && ( !fBestMatch || ( fBestMatch && isBetterSeasonMatch( seasonInfo, fBestMatch ) ) ) )
    {
        setBestMatch( seasonInfo );
    }
    bool episodeFound = false;
    for ( auto && ii : episodes )
    {
        episodeFound = loadEpisodeDetails( ii.toObject(), seasonInfo ) || episodeFound;
        if ( episodeFound )
            break;
    }
    if ( episodeFound || ( fSearchInfo->episode() == -1 ) )
        fSeasonInfoReplies.second = true;
    else if ( !fSeasonInfoReplies.second.has_value() )
        fSeasonInfoReplies.second = false;

    return true;
}

bool CSearchTMDB::loadEpisodeDetails( const QJsonObject & episodeObj, std::shared_ptr< STitleInfo > seasonInfo )
{
    auto episodeNumber = episodeObj.contains( "episode_number" ) ? episodeObj["episode_number"].toInt() : -1;
    if ( fSearchInfo->episode() != -1 )
    {
        if ( fSearchInfo->episode() != episodeNumber )
            return false;
    }
    auto episodeName = episodeObj.contains( "name" ) ? episodeObj["name"].toString() : QString();
    auto overview = episodeObj.contains( "overview" ) ? episodeObj["overview"].toString() : QString();

    auto episodeInfo = std::make_shared< STitleInfo >( ETitleInfoType::eTVEpisode );
    episodeInfo->fEpisode = episodeNumber == -1 ? QString() : QString::number( episodeNumber );
    episodeInfo->fSeason = episodeObj.contains( "season_number" ) ? QString::number( episodeObj["season_number"].toInt() ) : QString();
    episodeInfo->fEpisodeTitle = episodeName;
    episodeInfo->fDescription = overview;
    episodeInfo->fTMDBID = seasonInfo->fTMDBID;
    episodeInfo->fSeasonTMDBID = seasonInfo->fSeasonTMDBID;
    episodeInfo->fEpisodeTMDBID = episodeObj.contains( "id" ) ? QString::number( episodeObj["id"].toInt() ) : QString();
    episodeInfo->fTitle = seasonInfo->fTitle;
    episodeInfo->fReleaseDate = episodeObj.contains( "air_date" ) ? episodeObj["air_date"].toString() : QString();;

    seasonInfo->fChildren.push_back( episodeInfo );
    if ( ( fSearchInfo->episode() != -1 ) && ( !fBestMatch || ( fBestMatch && isBetterEpisodeMatch( episodeInfo, fBestMatch ) ) ) )
    {
        setBestMatch( episodeInfo );
    }
    return fSearchInfo->episode() != -1;
}

void CSearchTMDB::setBestMatch( std::shared_ptr<STitleInfo> info )
{
    fBestMatch = info;
    if ( fCurrentQueuedSearch.has_value() )
    {
        fQueuedResults[fCurrentQueuedSearch.value().first] = info;
    }
}

bool CSearchTMDB::isBetterSeasonMatch( std::shared_ptr< STitleInfo > lhs, std::shared_ptr< STitleInfo > rhs ) const
{
    if ( fSearchInfo->season() != -1 )
    {
        if ( lhs->fSeason != rhs->fSeason )
        {
            if ( lhs->fSeason == fSearchInfo->season() )
                return true;
            if ( rhs->fSeason == fSearchInfo->season() )
                return false;
        }
    }

    return isBetterTitleMatch( lhs, rhs );
}

bool CSearchTMDB::isBetterTitleMatch( std::shared_ptr<STitleInfo> lhs, std::shared_ptr<STitleInfo> rhs ) const
{
    if ( fSearchInfo->searchName() == lhs->fTitle && ( fSearchInfo->searchName() != rhs->fTitle ) )
        return true;
    return false;
}

// returns true if lhs > rhs
bool CSearchTMDB::isBetterEpisodeMatch( std::shared_ptr< STitleInfo > lhs, std::shared_ptr< STitleInfo > rhs ) const
{
    if ( isBetterSeasonMatch( lhs, rhs ) )
        return true;

    if ( fSearchInfo->episode() != -1 )
    {
        if ( lhs->fEpisode != rhs->fEpisode )
        {
            if ( lhs->fEpisode == fSearchInfo->episode() )
                return true;
            if ( rhs->fEpisode == fSearchInfo->episode() )
                return false;
        }
    }

    return isBetterTitleMatch( lhs, rhs );
}

bool CSearchTMDB::loadImageResults( std::shared_ptr< CNetworkReply > reply )
{
    auto pos = fImageInfoReplies.find( reply );
    if ( pos == fImageInfoReplies.end() )
        return false;
    auto info = ( *pos ).second;
    fImageInfoReplies.erase( pos );
    if ( !info )
        return false;

    auto data = reply->getData();

    QImage img;
    img.loadFromData( data );
    QPixmap pm = QPixmap::fromImage( img );
    //qDebug() << pm.size();

    info->fPixmap = pm;
    return true;
}

