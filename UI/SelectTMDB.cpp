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

#include "SelectTMDB.h"
#include "ui_SelectTMDB.h"
#include "Core/DirModel.h"
#include "Core/SearchResult.h"
#include "Core/SearchTMDB.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/Preferences.h"

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/QtUtils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QPushButton>
#include <QMessageBox>

namespace NMediaManager
{
    namespace NUi
    {
        CSelectTMDB::CSelectTMDB( const QString &text, std::shared_ptr< NCore::SSearchResult > searchResult, QWidget *parent )
            : QDialog( parent ),
            fImpl( new Ui::CSelectTMDB )
        {
            fImpl->setupUi( this );

            fSearchInfo = std::make_shared< NCore::SSearchTMDBInfo >( text, searchResult );
            fSearchTMDB = new NCore::CSearchTMDB( fSearchInfo, std::optional<QString>(), this );

            connect( this, &CSelectTMDB::sigStartSearch, fSearchTMDB, &NCore::CSearchTMDB::slotSearch );
            connect( fSearchTMDB, &NCore::CSearchTMDB::sigSearchFinished, this, &CSelectTMDB::slotSearchFinished );

            fImpl->resultExtraInfo->setText( searchResult ? searchResult->fExtraInfo : QString() );
            fImpl->resultEpisodeTitle->setText( fSearchInfo->episodeTitle() );

            fImpl->searchName->setDelay( 1000 );
            fImpl->searchSeason->setDelay( 1000 );
            fImpl->searchEpisode->setDelay( 1000 );
            fImpl->searchReleaseYear->setDelay( 1000 );
            fImpl->searchTMDBID->setDelay( 1000 );

            updateFromSearchInfo( fSearchInfo );
            slotReset();

            connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
            connect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );
            connect( fImpl->searchForTVShows, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );

