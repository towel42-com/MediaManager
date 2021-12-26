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

#include "SearchTMDBInfo.h"
#include "SearchResult.h"
#include "Preferences.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/StringUtils.h"

#include <QRegularExpression>
#include <QDebug>

namespace NMediaManager
{
    namespace NCore
    {
        SSearchTMDBInfo::SSearchTMDBInfo( const QString &text, std::shared_ptr< SSearchResult > searchResult )
        {
            fSearchResultInfo = searchResult;
            fInitSearchString = text;
            fIsTVShow = looksLikeTVShow( text, nullptr );
            updateSearchCriteria( true );
        }

        QString SSearchTMDBInfo::replaceKnownAbbreviations( const QString & string )
        {
            QString retVal = string;
            auto knownAbbreviations = CPreferences::instance()->getKnownAbbreviations();
            for ( auto &&ii = knownAbbreviations.begin(); ii != knownAbbreviations.end(); ++ii )
            {
                auto regExpStr = "(\\W|^)(?<word>" + QRegularExpression::escape( ii.key() ) + ")(\\W|$)";
                auto regExp = QRegularExpression( regExpStr, QRegularExpression::CaseInsensitiveOption );
                auto match = regExp.match( retVal );
                if ( match.hasMatch() )
                {
                    retVal.replace( match.capturedStart( "word" ), match.capturedLength( "word" ), ii.value().toString() );
                }
            }
            return retVal;
        }

        QString SSearchTMDBInfo::stripKnownData( const QString &string )
        {
            QString retVal = string;
            auto knownStrings = CPreferences::instance()->getKnownStrings();
            for ( auto &&knownString : knownStrings )
            {
                auto regExpStr1 = "((?<prefix>\\[|\\()|\\W)(?<word>" + QRegularExpression::escape(knownString) + ")((?<suffix>\\]|\\))|\\W|$)";

                auto regExp = QRegularExpression(regExpStr1, QRegularExpression::CaseInsensitiveOption);
                auto match = regExp.match(retVal);
                if (match.hasMatch())
                {
                    auto start = match.capturedStart( "prefix" );
                    if ( start == -1 )
                        start = match.capturedStart( "word" );
                    auto end = match.capturedEnd( "suffix" );
                    if ( end == -1 )
                        end = match.capturedEnd( "word" );

                    
                    retVal.remove( start, end-start );
                }
            }
            return retVal;
        }

        QString SSearchTMDBInfo::stripKnownExtendedData(const QString & string, QString & extendedData )
        {
            QString retVal = string;
            auto knownStrings = CPreferences::instance()->getKnownExtendedStrings();
            for (auto && knownString : knownStrings)
            {
                auto regExpStr = "\\W(?<word>" + QRegularExpression::escape(knownString) + ")(\\W|$)";
                auto regExp = QRegularExpression(regExpStr, QRegularExpression::CaseInsensitiveOption);
                auto match = regExp.match(retVal);
                if (match.hasMatch())
                {
                    retVal.remove(match.capturedStart("word"), match.capturedLength("word"));
                    extendedData = NStringUtils::transformTitle( match.captured("word"),true );
                    break;
                }
            }
            return retVal;
        }

        QString SSearchTMDBInfo::smartTrim( const QString &string, bool stripInnerPeriods )
        {
            auto retVal = string;
            auto pos = retVal.indexOf( QRegularExpression( "[^\\.\\s\\-]" ) );
            if ( pos != -1 )
                retVal = retVal.mid( pos );

            pos = retVal.lastIndexOf( QRegularExpression( "[^\\.\\s\\-]" ) );
            if ( pos != -1 )
                retVal = retVal.left( pos + 1 );
            if ( stripInnerPeriods )
            {
                retVal.replace( QRegularExpression( "\\.|(\\s{2,})|-|\\:" ), " " );
                retVal = retVal.trimmed();
            }
            return retVal;
        }

        QStringList SSearchTMDBInfo::stripOutPositions( const QString & inString, const std::list< std::pair< int, int > > & positions )
        {
            QStringList retVal;
            int posStart = 0;
            for( auto ii = positions.begin(); ii != positions.end(); ++ii )
            {
                auto curr = smartTrim( inString.mid( posStart, ( *ii ).first - posStart ) );
                posStart = ( *ii ).first + ( *ii ).second;
                retVal << curr;
            }
            auto curr = smartTrim( inString.mid( posStart ) );
            retVal << curr;
            retVal.removeAll( QString() );
            return retVal;
        }

