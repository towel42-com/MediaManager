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

#ifndef _SELECTTMDB_H
#define _SELECTTMDB_H

#include <QDialog>
#include <QStringList>
#include <QDate>
#include <optional>
#include <memory>

namespace Ui {class CSelectTMDB;};

class QNetworkAccessManager;
class QNetworkReply;
class QJsonObject;
class QTreeWidgetItem;
class CButtonEnabler;
struct STitleInfo;

class CSelectTMDB : public QDialog
{
    Q_OBJECT
public:
    enum EItemType
    {
        eMovie = Qt::UserRole + 1,
        eTVShow,
        eSeason,
        eEpisode
    };
    CSelectTMDB( const QString & searchText, std::shared_ptr< STitleInfo > titleInfo, bool isMovie, QWidget* parent = 0);

    void reset();

    ~CSelectTMDB();

    std::shared_ptr< STitleInfo > getTitleInfo() const;

public Q_SLOTS:
    void slotSearchTextChanged();
    void slotRequestFinished( QNetworkReply *reply );

    void slotGetConfig();
    void slotSelectionChanged();
    void slotByNameChanged();
private:
    [[nodiscard]] QString getText( QTreeWidgetItem *item, int column, bool forceTop=false ) const;
    bool hasConfiguration() const;

    void loadSearchResult();
    void loadConfig();
    void loadImageResults( QNetworkReply *reply );
    void getTVDetails( QTreeWidgetItem *item, int tmdbid, int seasonNum );
    void loadTVDetails( QNetworkReply *reply );
    void loadSeasonDetails( QNetworkReply *reply );
    [[nodiscard]] bool loadEpisodeDetails( const QJsonObject &episodeInfo, QTreeWidgetItem *seasonItem );
    void loadMovieResult();
    [[nodiscard]] bool loadSearchResult( const QJsonObject &resultItem, bool multipleResults );

    void deleteParent( QTreeWidgetItem *item );
    void removeFromMap( std::map< QNetworkReply *, QTreeWidgetItem * > &map, QTreeWidgetItem *item );
    void removeFromMaps( QTreeWidgetItem *item );

    void autoAcceptOnFinished();

    std::unique_ptr< Ui::CSelectTMDB > fImpl;
    QNetworkAccessManager *fManager{ nullptr };
    QNetworkReply *fConfigReply{ nullptr };
    QNetworkReply *fSearchReply{ nullptr };
    QNetworkReply *fGetMovieReply{ nullptr };
    QNetworkReply *fGetTVReply{ nullptr };
    std::optional< QString > fConfiguration;

    std::map< QNetworkReply *, QTreeWidgetItem * > fImageInfoReplies;
    std::map< QNetworkReply *, QTreeWidgetItem * > fTVInfoReplies;
    std::map< QNetworkReply *, QTreeWidgetItem * > fSeasonInfoReplies;

    CButtonEnabler *fButtonEnabler{ nullptr };
    int fConfigErrorCount{ 0 };
    bool fIsMovie{ false };
};

#endif 
