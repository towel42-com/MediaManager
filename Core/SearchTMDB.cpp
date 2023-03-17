// The MIT License( MIT )
//
// Copyright( c ) 2020-2021 Scott Aron Bloom
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
#include "TransformResult.h"
#include "SearchTMDBInfo.h"
#include "NetworkReply.h"

#include "Preferences/Core/Preferences.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImage>
#include <QPixmap>
#include <QDebug>

QDebug operator<<( QDebug debug, const NMediaManager::NCore::CSearchTMDB &searchTMDB )
{
    debug.nospace().noquote() << "(" << searchTMDB.toString() << ")";
    return debug;
}

namespace NMediaManager
{
    namespace NCore
    {
        CSearchTMDB::CSearchTMDB( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::optional< QString > &configuration, QObject *parent ) :
            QObject( parent ),
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
            fResults.clear();
            fRetrievedResults.clear();
            fResults.push_back( std::make_shared< NCore::CTransformResult >( NCore::EMediaType::eNotFoundType ) );
            fQueuedResults.clear();
            fCurrentQueuedSearch = {};
            fSearchQueue.clear();
            fSearchPageNumber = { -1, false };
            fAutoSearchTimer.second = false;
        }

        void CSearchTMDB::addSearch( const QString &filePath, std::shared_ptr< SSearchTMDBInfo > searchInfo )
        {
            //qDebug() << "AddSearch: " << filePath << searchInfo->searchName();

            fSearchQueue.emplace_back( filePath, searchInfo );
            startAutoSearchTimer();

            //qDebug() << "AddSearch After: SearchTMBD" << *this;
        }

        void CSearchTMDB::clearSearchCache()
        {
            fSearchQueue.clear();
            //fURLResultsCache.clear();
        }

        QString CSearchTMDB::toString() const
        {
            QString retVal = QString( "CSearchTMDB(Manager: 0x%1 " ).arg( reinterpret_cast< uintptr_t >( fManager ), 0, 16 )
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

            retVal += ( fSearchInfo ? fSearchInfo->toString( true ) : "nullptr" ) + " ";
            retVal += "CurrentQueuedSearch(";
            if ( fCurrentQueuedSearch.has_value() )
            {
                retVal += fCurrentQueuedSearch.value().first + " " + fCurrentQueuedSearch.value().second->toString( true );
            }
            else
                retVal += "nullptr";
            retVal += ") ";

            retVal += QString( "QueuedResults( %1 -" ).arg( fQueuedResults.size() );
            for ( auto &&ii : fQueuedResults )
            {
                retVal += QString( "(%1 - (" ).arg( ii.first );
                bool first = true;
                for ( auto &&jj : ii.second )
                {
                    if ( !first )
                        retVal += " ";
                    retVal += jj->toString( true );
                }
                retVal += ")";
            }
            retVal += ") ";

            retVal += QString( "SearchQueue( %1 -" ).arg( fSearchQueue.size() );
            for ( auto &&ii : fSearchQueue )
            {
                retVal += QString( "(%1 - %2)" ).arg( ii.first ).arg( ii.second->toString( true ) );
            }
            retVal += ") ";
            retVal += QString( "AutoSearchTimer isActive? %1 " ).arg( fAutoSearchTimer.first && fAutoSearchTimer.first->isActive() );
            retVal += QString( "AutoSearch Enabled? %1 " ).arg( fAutoSearchTimer.second );

            retVal += QString( "Error Message: %1 " ).arg( fErrorMessage.has_value() ? fErrorMessage.value() : QString() );
            retVal += QString( "Configuration: %1 ErrorCount: %2 " ).arg( fConfiguration.has_value() ? fConfiguration.value() : QString( "<notset>" ) ).arg( fConfigErrorCount );
            ;

            retVal += QString( "StopSearching: %1 SkipImages: %2 " ).arg( fStopSearching ).arg( fSkipImages );
            ;
            //retVal += QString( "BestMatch: %1" ).arg( fResults.first ? fResults.first->toString() : QString() );
            retVal += QString( "Results( %1 -" ).arg( fResults.size() );
            for ( auto &&ii : fResults )
            {
                retVal += QString( "(%1) " ).arg( ii->toString( true ) );
            }
            retVal += ") ";
            retVal += ")";
            return retVal;
        }