        bool SSearchTMDBInfo::looksLikeTVShow( const QString &searchString, QString *titleStr, QString *seasonStr, QString *episodeStr, QString * extraStr )
        {
            QString localRetVal = searchString;

            if ( titleStr )
                titleStr->clear();
            if ( seasonStr )
                seasonStr->clear();
            if ( episodeStr )
                episodeStr->clear();
            if ( extraStr )
                extraStr->clear();

            bool isTV = false;
            auto regExpStr = QString( "S(?<season>\\d{1,4})" );
            auto regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            auto match = regExp.match( localRetVal );
            std::list< std::pair< int, int > > positions;
            if ( match.hasMatch() )
            {
                if ( seasonStr )
                    *seasonStr = smartTrim( match.captured( "season" ) );
                positions.push_back( std::make_pair( match.capturedStart( "season" ) - 1, match.capturedLength( "season" ) + 1 ) );
                isTV = true;
            }

            if ( titleStr || !isTV )
            {
                regExpStr = "E(?<episode>\\d{1,4})";
                regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
                match = regExp.match( localRetVal );
                if ( match.hasMatch() )
                {
                    if ( episodeStr )
                        *episodeStr = smartTrim( match.captured( "episode" ) );

                    auto pos = std::make_pair( match.capturedStart( "episode" ) - 1, match.capturedLength( "episode" ) + 1 );

                    if ( positions.empty() || ( positions.front().first < match.capturedStart( "episode" ) ) )
                        positions.push_back( pos );
                    else
                        positions.push_front( pos );
                    isTV = true;
                }
            }

            if ( titleStr )
            {
                auto data = stripOutPositions( localRetVal, positions );
                if ( !data.isEmpty() )
                {
                    localRetVal = data.front();
                    data.pop_front();
                    if ( extraStr )
                        *extraStr = data.join( " " );
                }
            }

            if ( titleStr || !isTV )
            {
                regExpStr = ".*\\s??(?<seasonsuffix>-\\s??Season\\s?(?<season>\\d+))";
                regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
                match = regExp.match( localRetVal );
                if ( match.hasMatch() )
                {
                    if ( seasonStr )
                        *seasonStr = smartTrim( match.captured( ( "season" ) ) );
                    if ( titleStr )
                        localRetVal.replace( match.capturedStart( "seasonsuffix" ), match.capturedEnd( "seasonsuffix" ), "" );
                    isTV = true;
                }
            }
            if ( titleStr )
                *titleStr = localRetVal;
            return isTV;
        }

