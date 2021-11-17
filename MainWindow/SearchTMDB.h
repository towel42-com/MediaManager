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

#ifndef _SEARCHTMDB_H
#define _SEARCHTMDB_H

#include <QObject>
#include <QList>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <memory>
#include <QAuthenticator>
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
struct STitleInfo;
struct SSearchTMDBInfo;

enum class ERequestType
{
    eConfig,
    eTVSearch,
    eMovieSearch,
    eGetImage,
    eGetMovie,
    eGetTVShow,
    eSeasonInfo,
    eTVInfo
};

class CSearchTMDB : public QObject
{
    Q_OBJECT
public:
    CSearchTMDB( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::optional< QString > &configuration, QObject *parent = 0 );
    ~CSearchTMDB();

    void setSkipImages( bool value ) { fSkipImages = value; }
    bool isActive() const;

    std::shared_ptr< STitleInfo > getResult( const QString &path ) const;

    std::list< std::shared_ptr< STitleInfo > > getResults() const;
    std::shared_ptr< STitleInfo > bestMatch() const { return fBestMatch; }

    bool searchByName();
    
    void resetResults();

    bool hasError() const { return fErrorMessage.has_value(); }
    QString errorString() const { return fErrorMessage.value(); }

    void addSearch( const QString &filePath, std::shared_ptr< SSearchTMDBInfo > searchInfo );

    QString toString() const;
public Q_SLOTS:
    void slotAuthenticationRequired( QNetworkReply *reply, QAuthenticator *authenticator );
    void slotEncrypted( QNetworkReply *reply );
    void slotRequestFinished( QNetworkReply *reply );
    void slotPreSharedKeyAuthenticationRequired( QNetworkReply *reply, QSslPreSharedKeyAuthenticator *authenticator );
    void slotProxyAuthenticationRequired( const QNetworkProxy &proxy, QAuthenticator *authenticator );
    void slotSSlErrors( QNetworkReply *reply, const QList<QSslError> &errors );

    void slotGetConfig();
    void slotSearch();
    void slotAutoSearch();

Q_SIGNALS:
    void sigSearchFinished();
    void sigAutoSearchFinished( const QString & path);
    void sigMessage( const QString &msg );
private:
    void startAutoSearchTimer();
    QNetworkReply * sendRequest( const QNetworkRequest &request, ERequestType requestType );
    void emitSigFinished();
    QString getSearchName() const;
    bool isBetterSeasonMatch( std::shared_ptr< STitleInfo > lhs, std::shared_ptr< STitleInfo > rhs ) const;

    bool isBetterTitleMatch( std::shared_ptr<STitleInfo> lhs, std::shared_ptr<STitleInfo> rhs ) const;

    bool isBetterEpisodeMatch( std::shared_ptr< STitleInfo > lhs, std::shared_ptr< STitleInfo > rhs ) const;
    bool hasConfiguration() const;

    void loadConfig();
    void loadSearchResult();
    void loadMovieResult();
    void loadTVResult();

    bool loadImageResults( QNetworkReply *reply );
    bool loadTVDetails( QNetworkReply *reply );
    bool loadSeasonDetails( QNetworkReply *reply );

    void searchTVDetails( std::shared_ptr< STitleInfo > info, int tmdbid, int seasonNum );

    [[nodiscard]] bool loadSearchResult( const QJsonObject &resultItem, bool multipleResults );
    [[nodiscard]] bool loadEpisodeDetails( const QJsonObject &episodeInfo, std::shared_ptr< STitleInfo > seasonItem );

    void setBestMatch( std::shared_ptr< STitleInfo > info);

    void checkIfStillSearching();

    QNetworkAccessManager *fManager{ nullptr };
    QNetworkReply *fConfigReply{ nullptr };
    QNetworkReply *fSearchReply{ nullptr };
    QNetworkReply *fGetMovieReply{ nullptr };
    QNetworkReply *fGetTVReply{ nullptr };

    std::unordered_map< QNetworkReply *, std::shared_ptr< STitleInfo > > fImageInfoReplies;
    std::unordered_map< QNetworkReply *, std::shared_ptr< STitleInfo > > fTVInfoReplies;
    std::pair< std::unordered_map< QNetworkReply *, std::shared_ptr< STitleInfo > >, std::optional< bool > > fSeasonInfoReplies; // bool means episode found for this round of seasons searchess

    std::shared_ptr< SSearchTMDBInfo > fSearchInfo;
    std::optional< std::pair< QString, std::shared_ptr< SSearchTMDBInfo > > > fCurrentQueuedSearch;
    std::unordered_map< QString, std::shared_ptr< STitleInfo > > fQueuedResults;
    std::list< std::pair< QString, std::shared_ptr< SSearchTMDBInfo > > > fSearchQueue;
    QTimer *fAutoSearchTimer{ nullptr };

    std::optional< QString > fErrorMessage;

    std::optional< QString > fConfiguration;
    int fConfigErrorCount{ 0 };

    bool fStopSearching{ true };
    bool fSkipImages{ false };
    std::shared_ptr< STitleInfo > fBestMatch;
    std::list< std::shared_ptr< STitleInfo > > fTopLevelResults;
};

QDebug operator<<( QDebug debug, const CSearchTMDB & searchTMDB );

#endif 