        void CSearchTMDB::startAutoSearchTimer()
        {
            if ( !fAutoSearchTimer.first )
            {
                fAutoSearchTimer.first = new QTimer( this );
                fAutoSearchTimer.first->setInterval( 100 );
                fAutoSearchTimer.first->setSingleShot( true );
                connect( fAutoSearchTimer.first, &QTimer::timeout, this, &CSearchTMDB::slotAutoSearch );
            }
            fAutoSearchTimer.second = true;
            fAutoSearchTimer.first->stop();
            fAutoSearchTimer.first->start();
        }

        const QString kApiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
        QString CSearchTMDB::apiKeyV3()
        {
            return kApiKeyV3;
        }

        const QString kApiKeyV4 =
            "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";
        QString CSearchTMDB::apiKeyV4()
        {
            return kApiKeyV4;
        }

        void CSearchTMDB::slotAuthenticationRequired( QNetworkReply * /*reply*/, QAuthenticator * /*authenticator*/ )
        {
            //qDebug() << "slotAuthenticationRequired:" << reply << reply->url().toString() << authenticator;
        }

        void CSearchTMDB::slotEncrypted( QNetworkReply * /*reply*/ )
        {
            //qDebug() << "slotEncrypted:" << reply << reply->url().toString();
        }

        std::shared_ptr< CNetworkReply > CSearchTMDB::sendRequest( const QNetworkRequest &request, ERequestType requestType )
        {
            auto key = CNetworkReply::key( request, requestType );
            //qDebug() << "Sending request: Key" << key;
            auto pos = fURLResultsCache.find( key );
            if ( pos != fURLResultsCache.end() )
            {
                //qDebug().noquote().nospace() << "Cached Result " << NCore::toString( requestType ) << " request:" << (*pos).first;
                QTimer::singleShot( 0, [ this, requestType, pos ]() { emit sigFakeRequestFinished( requestType, ( *pos ).first, ( *pos ).second ); } );
                return std::make_shared< CNetworkReply >( requestType, ( *pos ).first, ( *pos ).second );
            }
            auto retVal = fManager->get( request );
            //qDebug().noquote().nospace() << "Sending " << ::toString( requestType ) << " request:" << request.url() << "Reply: 0x" << Qt::hex << reinterpret_cast< uint64_t >( retVal );
            //qDebug() << *this;
            addRequestType( retVal, requestType );
            return std::make_shared< CNetworkReply >( requestType, retVal );
        }

        void CSearchTMDB::addRequestType( QNetworkReply *retVal, ERequestType requestType )
        {
            fRequestTypeMap[ retVal ] = requestType;
        }

        void CSearchTMDB::removeRequestType( std::shared_ptr< CNetworkReply > reply )
        {
            if ( !reply )
                return;

            auto pos = fRequestTypeMap.find( reply->getNetworkReply() );
            fRequestTypeMap.erase( pos );
        }

        ERequestType CSearchTMDB::getRequestType( QNetworkReply *reply ) const
        {
            auto pos = fRequestTypeMap.find( reply );
            Q_ASSERT( pos != fRequestTypeMap.end() );
            auto requestType = ERequestType::eUnknownRequest;
            if ( pos != fRequestTypeMap.end() )
            {
                requestType = ( *pos ).second;
            }
            return requestType;
        }

        void CSearchTMDB::slotFakeRequestFinished( ERequestType requestType, const QString &url, const QByteArray &cachedReply )
        {
            handleRequestFinished( std::make_shared< CNetworkReply >( requestType, url, cachedReply ) );
        }

        void CSearchTMDB::slotRequestFinished( QNetworkReply *reply )
        {
            handleRequestFinished( std::make_shared< CNetworkReply >( getRequestType( reply ), reply ) );
        }