            connect( fImpl->searchName, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchTextChanged );
            connect( fImpl->searchSeason, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            connect( fImpl->searchEpisode, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            connect( fImpl->searchReleaseYear, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            connect( fImpl->searchTMDBID, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            connect( fImpl->results->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CSelectTMDB::slotItemChanged );
            connect( fImpl->results, &QTreeWidget::itemDoubleClicked, this, &CSelectTMDB::slotAcceptItem );

            fImpl->results->setIconSize( QSize( 128, 128 ) );

            updateByName( true );
            updateEnabled();

            QTimer::singleShot( 0, this, &CSelectTMDB::slotSearchCriteriaChanged );
        }

        void CSelectTMDB::updateFromSearchInfo( std::shared_ptr< NCore::SSearchTMDBInfo > searchInfo )
        {
            disconnect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
            disconnect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );
            disconnect( fImpl->searchForTVShows, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );

            disconnect( fImpl->searchName, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchTextChanged);
            disconnect( fImpl->searchSeason, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            disconnect( fImpl->searchEpisode, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            disconnect( fImpl->searchReleaseYear, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            disconnect( fImpl->searchTMDBID, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );

            fImpl->resultExtraInfo->setText(searchInfo->getExtendedInfo());

            fImpl->searchName->setText( fPrevSearchName = searchInfo->searchName() );
            fImpl->searchSeason->setValue( searchInfo->season() );
            fImpl->searchEpisode->setValue( searchInfo->episode() );
            fImpl->searchReleaseYear->setText( searchInfo->releaseDateString() );
            fImpl->searchTMDBID->setText( searchInfo->tmdbIDString() );
            fImpl->byName->setChecked( fSearchTMDB->searchByName() );
            fImpl->byTMDBID->setChecked( !fSearchTMDB->searchByName() );
            fImpl->exactMatchesOnly->setChecked( searchInfo->exactMatchOnly() );
            fImpl->searchForTVShows->setChecked( searchInfo->isTVShow() );
            updateEnabled();

            connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotExactOrForTVShowsChanged);
            connect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );
            connect( fImpl->searchForTVShows, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );

            connect( fImpl->searchName, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchTextChanged);
            connect( fImpl->searchSeason, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            connect( fImpl->searchEpisode, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            connect( fImpl->searchReleaseYear, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            connect( fImpl->searchTMDBID, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
        }

        void CSelectTMDB::slotReset()
        {
            if ( fLoading )
            {
                fStopLoading = true;
                fSearchTMDB->resetResults();
                QTimer::singleShot( 500, this, &CSelectTMDB::slotReset );
                return;
            }
            fStopLoading = false;
            fSearchTMDB->resetResults();
            fImpl->results->clear();
            fSearchResultMap.clear();
            resetHeader();
        }

        void CSelectTMDB::resetHeader()
        {
            fImpl->results->setColumnCount( 0 );
            if ( fImpl->searchForTVShows->isChecked() )
                fImpl->results->setHeaderLabels( QStringList() << "Title" << "TMDB ID" << "Season" << "Episode" << "Air Date" << "Episode Title" << "Desc" );
            else
                fImpl->results->setHeaderLabels( QStringList() << "Title" << "TMDB ID" << "Release Date" << "Desc" );
        }

        CSelectTMDB::~CSelectTMDB()
        {
        }

        const QString apiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
        const QString apiKeyV4 = "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";

        void CSelectTMDB::deleteParent( QTreeWidgetItem *item )
        {
            if ( !item )
                return;
            auto parent = item->parent();
            if ( item->type() == EItemType::eTVShow )
            {
                delete item;
                deleteParent( parent );
            }
            else if ( item->type() == EItemType::eSeason )
            {
                if ( item->childCount() == 0 )
                {
                    delete item;
                    deleteParent( parent );
                }
            }
            else if ( item->type() == EItemType::eTVShow )
            {
                if ( item->childCount() == 0 )
                {
                    delete item;
                }
            }
        }

        void CSelectTMDB::slotSearchFinished()
        {
            if ( fSearchTMDB->hasError() )
            {
                QMessageBox::information( this, tr( "Error searching themoviedb" ), fSearchTMDB->errorString() );
                return;
            }

            auto currResults = fSearchTMDB->getResults();
            fCurrentResults.insert( fCurrentResults.end(), currResults.begin(), currResults.end() );
            fBestMatch = fSearchTMDB->bestMatch();

            fLoading = true;
            //qDebug() << QDateTime::currentDateTime().toString() << "launching slotLoadNextResult" << fCurrentResults.size();
            QTimer::singleShot( 0, this, &CSelectTMDB::slotLoadNextResult );
        }

        void CSelectTMDB::slotLoadNextResult()
        {
            //qDebug() << QDateTime::currentDateTime().toString() << "handling slotLoadNextResult" << fCurrentResults.size();
            if ( fStopLoading )
                fCurrentResults.clear();

            if ( fCurrentResults.empty() )
            {
                delete fButtonEnabler;
                fButtonEnabler = new NSABUtils::CButtonEnabler( fImpl->results, fImpl->buttonBox->button( QDialogButtonBox::Ok ) );

                fLoading = false;
                fSearchTMDB->resetResults();

                if ( fQueuedSearchInfo )
                {
                    fSearchInfo = fQueuedSearchInfo;
                    fQueuedSearchInfo.reset();
                    QTimer::singleShot( 500, this, &CSelectTMDB::slotSearchCriteriaChanged );
                }

                auto childNode = getSingleMatchingItem( nullptr );
                if ( childNode && childNode->isSelected() )
                    slotAcceptItem();
                return;
            }

            auto curr = fCurrentResults.front();
            fCurrentResults.pop_front();
            loadResults( curr, nullptr );

            //qDebug() << QDateTime::currentDateTime().toString() << "Launching slotLoadNextResult" << fCurrentResults.size();
            QTimer::singleShot( 0, this, &CSelectTMDB::slotLoadNextResult );
        }

        bool CSelectTMDB::isMatchingItem( QTreeWidgetItem *item ) const
        {
            if ( !fSearchInfo )
                return false;

            auto pos = fSearchResultMap.find( item );
            if ( pos == fSearchResultMap.end() )
                return false;
            return fSearchInfo && fSearchInfo->isMatch( ( *pos ).second );
        }

        // childless items OR if its a season search, and its a season node
        std::list < QTreeWidgetItem * > CSelectTMDB::getMatchingItems( QTreeWidgetItem *parentItem ) const
        {
            std::list < QTreeWidgetItem * > retVal;

            auto childCount = parentItem ? parentItem->childCount() : fImpl->results->topLevelItemCount();
            for ( auto ii = 0; ii < childCount; ++ii )
            {
                auto child = parentItem ? parentItem->child( ii ) : fImpl->results->topLevelItem( ii );
                if ( !child )
                    continue;
                if ( isMatchingItem( child ) )
                    retVal.push_back( child );
                auto children = getMatchingItems( child );
                retVal.insert( retVal.end(), children.begin(), children.end() );
            }
            return retVal;
        }

        QTreeWidgetItem *CSelectTMDB::getSingleMatchingItem( QTreeWidgetItem *parentItem ) const
        {
            auto children = getMatchingItems( parentItem );
            if ( children.empty() )
                return nullptr;
            QTreeWidgetItem *retVal = nullptr;
            for ( auto &&ii : children )
            {
                if ( ii && ii->isSelected() )
                {
                    if ( retVal != nullptr )
                        return nullptr;
                    retVal = ii;
                }
            }
            return retVal;
        }

        void CSelectTMDB::loadResults( std::shared_ptr< NCore::SSearchResult > info, QTreeWidgetItem *parent )
        {
            if ( fStopLoading )
                return;

            if ( info->fInfoType == NCore::EResultInfoType::eTVShow || info->fInfoType == NCore::EResultInfoType::eTVSeason )
            {
                if ( info->fChildren.empty() )
                    return;
            }

            //qDebug() << QDateTime::currentDateTime().toString() << "loading result" << fCurrentResults.size();
            QStringList data;
            EItemType itemType;
            int labelPos = -1;
            if ( fImpl->searchForTVShows->isChecked() )
            {
                data = QStringList() << info->fTitle << info->getTMDBID() << info->fSeason << info->fEpisode << info->fReleaseDate << info->fEpisodeTitle << QString();
                itemType = EItemType::eTVShow;
                labelPos = 6;
            }
            else
            {
                data = QStringList() << info->fTitle << info->getTMDBID() << info->fReleaseDate << QString();
                itemType = EItemType::eMovie;
                labelPos = 3;
            }

            QTreeWidgetItem *item;
            if ( parent )
                item = new QTreeWidgetItem( parent, data, EItemType::eMovie );
            else
                item = new QTreeWidgetItem( fImpl->results, data, itemType );

            fSearchResultMap[item] = info;

            auto label = new QLabel( info->fDescription, this );
            label->setWordWrap( true );
            fImpl->results->setItemWidget( item, labelPos, label );
            item->setExpanded( true );

            if ( !info->fPixmap.isNull() )
            {
                item->setIcon( 0, QIcon( info->fPixmap ) );
            }

            fImpl->results->resizeColumnToContents( 0 );
            if ( fBestMatch && ( info.get() == fBestMatch.get() ) )
            {
                item->setSelected( true );
                fImpl->results->scrollToItem( item );
                auto parent = item->parent();
                while ( parent )
                {
                    fImpl->results->scrollToItem( parent );
                    parent = parent->parent();
                }
            }
            qApp->processEvents();

            for ( auto &&ii : info->fChildren )
            {
                loadResults( ii, item );
                if ( fStopLoading )
                    break;
            }
        }


        std::shared_ptr< NCore::SSearchResult > CSelectTMDB::getSearchResult() const
        {
            auto first = getFirstSelected();
            if ( !first )
                return {};
            auto pos = fSearchResultMap.find( first );
            if ( pos == fSearchResultMap.end() )
                return {};

            auto retVal = ( *pos ).second;
            retVal->fExtraInfo = fImpl->resultExtraInfo->text();
            return ( *pos ).second;
        }

        void CSelectTMDB::slotAcceptItem()
        {
            slotItemChanged();
            //accept();
        }

        void CSelectTMDB::slotItemChanged()
        {
            auto first = getFirstSelected();
            if ( !first )
                return;

            fImpl->resultTitle->setText( first->text( 0 ) );
            fImpl->resultTMDBID->setText( first->text( 1 ) );
            if ( fImpl->searchForTVShows->isChecked() )
            {
                fImpl->resultSeason->setText( first->text( 2 ) );
                fImpl->resultEpisode->setText( first->text( 3 ) );
                fImpl->resultReleaseDate->setText( first->text( 4 ) );
                fImpl->resultEpisodeTitle->setText( first->text( 5 ) );
            }
            else
            {
                fImpl->resultReleaseDate->setText( first->text( 2 ) );
            }
        }

        QTreeWidgetItem *CSelectTMDB::getFirstSelected() const
        {
            auto selected = fImpl->results->selectedItems();
            if ( selected.empty() )
                return nullptr;

            auto first = selected.front();
            if ( !first )
                return nullptr;
            return first;
        }

        std::shared_ptr< NCore::SSearchTMDBInfo > CSelectTMDB::getSearchInfo()
        {
            std::shared_ptr< NCore::SSearchTMDBInfo > searchInfo;
            if ( fSearchTMDB->isActive() )
            {
                if ( !fQueuedSearchInfo )
                    fQueuedSearchInfo = std::make_shared< NCore::SSearchTMDBInfo >( *fSearchInfo );
                searchInfo = fQueuedSearchInfo;
            }
            else
                searchInfo = fSearchInfo;
            return searchInfo;
        }

        void CSelectTMDB::setSearchForTVShows( bool value, bool init )
        {
            auto searchInfo = getSearchInfo();

            searchInfo->setIsTVShow( value );
            searchInfo->updateSearchCriteria( init );
            if ( init && !fSearchPending )
                updateFromSearchInfo( searchInfo );
        }

        void CSelectTMDB::setExactMatchOnly( bool value, bool init )
        {
            auto searchInfo = getSearchInfo();
            searchInfo->setExactMatchOnly( value );
            if ( init && !fSearchPending )
                updateFromSearchInfo( searchInfo );
        }

        void CSelectTMDB::slotByNameChanged()
        {
            updateByName( false );
        }

        void CSelectTMDB::updateByName( bool init )
        {
            fImpl->searchName->setEnabled( !fImpl->byTMDBID->isChecked() );
            fImpl->searchReleaseYear->setEnabled( !fImpl->byTMDBID->isChecked() );
            fImpl->searchSeason->setEnabled( fImpl->searchForTVShows->isChecked() && !fImpl->byTMDBID->isChecked() );
            fImpl->searchEpisode->setEnabled( fImpl->searchForTVShows->isChecked() && !fImpl->byTMDBID->isChecked() );
            fImpl->searchTMDBID->setEnabled( true );
            if ( !init )
                slotSearchCriteriaChanged();
        }

        void CSelectTMDB::slotExactOrForTVShowsChanged()
        {
            auto searchInfo = getSearchInfo();

            searchInfo->setExactMatchOnly( fImpl->exactMatchesOnly->isChecked() );
            searchInfo->setIsTVShow( fImpl->searchForTVShows->isChecked() );
            updateEnabled();
            slotSearchCriteriaChanged();
        }


        void CSelectTMDB::updateEnabled()
        {
            fImpl->searchSeason->setEnabled( fImpl->searchForTVShows->isChecked() && !fImpl->byTMDBID->isChecked() );
            fImpl->searchEpisode->setEnabled( fImpl->searchForTVShows->isChecked() && !fImpl->byTMDBID->isChecked() );
            fImpl->resultSeason->setEnabled( fImpl->searchForTVShows->isChecked() );
            fImpl->resultEpisode->setEnabled( fImpl->searchForTVShows->isChecked() );
            fImpl->resultEpisodeTitle->setEnabled( fImpl->searchForTVShows->isChecked() );
        }

        void CSelectTMDB::slotSearchTextChanged()
        {
            if (fPrevSearchName.startsWith( fImpl->searchName->text() ) )
            {
                auto tmp = fPrevSearchName.mid(fImpl->searchName->text().length()).split(QRegularExpression("\\W"),Qt::SkipEmptyParts);
                if (!tmp.isEmpty())
                {
                    auto origWords = tmp;
                    for (auto && curr : tmp)
                    {
                        curr = QString("<li>%1</li>").arg(curr);
                    }
                    auto msg = tr("Words you removed: <ul>%1</ul>").arg(tmp.join(""));
                    if (QMessageBox::question(this, tr("Would you like to add these words to the known words list?"), msg, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No) == QMessageBox::StandardButton::Yes)
                    {
                        NCore::CPreferences::instance()->addKnownStrings(origWords);
                    }
                }
            }
            fPrevSearchName = fImpl->searchName->text();
            slotSearchCriteriaChanged();
        }
        void CSelectTMDB::slotSearchCriteriaChanged()
        {
            if ( fSearchTMDB->isActive() )
            {
                QTimer::singleShot( 500, this, &CSelectTMDB::slotSearchCriteriaChanged );
                return;
            }

            auto searchInfo = getSearchInfo();
            if ( fImpl->searchTMDBID->text().isEmpty() && !fImpl->byName->isChecked() )
            {
                disconnect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
                fImpl->byName->setChecked( true );
                connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
            }

            searchInfo->setSearchName( fImpl->searchName->text().trimmed() );
            searchInfo->setReleaseDate( fImpl->searchReleaseYear->text().trimmed());
            searchInfo->setSeason( fImpl->searchSeason->value() );
            searchInfo->setEpisode( fImpl->searchEpisode->value() );
            searchInfo->setTMDBID( fImpl->searchTMDBID->text().trimmed());
            searchInfo->setIsTVShow( fImpl->searchForTVShows->isChecked() );
            searchInfo->setExactMatchOnly( fImpl->exactMatchesOnly->isChecked() );
            searchInfo->setSearchByName( fImpl->byName->isChecked() );


            slotReset();

            emit sigStartSearch();
        }

    }
}