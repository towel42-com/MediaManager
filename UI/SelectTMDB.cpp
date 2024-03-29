// The MIT License( MIT )
//
// Copyright( c ) 2020-2023 Scott Aron Bloom
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
#include "Models/DirModel.h"
#include "Core/TransformResult.h"
#include "Core/SearchTMDB.h"
#include "Core/SearchTMDBInfo.h"
#include "Preferences/Core/Preferences.h"

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/StringUtils.h"

#include <QTimer>
#include <QDebug>
#include <QRegularExpression>
#include <QPushButton>
#include <QMessageBox>

namespace NMediaManager
{
    namespace NUi
    {
        CSelectTMDB::CSelectTMDB( const QString &text, std::shared_ptr< NCore::CTransformResult > searchResult, QWidget *parent ) :
            QDialog( parent ),
            fImpl( new Ui::CSelectTMDB )
        {
            fImpl->setupUi( this );

            fSearchInfo = std::make_shared< NCore::SSearchTMDBInfo >( text, searchResult );
            fSearchTMDB = new NCore::CSearchTMDB( fSearchInfo, std::optional< QString >(), this );

            fImpl->resultExtraInfo->setText( searchResult ? searchResult->extraInfo() : QString() );
            fImpl->resultEpisodeTitle->setText( fSearchInfo->subTitle() );

            updateFromSearchInfo( fSearchInfo );
            slotReset();

            fImpl->results->setIconSize( QSize( 128, 128 ) );

            updateByName( true );
            updateEnabled();

            QObject::connect( this, &CSelectTMDB::sigStartSearch, fSearchTMDB, &NCore::CSearchTMDB::slotSearch );
            QObject::connect( fImpl->results, &QTreeWidget::itemDoubleClicked, this, &CSelectTMDB::slotAcceptItem );
            QObject::connect( fSearchTMDB, &NCore::CSearchTMDB::sigSearchFinished, this, &CSelectTMDB::slotSearchFinished );
            QObject::connect( fSearchTMDB, &NCore::CSearchTMDB::sigAutoSearchPartialFinished, this, &CSelectTMDB::slotSearchPartialFinished );
            QObject::connect( fImpl->results->selectionModel(), &QItemSelectionModel::selectionChanged, this, &CSelectTMDB::slotItemChanged );

            connect();

            QTimer::singleShot( 0, this, &CSelectTMDB::slotSearchCriteriaChanged );
        }

        void CSelectTMDB::updateFromSearchInfo( std::shared_ptr< NCore::SSearchTMDBInfo > searchInfo )
        {
            disconnect();

            fImpl->resultExtraInfo->setText( searchInfo->getExtendedInfo() );

            fImpl->searchName->setText( fPrevSearchName = searchInfo->searchName() );
            fImpl->searchSeason->setValue( searchInfo->season() );
            fImpl->searchEpisode->setValue( searchInfo->hasEpisodes() ? searchInfo->episodes().front() : -1 );
            fImpl->searchReleaseYear->setText( searchInfo->releaseDate().second );
            fImpl->searchTMDBID->setText( searchInfo->tmdbIDString() );
            fImpl->byName->setChecked( fSearchTMDB->searchByName() );
            fImpl->byTMDBID->setChecked( !fSearchTMDB->searchByName() );
            fImpl->exactMatchesOnly->setChecked( searchInfo->exactMatchOnly() );
            fImpl->searchForTVShows->setChecked( searchInfo->isTVMedia() );
            updateEnabled();

            connect();
        }

        void CSelectTMDB::connect()
        {
            QObject::connect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
            QObject::connect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );
            QObject::connect( fImpl->searchForTVShows, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );

            fImpl->searchName->setDelay( 1000 );
            fImpl->searchSeason->setDelay( 1000 );
            fImpl->searchEpisode->setDelay( 1000 );
            fImpl->searchReleaseYear->setDelay( 1000 );
            fImpl->searchTMDBID->setDelay( 1000 );

