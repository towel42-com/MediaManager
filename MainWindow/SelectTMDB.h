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

struct SSearchTMDBInfo;
class CSearchTMDB;
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

    CSelectTMDB( const QString & searchText, std::shared_ptr< STitleInfo > titleInfo, QWidget* parent = 0);

    ~CSelectTMDB();

    std::shared_ptr< STitleInfo > getTitleInfo() const;

    void setSearchForMovies( bool value, bool init );
    void setExactMatchOnly( bool value, bool init );

public Q_SLOTS:
    void slotSearchTextChanged();

    void slotItemChanged();

    QTreeWidgetItem * getFirstSelected() const;

    void slotItemSelected();
    void slotByNameChanged();
    void slotExactOrForMovieChanged();

    void slotSearchFinished();
    void slotLoadNextResult();
    void slotReset();
Q_SIGNALS:
    void sigStartSearch();
private:
    void updateEnabled();
    void updateByName( bool init );
    void updateFromSearchInfo( std::shared_ptr< SSearchTMDBInfo > searchInfo );

    std::shared_ptr< SSearchTMDBInfo > getSearchInfo();
    void resetHeader();

    void loadResults( std::shared_ptr< STitleInfo > item, QTreeWidgetItem *parent );
    void deleteParent( QTreeWidgetItem *item );

    std::unique_ptr< Ui::CSelectTMDB > fImpl;

    CButtonEnabler *fButtonEnabler{ nullptr };
    CSearchTMDB * fSearchTMDB{ nullptr };
    std::shared_ptr< SSearchTMDBInfo > fSearchInfo;

    bool fLoading{ false };
    bool fStopLoading{ false };
    bool fSearchPending{ false };
    std::shared_ptr< SSearchTMDBInfo > fQueuedSearchInfo;

    std::shared_ptr< STitleInfo > fBestMatch;
    std::list< std::shared_ptr< STitleInfo > > fCurrentResults;
    std::unordered_map< QTreeWidgetItem *, std::shared_ptr< STitleInfo > > fResultTitleInfo;
};

#endif 