        void SSearchTMDBInfo::updateSearchCriteria( bool updateSearchBy )
        {
            QString extendedInfo;

            fSearchName = smartTrim( stripKnownData( fInitSearchString ) );
            fSearchName = smartTrim( stripKnownExtendedData( fSearchName, extendedInfo ) );
            fSearchName = smartTrim( replaceKnownAbbreviations( fSearchName ) );

            fFoundExtendedInfo = extendedInfo;

            QString seasonStr;
            QString episodeStr;

            if ( fIsTVShow )
            {
                looksLikeTVShow( fSearchName, &fSearchName, &seasonStr, &episodeStr, &fEpisodeTitle );
                fEpisodeTitle = smartTrim( fEpisodeTitle, true );
            }

                            //(?<fulltext>[\.\(]  (?<releaseDate>((\\d{2}){1,2}))(?:[\.\)]?|$))
                            //(?<!\d)
            //(?<fulltext>[[|\(|\W|^](?<releaseDate>((\d{2}){1,2}))((?<suffix>\]|\))|\W|$))

            auto regExpStr = "(?<fulltext>(([\\(\\[]|^)|(?<!\\d))(?<releaseDate>\\d{2}|\\d{4})(\\D|\\)|\\]|$))";
            auto regExp = QRegularExpression( regExpStr );
            auto match = regExp.match( fSearchName );
            if ( match.hasMatch() )
            {
                fReleaseDate = smartTrim( match.captured( "releaseDate" ) );
                fSearchName.replace( match.capturedStart( "fulltext" ), match.capturedLength( "fulltext" ), "" );
            }

            regExp = QRegularExpression( "(?<fulltext>\\[tmdbid=(?<tmdbid>\\d+)\\])" );
            match = regExp.match( fSearchName );
            if ( match.hasMatch() )
            {
                fTMDBID = smartTrim( match.captured( "tmdbid" ) );
                fSearchName.replace( match.capturedStart( "fulltext" ), match.capturedLength( "fulltext" ), "" );
            }

            fSearchName = smartTrim( fSearchName, true );

            if ( fSearchResultInfo )
            {
                fSearchName = fSearchResultInfo->fTitle;
                episodeStr = fSearchResultInfo->fEpisode;
                seasonStr = fSearchResultInfo->fSeason;
                fReleaseDate = fSearchResultInfo->fReleaseDate;
                fTMDBID = fSearchResultInfo->fTMDBID;// always get the main one
                fEpisodeTitle = fSearchResultInfo->fEpisodeTitle;
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

        int SSearchTMDBInfo::releaseDate( bool *aOK ) const
        {
            return fReleaseDate.toInt( aOK );
        }

        int SSearchTMDBInfo::tmdbID( bool *aOK ) const
        {
            return fTMDBID.toInt( aOK );
        }

        QDebug operator<<( QDebug debug, const SSearchTMDBInfo &info )
        {
            debug << info.toString( true );
            return debug;
        }

        QString SSearchTMDBInfo::toString( bool forDebug ) const
        {
            auto retVal = forDebug
                ? QString( "SSearchTMDBInfo(%1 (%2)-S%3E%4-%5-%6-%7)" ).arg( searchName() ).arg( releaseDateString() ).arg( season() ).arg( episode() ).arg( tmdbIDString() ).arg( isTVShow() ).arg( exactMatchOnly() )
                : QString( "Search Name: '%1' - Release Date: %2 - Season: %3 - Episode: %4 - TMDB ID: %5 - Is TV Show? %6 - Exact Match Only: %7" ).arg( searchName() ).arg( releaseDateString().isEmpty() ? "<Not Set>" : releaseDateString() ).arg( (season() == -1) ? "<Not Set>" : QString::number( season() ) ).arg( ( episode() == -1 ) ? "<Not Set>" : QString::number( episode() ) ).arg( tmdbIDString().isEmpty() ? "<Not Set>" : tmdbIDString() ).arg( isTVShow() ? "Yes" : "No" ).arg( exactMatchOnly() ? "Yes" : "No" );
            return retVal;
        }

        bool SSearchTMDBInfo::isMatch( std::shared_ptr< SSearchResult > searchResult ) const
        {
            return isMatch( searchResult->fReleaseDate, searchResult->fTMDBID, searchResult->getTitle(), searchResult->isTVShow(), searchResult->getSeason(), searchResult->getEpisode() );
        }

        bool SSearchTMDBInfo::isSeasonMatch( int seasonMatch ) const
        {
            if ( seasonMatch == -1 )
                return ( fSeason == -1 );
            if ( fSeason == -1 )
                return false;
            return seasonMatch == fSeason;
        }


        bool SSearchTMDBInfo::isSeasonMatch( const QString &seasonMatch ) const
        {
            if ( seasonMatch.isEmpty() )
                return ( fSeason == -1 );
            bool aOK;
            auto season = seasonMatch.toInt( &aOK );
            if ( !aOK )
                return false;
            return isSeasonMatch( season );
        }

        bool SSearchTMDBInfo::isEpisodeMatch( int episodeMatch ) const
        {
            if ( episodeMatch == -1 )
                return ( fEpisode == -1 );
            if ( fEpisode == -1 )
                return false;
            return episodeMatch == fSeason;
        }


        bool SSearchTMDBInfo::isEpisodeMatch( const QString &episodeMatch ) const
        {
            if ( episodeMatch.isEmpty() )
                return ( fEpisode == -1 );
            bool aOK;
            auto season = episodeMatch.toInt( &aOK );
            if ( !aOK )
                return false;
            return isEpisodeMatch( season );
        }

        bool SSearchTMDBInfo::isMatchingTMDBID( const QString &inTMDBID ) const
        {
            if ( inTMDBID.isEmpty() )
                return false;

            bool aOK;
            int tmdbid = inTMDBID.toInt( &aOK );
            if ( !aOK )
                tmdbid = -1;

            return isMatchingTMDBID( tmdbid );
        }

        bool SSearchTMDBInfo::isMatchingTMDBID( int tmdbid ) const
        {
            bool aOK;
            int myTmdbID = tmdbID( &aOK );
            bool canCheckTMDB = tmdbIDSet() && ( tmdbid != -1 );

            if ( aOK && canCheckTMDB && !isTVShow() ) // dont check for TV shows, as the TMDB could be the episode ID
            {
                if ( tmdbid != myTmdbID )
                    return false;
            }
            return true;
        }

        bool SSearchTMDBInfo::isMatchingName( const QString &name ) const
        {
            if ( !fSearchByName )
                return true;
            if ( fExactMatchOnly )
            {
                return name.compare( fSearchName, Qt::CaseInsensitive ) == 0;
            }
            return NStringUtils::isSimilar( name, fSearchName ); // if every word we are searching for is covered by name, we match
        }

        bool SSearchTMDBInfo::isMatchingDate( const QString &releaseDate ) const
        {
            bool aOK;
            int releaseYear = this->releaseDate( &aOK );
            if ( aOK && searchByName() && releaseDateSet() && !releaseDate.isEmpty() )
            {
                auto dt = NQtUtils::findDate( releaseDate );

                if ( dt.isValid() && dt.year() != releaseYear )
                {
                    return false;
                }
            }
            return true;
        }
    }
}