        void CSearchTMDB::handleRequestFinished( std::shared_ptr< CNetworkReply > reply )
        {
            Q_ASSERT( reply && reply->isValid() );
            //qDebug() << "Reply Finished:" << Qt::hex << *reply;
            //qDebug() << "Before" << *this;
            if ( reply && reply->hasError() )   // replys with an error do not get cached
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
                    fErrorMessage = tr( "Could not find Movie with TMDBID: <b>%1</b>" ).arg( fGetMovieReply->tmdbID() );
                    fGetMovieReply.reset();
                    emitSigFinished();
                    return;
                }
                else if ( reply->isReply( fGetTVReply ) )
                {
                    fErrorMessage = tr( "Could not find TV Show with TMDBID: <b>%1</b>" ).arg( fGetTVReply->tmdbID() );
                    fGetTVReply.reset();
                    emitSigFinished();
                    return;
                }
                else
                {
                    if ( fImageInfoReplies.find( reply->key() ) != fImageInfoReplies.end() )
                    {
                        title = tr( "Could not get image(s)" );

                        auto pos = fImageInfoReplies.find( reply->key() );
                        ( *pos ).second->setPixmapPath( QString() );
                        fImageInfoReplies.erase( pos );
                    }
                    else if ( fTVInfoReplies.find( reply->key() ) != fTVInfoReplies.end() )
                    {
                        title = tr( "Could not find TV Details information" );
                        auto pos = fTVInfoReplies.find( reply->key() );
                        fTVInfoReplies.erase( pos );
                    }
                    else if ( fSeasonInfoReplies.first.find( reply->key() ) != fSeasonInfoReplies.first.end() )
                    {
                        title = tr( "Could not find Season/Episode information" );
                        auto pos = fSeasonInfoReplies.first.find( reply->key() );
                        auto info = ( *pos ).second;
                        fSeasonInfoReplies.first.erase( pos );
                        auto parent = info->parent().lock();
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
                {
                    checkIfStillSearching();
                    return;
                }
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
            else if ( !reply->isCached() )
            {
                fURLResultsCache[ reply->key() ] = reply->getData();
            }

            if ( !reply->isCached() )
                removeRequestType( reply );

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

        void CSearchTMDB::slotSSlErrors( QNetworkReply * /*reply*/, const QList< QSslError > & /*errors*/ )
        {
            //qDebug() << "slotSSlErrors: 0x" << Qt::hex << reply << errors;
        }

        QString CSearchTMDB::getSearchName() const
        {
            return fSearchInfo ? ( fSearchInfo->isTVMedia() ? tr( "TV Show" ) : tr( "Movie" ) ) : QString();
        }

        void CSearchTMDB::checkIfStillSearching()
        {
            if ( isActive() )
                return;
            if ( fSearchPageNumber.first != -1 )
                return;
            if ( fSearchInfo && fSeasonInfoReplies.second.has_value() && !fSeasonInfoReplies.second.value() )
            {
                fErrorMessage = QString( "Could not find episode '%1' for TV show '%2'" ).arg( fSearchInfo->episodeString( false ) ).arg( fSearchInfo->searchName() );
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
                emit sigAutoSearchFinished( path, fSearchInfo.get(), !fSearchQueue.empty() );
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

        std::list< std::shared_ptr< CTransformResult > > CSearchTMDB::getResult( const QString &path ) const
        {
            auto pos = fQueuedResults.find( path );
            if ( pos == fQueuedResults.end() )
                return {};
            return ( *pos ).second;
        }

        std::list< std::shared_ptr< CTransformResult > > CSearchTMDB::getResults() const
        {
            if ( fRetrievedResults.empty() )
                return fResults;
            else
            {
                std::list< std::shared_ptr< CTransformResult > > retVal;
                for ( auto &&ii = fResults.begin(); ii != fResults.end(); ++ii )
                {
                    if ( fRetrievedResults.find( *ii ) != fRetrievedResults.end() )
                        continue;

                    retVal.emplace_back( *ii );
                }

                return retVal;
            }
        }

        std::list< std::shared_ptr< CTransformResult > > CSearchTMDB::getPartialResults()
        {
            std::list< std::shared_ptr< CTransformResult > > retVal;
            for ( auto &&ii = fResults.begin(); ii != fResults.end(); ++ii )
            {
                if ( ( *ii )->isNotFoundResult() )
                    continue;
                if ( !( *ii )->pixmapFinished() )
                    continue;
                if ( fRetrievedResults.find( *ii ) != fRetrievedResults.end() )
                    continue;

                retVal.emplace_back( *ii );
                fRetrievedResults.insert( *ii );
            }

            return retVal;
        }

        std::shared_ptr< NMediaManager::NCore::CTransformResult > CSearchTMDB::bestMatch() const
        {
            auto pos = fResults.begin();
            while ( pos != fResults.end() )
            {
                if ( !( *pos )->isNotFoundResult() )
                    return ( *pos );
                pos++;
            }
            return {};
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
            query.addQueryItem( "api_key", apiKeyV3() );
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
            auto images = doc.object()[ "images" ].toObject();
            if ( !images.contains( "poster_sizes" ) || !images[ "poster_sizes" ].isArray() )
                return false;
            auto posterSizes = images[ "poster_sizes" ].toArray();

            QString posterSize = "original";
            //for( int ii = 0; ii < posterSizes.size(); ++ii )
            //{
            //    auto curr = posterSizes[ii].toString();
            //}
            auto posterURL = images.contains( "secure_base_url" ) ? images[ "secure_base_url" ].toString() : QString();
            if ( !posterURL.isEmpty() )
                fConfiguration = posterURL + posterSize;
            return true;
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
                fQueuedResults[ fCurrentQueuedSearch.value().first ].push_back( std::make_shared< NCore::CTransformResult >( NCore::EMediaType::eNotFoundType ) );
                fSearchQueue.pop_front();
                fSearchInfo = fCurrentQueuedSearch.value().second;
            }

            fSearchInfo->setPageNumber( fSearchPageNumber.first );
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
            bool found = false;
            if ( doc.object().contains( "results" ) )
            {
                auto results = doc.object()[ "results" ].toArray();
                auto numResults = results.count();
                for ( int ii = 0; ii < numResults; ++ii )
                {
                    found = loadSearchResult( results[ ii ].toObject() ) || found;
                }
            }

            bool continueSearch = false;
            if ( doc.object().contains( "page" ) && doc.object().contains( "total_pages" ) )
            {
                auto page = doc.object()[ "page" ].toInt();
                auto totalPages = doc.object()[ "total_pages" ].toInt();
                fSearchPageNumber.first = -1;
                if ( ( page < totalPages ) && !fAutoSearchTimer.second )
                {
                    auto maxPages = NMediaManager::NPreferences::NCore::CPreferences::instance()->getNumSearchPages();
                    if ( ( maxPages == -1 ) || ( page < maxPages ) )
                    {
                        fSearchPageNumber.first = page + 1;
                        fSearchPageNumber.second = fSearchPageNumber.second || found;
                        continueSearch = true;
                    }
                }
            }

            if ( !found && !continueSearch && !fSearchPageNumber.second )
            {
                fErrorMessage = tr( "Could not find %1 - No results found that match search criteria" ).arg( getSearchName() );
                return false;
            }
            if ( continueSearch )
            {
                if ( ( fSearchPageNumber.first != -1 ) && !fStopSearching )
                    QTimer::singleShot( 200, this, &CSearchTMDB::slotSearch );
                if ( ( fRetrievedResults.empty() || ( ( fResults.size() % 10 ) == 0 ) ) && !fAutoSearchTimer.second )
                    emit sigAutoSearchPartialFinished();
            }
            return found || fSearchPageNumber.second;
        }

        bool CSearchTMDB::loadMovieResult( std::shared_ptr< CNetworkReply > reply )
        {
            if ( !fGetMovieReply || !reply->isType( ERequestType::eGetMovie ) )
                return false;

            auto data = reply->getData();
            auto tmdbid = reply->tmdbID();
            fGetMovieReply.reset();

            auto doc = QJsonDocument::fromJson( data );
            //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );
            if ( !loadSearchResult( doc.object() ) )
            {
                auto foundTMDBID = doc.object().contains( "id" ) ? doc.object()[ "id" ].toInt() : -1;

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
            auto tmdbid = reply->tmdbID();
            fGetTVReply.reset();

            auto doc = QJsonDocument::fromJson( data );
            //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

            if ( !loadSearchResult( doc.object() ) )
            {
                auto foundTMDBID = doc.object().contains( "id" ) ? doc.object()[ "id" ].toInt() : -1;

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
        bool CSearchTMDB::loadSearchResult( const QJsonObject &resultItem )
        {
            //qDebug().nospace().noquote() << QJsonDocument( resultItem ).toJson( QJsonDocument::Indented );

            if ( !fSearchInfo )
                return false;

            auto tmdbid = resultItem.contains( "id" ) ? resultItem[ "id" ].toInt() : -1;
            auto desc = resultItem.contains( "overview" ) ? resultItem[ "overview" ].toString() : QString();
            QString title;
            if ( fSearchInfo->isTVMedia() )
                title = resultItem.contains( "name" ) ? resultItem[ "name" ].toString() : QString();
            else
                title = resultItem.contains( "title" ) ? resultItem[ "title" ].toString() : QString();
            auto releaseDate = resultItem[ "release_date" ].toString();
            auto firstAirDate = resultItem[ "first_air_date" ].toString();

            auto posterPath = resultItem.contains( "poster_path" ) ? resultItem[ "poster_path" ].toString() : QString();

            if ( !fSearchInfo->isMatch( releaseDate, tmdbid, title ) && !fSearchInfo->isMatch( firstAirDate, tmdbid, title ) )
                return false;

            auto searchResult = std::make_shared< CTransformResult >( fSearchInfo->isTVMedia() ? EMediaType::eTVShow : EMediaType::eMovie );   // movie or TV show
            searchResult->setDescription( desc );
            searchResult->setMovieReleaseDate( releaseDate );
            searchResult->setShowFirstAirDate( firstAirDate );
            searchResult->setTitle( title );
            searchResult->setTMDBID( QString::number( tmdbid ) );
            searchResult->setExtraInfo( fSearchInfo->getExtendedInfo() );

            if ( resultItem.contains( "number_of_seasons" ) )
            {
                searchResult->setSeason( QString( "%1 Season%2" ).arg( resultItem[ "number_of_seasons" ].toInt() ).arg( resultItem[ "number_of_seasons" ].toInt() == 1 ? "" : "s" ) );
            }
            if ( resultItem.contains( "number_of_episodes" ) )
            {
                searchResult->setEpisode( QString( "%1 Episode%2" ).arg( resultItem[ "number_of_episodes" ].toInt() ).arg( resultItem[ "number_of_episodes" ].toInt() == 1 ? "" : "s" ) );
            }
            if ( !posterPath.isEmpty() && hasConfiguration() && !fSkipImages )
            {
                auto path = fConfiguration.value() + posterPath;
                QUrl url( path );

                QUrlQuery query;
                query.addQueryItem( "api_key", apiKeyV3() );
                url.setQuery( query );
                //qDebug() << url.toString();

                if ( !fStopSearching )
                {
                    auto reply = sendRequest( QNetworkRequest( url ), ERequestType::eGetImage );
                    fImageInfoReplies[ reply->key() ] = searchResult;
                    searchResult->setPixmap( path );
                }
            }

            if ( fSearchInfo->isTVMedia() && ( tmdbid != -1 ) )
            {
                searchTVDetails( searchResult, tmdbid, -1 );
            }

            addResult( searchResult );

            return true;
        }

        void CSearchTMDB::searchTVDetails( std::shared_ptr< CTransformResult > showInfo, int tmdbid, int seasonNum )
        {
            QUrl url;
            url.setScheme( "https" );
            url.setHost( "api.themoviedb.org" );

            auto path = QString( "/3/tv/%1" ).arg( tmdbid );
            bool hasEpisode = fSearchInfo->hasEpisodes();
            if ( seasonNum == -1 )
                seasonNum = fSearchInfo->season();
            if ( seasonNum == -1 && fSearchInfo->hasEpisodes() )
            {
                seasonNum = 1;
                fSearchInfo->setSeason( 1 );
            }
            std::shared_ptr< CTransformResult > seasonInfo;
            if ( seasonNum != -1 )
            {
                path += QString( "/season/%1" ).arg( seasonNum );
                seasonInfo = std::make_shared< CTransformResult >( EMediaType::eTVSeason );
                seasonInfo->setTitle( showInfo->title() );
                seasonInfo->setTMDBID( showInfo->tmdbID() );
                seasonInfo->setSeason( QString::number( seasonNum ) );
                seasonInfo->setSeasonOnly( true );
                showInfo->addChild( seasonInfo );
                seasonInfo->setParent( showInfo );
            }
            url.setPath( path );

            QUrlQuery query;
            query.addQueryItem( "api_key", apiKeyV3() );
            url.setQuery( query );

            if ( !fStopSearching )
            {
                auto reply = sendRequest( QNetworkRequest( url ), seasonInfo ? ERequestType::eSeasonInfo : ERequestType::eTVInfo );
                if ( seasonInfo )
                    fSeasonInfoReplies.first[ reply->key() ] = seasonInfo;
                else
                    fTVInfoReplies[ reply->key() ] = showInfo;
            }
        }

        // from a /tv/tv_id request
        bool CSearchTMDB::loadTVDetails( std::shared_ptr< CNetworkReply > reply )
        {
            auto pos = fTVInfoReplies.find( reply->key() );
            if ( pos == fTVInfoReplies.end() )
                return false;

            auto showInfo = ( *pos ).second;
            fTVInfoReplies.erase( pos );
            if ( !showInfo )
                return false;

            auto data = reply->getData();
            auto doc = QJsonDocument::fromJson( data );
            //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

            auto numSeasons = doc.object().contains( "number_of_seasons" ) ? doc.object()[ "number_of_seasons" ].toInt() : -1;
            //showInfo->fSubTitle = QString( "%1 Season%2" ).arg( numSeasons ).arg( numSeasons != 1 ? "s" : "" );

            auto seasons = doc.object()[ "seasons" ].toArray();

            for ( auto &&ii : seasons )   // int ii = 0; ii < seasons.count(); ++ii )
            {
                //qDebug().nospace().noquote() << QJsonDocument( ii.toObject() ).toJson( QJsonDocument::Indented );
                searchTVDetails( showInfo, showInfo->tmdbID().toInt(), ii.toObject()[ "season_number" ].toInt() );
            }
            return true;
        }

        bool CSearchTMDB::loadSeasonDetails( std::shared_ptr< CNetworkReply > reply )
        {
            auto pos = fSeasonInfoReplies.first.find( reply->key() );
            if ( pos == fSeasonInfoReplies.first.end() )
                return false;
            auto seasonInfo = ( *pos ).second;
            fSeasonInfoReplies.first.erase( pos );
            if ( !seasonInfo )
                return false;

            auto data = reply->getData();
            auto doc = QJsonDocument::fromJson( data );
            //qDebug().nospace().noquote() << doc.toJson( QJsonDocument::Indented );

            auto seasonStartDate = doc.object()[ "air_date" ].toString();
            {
                auto parentPtr = seasonInfo->parent().lock();
                if ( parentPtr )
                {
                    seasonInfo->setShowFirstAirDate( parentPtr->getShowFirstAirDate() );   // should go to the parent
                    seasonInfo->setShowTMDBID( parentPtr->getTMDBID() );
                }
            }
            seasonInfo->setSeasonStartDate( seasonStartDate );
            seasonInfo->setSubTitle( doc.object().contains( "name" ) ? doc.object()[ "name" ].toString() : QString() );
            seasonInfo->setSeason( doc.object().contains( "season_number" ) ? QString::number( doc.object()[ "season_number" ].toInt() ) : QString() );
            seasonInfo->setSeasonOnly( true );

            auto episodes = doc.object()[ "episodes" ].toArray();
            seasonInfo->setEpisode( QString( "%1 Episode%2" ).arg( episodes.count() ).arg( episodes.count() == 1 ? "" : "s" ) );
            seasonInfo->setSeasonTMDBID( doc.object().contains( "id" ) ? QString::number( doc.object()[ "id" ].toInt() ) : QString() );

            // season match
            if ( !fSearchInfo->hasEpisodes() && ( fSearchInfo->season() != -1 ) )
            {
                addResult( seasonInfo );
            }

            auto searchEpisodes = fSearchInfo->episodes();
            bool anEpisodeNotFound = false;
            for ( auto &&ii : searchEpisodes )
            {
                bool episodeFound = false;
                for ( auto &&jj : episodes )
                {
                    if ( loadEpisodeDetails( ii, jj.toObject(), seasonInfo ) )
                    {
                        episodeFound = true;
                        break;
                    }
                }
                if ( !episodeFound )
                {
                    anEpisodeNotFound = true;
                    break;
                }
            }
            if ( !anEpisodeNotFound || !fSearchInfo->hasEpisodes() )
                fSeasonInfoReplies.second = true;
            else if ( !fSeasonInfoReplies.second.has_value() )
                fSeasonInfoReplies.second = false;

            return true;
        }

        bool CSearchTMDB::loadEpisodeDetails( int episodeNum, const QJsonObject &episodeObj, std::shared_ptr< CTransformResult > seasonInfo )
        {
            auto episodeNumber = episodeObj.contains( "episode_number" ) ? episodeObj[ "episode_number" ].toInt() : -1;

            if ( episodeNumber == -1 )
                return false;

            if ( episodeNum != episodeNumber )
                return false;

            auto episodeName = episodeObj.contains( "name" ) ? episodeObj[ "name" ].toString() : QString();
            auto overview = episodeObj.contains( "overview" ) ? episodeObj[ "overview" ].toString() : QString();

            auto episodeInfo = std::make_shared< CTransformResult >( EMediaType::eTVEpisode );
            episodeInfo->setEpisode( episodeNumber == -1 ? QString() : QString::number( episodeNumber ) );
            episodeInfo->setSeason( episodeObj.contains( "season_number" ) ? QString::number( episodeObj[ "season_number" ].toInt() ) : QString() );
            episodeInfo->setSubTitle( episodeName );
            episodeInfo->setDescription( overview );
            episodeInfo->setTMDBID( seasonInfo->tmdbID() );
            episodeInfo->setSeasonTMDBID( seasonInfo->seasonTMDBID() );
            episodeInfo->setEpisodeTMDBID( episodeObj.contains( "id" ) ? QString::number( episodeObj[ "id" ].toInt() ) : QString() );
            episodeInfo->setShowTMDBID( seasonInfo->showTMDBID() );
            episodeInfo->setTitle( seasonInfo->title() );
            episodeInfo->setShowFirstAirDate( seasonInfo->getShowFirstAirDate() );
            episodeInfo->setSeasonStartDate( seasonInfo->getSeasonStartDate() );
            episodeInfo->setEpisodeAirDate( episodeObj[ "air_date" ].toString() );

            seasonInfo->addChild( episodeInfo );
            if ( fSearchInfo->hasEpisodes() )
            {
                addResult( episodeInfo );
                return true;
            }
            return false;
        }

        void CSearchTMDB::addResultToList( std::list< std::shared_ptr< CTransformResult > > &list, std::shared_ptr< CTransformResult > result, std::shared_ptr< SSearchTMDBInfo > searchInfo ) const
        {
            if ( ( list.size() == 1 ) && ( list.front()->isNotFoundResult() ) )
            {
                list.pop_front();
            }

            auto pos = list.begin();
            for ( ; pos != list.end(); ++pos )
            {
                if ( result->isBetterMatch( searchInfo, ( *pos ) ) )
                    break;
            }
            list.insert( pos, result );
        }

        void CSearchTMDB::addResult( std::shared_ptr< CTransformResult > result )   //, TBettterMatchFunc isBetterMatchFunc )
        {
            if ( fCurrentQueuedSearch.has_value() )
            {
                addResultToList( fQueuedResults[ fCurrentQueuedSearch.value().first ], result, fCurrentQueuedSearch.value().second );
            }

            addResultToList( fResults, result, fSearchInfo );
        }

        bool CSearchTMDB::loadImageResults( std::shared_ptr< CNetworkReply > reply )
        {
            auto pos = fImageInfoReplies.find( reply->key() );
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

            info->setPixmap( pm );
            return true;
        }

    }
}