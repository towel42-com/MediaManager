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

#ifndef _SEARCHTMDB_H
#define _SEARCHTMDB_H

#include "NetworkReply.h"

#include <QObject>
#include <QList>
#include <QAuthenticator>
#include <QHash>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QAuthenticator;
class QSslPreSharedKeyAuthenticator;
class QNetworkProxy;
class QSslError;

class QJsonObject;
class QTreeWidgetItem;
class CButtonEnabler;
class QNetworkRequest;

namespace NMediaManager
{
    namespace NCore
    {
        struct STransformResult;
        struct SSearchTMDBInfo;

        class CNetworkReply;
        enum class ERequestType;

        class CSearchTMDB : public QObject
        {
            Q_OBJECT
        public:
            CSearchTMDB( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::optional< QString > & configuration, QObject * parent = 0 );
            ~CSearchTMDB();

            void setSkipImages( bool value ) { fSkipImages = value; }
            bool isActive() const;

            std::list < std::shared_ptr< STransformResult > > getResult( const QString & path ) const; // uses the queued results
            std::list< std::shared_ptr< STransformResult > > getResults() const;
            std::shared_ptr< STransformResult > bestMatch() const;

            bool searchByName();

            void resetResults();

            bool hasError() const { return fErrorMessage.has_value(); }
            QString errorString() const { return fErrorMessage.value(); }

            void addSearch( const QString & filePath, std::shared_ptr< SSearchTMDBInfo > searchInfo );
            void clearSearchCache();

            QString toString() const;

            static QString apiKeyV3();
            static QString apiKeyV4();
        public Q_SLOTS:
            void slotAuthenticationRequired( QNetworkReply * reply, QAuthenticator * authenticator );
            void slotEncrypted( QNetworkReply * reply );
            void slotRequestFinished( QNetworkReply * reply );

            void slotPreSharedKeyAuthenticationRequired( QNetworkReply * reply, QSslPreSharedKeyAuthenticator * authenticator );
            void slotProxyAuthenticationRequired( const QNetworkProxy & proxy, QAuthenticator * authenticator );
            void slotSSlErrors( QNetworkReply * reply, const QList<QSslError> & errors );

            void slotFakeRequestFinished( ERequestType requestType, const QString & url, const QByteArray & cachedReply );

            void slotGetConfig();
            void slotSearch();
            void slotAutoSearch();

        Q_SIGNALS:
            void sigSearchFinished();
            void sigAutoSearchFinished( const QString & path, SSearchTMDBInfo * searchInfo, bool remaining );
            void sigMessage( const QString & msg );
            void sigFakeRequestFinished( ERequestType requestType, const QString & url, const QByteArray & cachedReply );
        private:
            void emitSigFinished();
            void handleRequestFinished( std::shared_ptr< CNetworkReply > networkReply );

            void startAutoSearchTimer();
            std::shared_ptr< CNetworkReply > sendRequest( const QNetworkRequest & request, ERequestType requestType ); // sometimes returns the cache value

            void addRequestType( QNetworkReply * retVal, ERequestType requestType );
            void removeRequestType( std::shared_ptr<CNetworkReply> reply );
            ERequestType getRequestType( QNetworkReply * reply ) const;

            QString getSearchName() const;
            bool hasConfiguration() const;

            bool loadConfig( std::shared_ptr< CNetworkReply > networkReply );
            bool loadSearchResult( std::shared_ptr< CNetworkReply > networkReply );
            bool loadMovieResult( std::shared_ptr< CNetworkReply > networkReply );
            bool loadTVResult( std::shared_ptr< CNetworkReply > networkReply );
            bool loadImageResults( std::shared_ptr< CNetworkReply > reply );
            bool loadTVDetails( std::shared_ptr< CNetworkReply > reply );
            bool loadSeasonDetails( std::shared_ptr< CNetworkReply > reply );

            void searchTVDetails( std::shared_ptr< STransformResult > info, int tmdbid, int seasonNum );

            [[nodiscard]] bool loadSearchResult( const QJsonObject & resultItem );
            [[nodiscard]] bool loadEpisodeDetails( const QJsonObject & episodeInfo, std::shared_ptr< STransformResult > seasonItem );

            void addResult( std::shared_ptr<STransformResult> result );
            void addResultToList( std::list< std::shared_ptr< STransformResult > > & list, std::shared_ptr<STransformResult> result, std::shared_ptr< SSearchTMDBInfo > searchInfo ) const;

            void checkIfStillSearching();

            QNetworkAccessManager * fManager{ nullptr };
            std::shared_ptr< CNetworkReply > fConfigReply;
            std::shared_ptr< CNetworkReply > fSearchReply;
            std::shared_ptr< CNetworkReply > fGetMovieReply;
            std::shared_ptr< CNetworkReply > fGetTVReply;

            std::unordered_map< QString, std::shared_ptr< STransformResult > > fImageInfoReplies;
            std::unordered_map< QString, std::shared_ptr< STransformResult > > fTVInfoReplies;
            std::pair< std::unordered_map< QString, std::shared_ptr< STransformResult > >, std::optional< bool > > fSeasonInfoReplies; // bool means episode found for this round of seasons searchess

            std::shared_ptr< SSearchTMDBInfo > fSearchInfo;
            std::optional< std::pair< QString, std::shared_ptr< SSearchTMDBInfo > > > fCurrentQueuedSearch;
            std::unordered_map< QString, std::list< std::shared_ptr< STransformResult > > > fQueuedResults;
            std::list< std::pair< QString, std::shared_ptr< SSearchTMDBInfo > > > fSearchQueue;
            QTimer * fAutoSearchTimer{ nullptr };

            std::optional< QString > fErrorMessage;

            std::optional< QString > fConfiguration;
            int fConfigErrorCount{ 0 };

            bool fStopSearching{ true };
            bool fSkipImages{ false };
            std::list< std::shared_ptr< STransformResult > > fResults;

            std::unordered_map< QString, QByteArray > fURLResultsCache;
            std::unordered_map< QNetworkReply *, ERequestType > fRequestTypeMap;

        };
    }
}
QDebug operator<<( QDebug debug, const NMediaManager::NCore::CSearchTMDB & searchTMDB );

#endif 