            QObject::connect( fImpl->searchName, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchTextChanged );
            QObject::connect( fImpl->searchSeason, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            QObject::connect( fImpl->searchEpisode, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            QObject::connect( fImpl->searchReleaseYear, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            QObject::connect( fImpl->searchTMDBID, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
        }

        void CSelectTMDB::disconnect()
        {
            QObject::disconnect( fImpl->byName, &QRadioButton::toggled, this, &CSelectTMDB::slotByNameChanged );
            QObject::disconnect( fImpl->exactMatchesOnly, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );
            QObject::disconnect( fImpl->searchForTVShows, &QCheckBox::clicked, this, &CSelectTMDB::slotExactOrForTVShowsChanged );

            fImpl->searchName->setDelay( -1 );
            fImpl->searchSeason->setDelay( -1 );
            fImpl->searchEpisode->setDelay( -1 );
            fImpl->searchReleaseYear->setDelay( -1 );
            fImpl->searchTMDBID->setDelay( -1 );

            QObject::disconnect( fImpl->searchName, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchTextChanged );
            QObject::disconnect( fImpl->searchSeason, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            QObject::disconnect( fImpl->searchEpisode, &NSABUtils::CDelaySpinBox::sigValueChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            QObject::disconnect( fImpl->searchReleaseYear, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
            QObject::disconnect( fImpl->searchTMDBID, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CSelectTMDB::slotSearchCriteriaChanged );
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
            fImpl->resultsLabel->setText( tr( "Results:" ) );
            fSearchResultMap.clear();
            resetHeader();
        }

        void CSelectTMDB::resetHeader()
        {
            fImpl->results->setColumnCount( 0 );
            if ( fImpl->searchForTVShows->isChecked() )
                fImpl->results->setHeaderLabels(
                    QStringList() << "Title"
                                  << "TMDB ID"
                                  << "Season"
                                  << "Episode"
                                  << "Air Date"
                                  << "Episode Title"
                                  << "Desc" );
            else
                fImpl->results->setHeaderLabels(
                    QStringList() << "Title"
                                  << "TMDB ID"
                                  << "Release Date"
                                  << "Desc" );
        }

        CSelectTMDB::~CSelectTMDB()
        {
        }

        const QString apiKeyV3 = "7c58ff37c9fadd56c51dae3a97339378";
        const QString apiKeyV4 =
            "eyJhbGciOiJIUzI1NiJ9.eyJhdWQiOiI3YzU4ZmYzN2M5ZmFkZDU2YzUxZGFlM2E5NzMzOTM3OCIsInN1YiI6IjVmYTAzMzJiNjM1MDEzMDAzMTViZjg2NyIsInNjb3BlcyI6WyJhcGlfcmVhZCJdLCJ2ZXJzaW9uIjoxfQ.MBAzJIxvsRm54kgPKcfixxtfbg2bdNGDHKnEt15Nuac";

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
            if ( fPartialResults )
                fLoading = false;

            fPartialResults = false;
            searchFinished();
        }

        void CSelectTMDB::slotSearchPartialFinished()
        {
            fPartialResults = true;
            searchFinished();
        }

        void CSelectTMDB::searchFinished()
        {
            QApplication::restoreOverrideCursor();

            if ( fSearchTMDB->hasError() )
            {
                QMessageBox::information( this, tr( "Error searching themoviedb" ), fSearchTMDB->errorString() );
                return;
            }

            if ( fLoading && !fPartialResults )
                return;

            auto currResults = fPartialResults ? fSearchTMDB->getPartialResults() : fSearchTMDB->getResults();
            for ( auto &&ii : currResults )
            {
                if ( ii->isNotFoundResult() )
                    continue;
                fCurrentResults.push_back( ii );
            }
            //fCurrentResults.insert( fCurrentResults.end(), currResults.begin(), currResults.end() );
            if ( !fPartialResults )
                fBestMatch = fSearchTMDB->bestMatch();

            fLoading = true;
            //qDebug() << QDateTime::currentDateTime().toString() << "launching slotLoadNextResult" << fCurrentResults.size();
            QTimer::singleShot( 0, this, &CSelectTMDB::slotLoadNextResult );
        }

        void CSelectTMDB::countResults( QTreeWidgetItem *parent, std::tuple< int, int, int > &count )
        {
            auto childCount = parent ? parent->childCount() : fImpl->results->topLevelItemCount();
            for ( int ii = 0; ii < childCount; ++ii )
            {
                auto item = parent ? parent->child( ii ) : fImpl->results->topLevelItem( ii );
                if ( !item )
                    continue;
                auto type = static_cast< EItemType >( item->type() );
                if ( type == EItemType::eMovie )
                    std::get< 0 >( count )++;
                else if ( type == EItemType::eTVShow )
                    std::get< 1 >( count )++;
                else if ( type == EItemType::eEpisode )
                    std::get< 1 >( count )++;

                countResults( item, count );
            }
        }

        void CSelectTMDB::setResultsLabel()
        {
            std::tuple< int, int, int > count{ 0, 0, 0 };
            countResults( nullptr, count );

            QStringList results;
            if ( std::get< 0 >( count ) )
                results << tr( "Movies: %1" ).arg( std::get< 0 >( count ) );
            if ( std::get< 1 >( count ) )
                results << tr( "TV Shows: %1" ).arg( std::get< 1 >( count ) );
            if ( std::get< 2 >( count ) )
                results << tr( "TV Episodes: %1" ).arg( std::get< 2 >( count ) );

            fImpl->resultsLabel->setText( tr( "Results: %1" ).arg( results.join( " - " ) ) );
        }

        void CSelectTMDB::slotLoadNextResult()
        {
            //qDebug() << QDateTime::currentDateTime().toString() << "handling slotLoadNextResult" << fCurrentResults.size();
            if ( fStopLoading )
                fCurrentResults.clear();

            if ( fCurrentResults.empty() )
            {
                if ( fPartialResults )
                    return;

                QApplication::restoreOverrideCursor();

                setResultsLabel();

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
                    slotItemChanged();
                return;
            }

            auto curr = fCurrentResults.front();
            fCurrentResults.pop_front();
            if ( !curr->isNotFoundResult() )
                loadResults( curr, nullptr );

            //qDebug() << QDateTime::currentDateTime().toString() << "Launching slotLoadNextResult" << fCurrentResults.size();
            QTimer::singleShot( 100, this, &CSelectTMDB::slotLoadNextResult );
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
        std::list< QTreeWidgetItem * > CSelectTMDB::getMatchingItems( QTreeWidgetItem *parentItem ) const
        {
            std::list< QTreeWidgetItem * > retVal;

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

        void CSelectTMDB::loadResults( std::shared_ptr< NCore::CTransformResult > info, QTreeWidgetItem *parent )
        {
            if ( fStopLoading )
                return;

            if ( info->mediaType() == NCore::EMediaType::eTVShow || info->mediaType() == NCore::EMediaType::eTVSeason )
            {
                if ( !info->hasChildren() )
                    return;
            }

            //if ( fAddedInfo.find( info ) != fAddedInfo.end() )
            //    return;

            fAddedInfo.insert( info );
            //qDebug() << QDateTime::currentDateTime().toString() << "loading result" << fCurrentResults.size();
            QStringList data;
            EItemType itemType;
            int labelPos = -1;
            if ( fImpl->searchForTVShows->isChecked() )
            {
                data = QStringList() << info->title() << info->getTMDBID() << info->season() << info->episode() << info->getDate().second << info->subTitle() << QString();
                itemType = EItemType::eTVShow;
                labelPos = 6;
            }
            else
            {
                data = QStringList() << info->title() << info->getTMDBID() << info->getDate().second << QString();
                itemType = EItemType::eMovie;
                labelPos = 3;
            }

            QTreeWidgetItem *item;
            if ( parent )
                item = new QTreeWidgetItem( parent, data, EItemType::eMovie );
            else
                item = new QTreeWidgetItem( fImpl->results, data, itemType );

            fSearchResultMap[ item ] = info;

            auto label = new QLabel( info->description(), this );
            label->setWordWrap( true );
            fImpl->results->setItemWidget( item, labelPos, label );
            item->setExpanded( true );

            if ( !info->pixmap().isNull() )
            {
                item->setIcon( 0, QIcon( info->pixmap() ) );
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

            info->onAllChildren( [ item, this ]( std::shared_ptr< NCore::CTransformResult > child ) { loadResults( child, item ); }, [ this ]() { return fStopLoading; } );
        }

        std::shared_ptr< NCore::CTransformResult > CSelectTMDB::getSearchResult() const
        {
            auto first = getFirstSelected();
            if ( !first )
                return {};
            auto pos = fSearchResultMap.find( first );
            if ( pos == fSearchResultMap.end() )
                return {};

            auto retVal = ( *pos ).second;
            retVal->setExtraInfo( fImpl->resultExtraInfo->text() );
            return ( *pos ).second;
        }

        void CSelectTMDB::slotAcceptItem()
        {
            slotItemChanged();
            accept();
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

            searchInfo->setMediaType( value ? NCore::EMediaType::eTVShow : NCore::EMediaType::eMovie );
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
            bool byName = searchByName();
            fImpl->searchName->setEnabled( byName );
            fImpl->searchReleaseYear->setEnabled( byName );
            fImpl->searchSeason->setEnabled( fImpl->searchForTVShows->isChecked() && byName );
            fImpl->searchEpisode->setEnabled( fImpl->searchForTVShows->isChecked() && byName );
            fImpl->searchTMDBID->setEnabled( true );
            if ( !init )
                slotSearchCriteriaChanged();
        }

        void CSelectTMDB::slotExactOrForTVShowsChanged()
        {
            auto searchInfo = getSearchInfo();

            searchInfo->setExactMatchOnly( fImpl->exactMatchesOnly->isChecked() );
            searchInfo->setMediaType( fImpl->searchForTVShows->isChecked() ? NCore::EMediaType::eTVShow : NCore::EMediaType::eMovie );
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
            if ( fPrevSearchName.startsWith( fImpl->searchName->text() ) )
            {
                auto tmp = fPrevSearchName.mid( fImpl->searchName->text().length() ).split( QRegularExpression( "\\W" ), NSABUtils::NStringUtils::TSkipEmptyParts );
                if ( !tmp.isEmpty() )
                {
                    auto origWords = tmp;
                    for ( auto &&curr : tmp )
                    {
                        curr = QString( "<li>%1</li>" ).arg( curr );
                    }
                    auto msg = tr( "Words you removed: <ul>%1</ul>" ).arg( tmp.join( "" ) );
                    if ( QMessageBox::question( this, tr( "Would you like to add these words to the known words list?" ), msg, QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::Yes )
                    {
                        NPreferences::NCore::CPreferences::instance()->addKnownStrings( origWords );
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

            searchInfo->setSearchName( fImpl->searchName->text().trimmed() );
            searchInfo->setReleaseDate( fImpl->searchReleaseYear->text().trimmed() );
            searchInfo->setSeason( fImpl->searchSeason->value() );
            searchInfo->setEpisodes( { fImpl->searchEpisode->value() } );
            searchInfo->setTMDBID( fImpl->searchTMDBID->text().trimmed() );
            searchInfo->setMediaType( fImpl->searchForTVShows->isChecked() ? NCore::EMediaType::eTVShow : NCore::EMediaType::eMovie );
            searchInfo->setExactMatchOnly( fImpl->exactMatchesOnly->isChecked() );
            searchInfo->setSearchByName( fImpl->byName->isChecked() );

            if ( !searchInfo->canSearch() )
                return;

            QApplication::setOverrideCursor( Qt::WaitCursor );

            slotReset();

            emit sigStartSearch();
        }

        bool CSelectTMDB::searchByName()
        {
            return fImpl->byName->isChecked();
        }

    }
}