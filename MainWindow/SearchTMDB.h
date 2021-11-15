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
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <memory>

class QNetworkAccessManager;
class QNetworkReply;
class QJsonObject;
class QTreeWidgetItem;
class CButtonEnabler;
struct STitleInfo;
struct SSearchTMDBInfo;

class CSearchTMDB : public QObject
{
    Q_OBJECT
public:
    CSearchTMDB( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::optional< QString > &configuration, QObject *parent = 0 );
    ~CSearchTMDB();

    bool isActive() const;

    std::list< std::shared_ptr< STitleInfo > > getResults() const;
    std::shared_ptr< STitleInfo > bestMatch() const { return fBestMatch; }

    bool searchByName();
    
    void resetResults();

    bool hasError() const { return fErrorMessage.has_value(); }
    QString errorString() const { return fErrorMessage.value(); }

public Q_SLOTS:
    void slotRequestFinished( QNetworkReply *reply );

    void slotGetConfig();
    void slotSearch();

Q_SIGNALS:
    void sigSearchFinished();
    void sigMessage( const QString &msg );
private:
    QString getSearchName() const;
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

    std::optional< QString > fErrorMessage;

    std::optional< QString > fConfiguration;
    int fConfigErrorCount{ 0 };

    bool fStopSearching{ true };
    std::shared_ptr< STitleInfo > fBestMatch;
    std::list< std::shared_ptr< STitleInfo > > fTopLevelResults;
};

#endif 
