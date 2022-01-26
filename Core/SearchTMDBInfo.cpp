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
#include "TransformResult.h"
#include "Preferences.h"
#include "SearchTMDB.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/StringUtils.h"

#include <QRegularExpression>
#include <QDebug>
#include <QUrlQuery>
#include <QFileInfo>

namespace NMediaManager
{
    namespace NCore
    {
        SSearchTMDBInfo::SSearchTMDBInfo( const QString &text, std::shared_ptr< STransformResult > searchResult )
        {
            fSearchResultInfo = searchResult;
            fInitSearchString = text;
            fMediaType = std::make_pair( looksLikeTVShow( text, nullptr ), true );
            updateSearchCriteria( true );
        }

        SSearchTMDBInfo::SSearchTMDBInfo() :
            fMediaType( std::make_pair( EMediaType::eUnknownType, true ) )
        {

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
            auto regExs = CPreferences::instance()->getKnownStringRegExs();
            for ( auto &&ii : regExs )
            {
                auto regEx = QRegularExpression( ii, QRegularExpression::CaseInsensitiveOption);
                auto match = regEx.match(retVal);
                while (match.hasMatch())
                {
                    auto start = match.capturedStart( "prefix" );
                    if ( start == -1 )
                        start = match.capturedStart( "word" );
                    auto end = match.capturedEnd( "suffix" );
                    if ( end == -1 )
                        end = match.capturedEnd( "word" );

                    retVal.remove( start, end-start );
                    
                    match = regEx.match( retVal, start );
                }
            }
            return retVal;
        }
        
        QString SSearchTMDBInfo::stripExistingExtraInfo(const QString & string, QString & extendedData)
        {
            auto regExStr = R"([\)\]\s*(?<total>( - )(?<extendedData>[^\[\(]+))$)";
            auto regEx = QRegularExpression(regExStr);
            QString retVal = string;
            auto match = regEx.match(retVal);
            if (match.hasMatch())
            {
                extendedData = match.captured("extendedData");
                retVal.remove( match.capturedStart("total"), match.capturedLength("total") );
            }
            return retVal;
        }

        QString SSearchTMDBInfo::stripKnownExtendedData(const QString & string, QString & extendedData)
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
                    extendedData = NSABUtils::NStringUtils::transformTitle( match.captured("word"),true );
                    break;
                }
            }
            return retVal;
        }

        QString SSearchTMDBInfo::smartTrim( const QString &string, bool stripInnerSeparators )
        {
            auto retVal = string;
            auto pos = retVal.indexOf( QRegularExpression( R"([^\.\s\-\_])" ) );
            if ( pos != -1 )
                retVal = retVal.mid( pos );

            pos = retVal.lastIndexOf( QRegularExpression( R"([^\.\s\-\_])" ) );
            if ( pos != -1 )
                retVal = retVal.left( pos + 1 );
            if ( stripInnerSeparators )
            {
                retVal.replace( QRegularExpression( R"(\.|(\s{2,})|-|\:|_)" ), "" );
                retVal = retVal.trimmed();
            }
            return retVal;
        }

        QStringList SSearchTMDBInfo::stripOutPositions( const QString & inString, const std::list< std::pair< int, int > > & positions )
        {
            QStringList retVal;
            int posStart = 0;
            for(const auto & position : positions)
            {
                auto curr = smartTrim( inString.mid( posStart, position.first - posStart ) );
                posStart = position.first + position.second;
                retVal << curr;
            }
            auto curr = smartTrim( inString.mid( posStart ) );
            retVal << curr;
            retVal.removeAll( QString() );
            return retVal;
        }

        EMediaType SSearchTMDBInfo::looksLikeTVShow( const QString &searchString, QString *titleStr, QString *seasonStr, QString *episodeStr, QString * extraStr )
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

            EMediaType retVal = EMediaType::eUnknownType; // default is a movie

            auto regExpStr = QString( R"((^|[^A-Z])S(?<garbage>EASON)?(?<season>\d{1,4}))" );
            auto regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            auto match = regExp.match( localRetVal );
            std::list< std::pair< int, int > > positions;
            if ( match.hasMatch() )
            {
                if ( seasonStr )
                    *seasonStr = smartTrim( match.captured( "season" ) );
                if ( match.capturedStart( "garbage" ) != -1 )
                    positions.emplace_back( match.capturedStart( "garbage" ) - 1, match.capturedLength( "garbage" ) + 1);
                positions.emplace_back( match.capturedStart( "season" ) - 1, match.capturedLength( "season" ) + 1);

                retVal = EMediaType::eTVSeason;
            }

            regExpStr = R"((^|[^A-Z])E(?<garbage>PISODE)?(?<episode>\d{1,4}))";
            regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            match = regExp.match( localRetVal );
            if ( match.hasMatch() )
            {
                if ( episodeStr )
                    *episodeStr = smartTrim( match.captured( "episode" ) );

                if ( match.capturedStart( "garbage" ) != -1 )
                    positions.emplace_back( match.capturedStart( "garbage" ) - 1, match.capturedLength( "garbage" ) + 1);
                auto pos = std::make_pair( match.capturedStart( "episode" ) - 1, match.capturedLength( "episode" ) + 1 );

                if ( positions.empty() || ( positions.front().first < match.capturedStart( "episode" ) ) )
                    positions.push_back( pos );
                else
                    positions.push_front( pos );
                retVal = EMediaType::eTVEpisode;
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

            regExpStr = R"(.*\s??(?<seasonsuffix>-\s??Season\s?(?<season>\d+)))";
            regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            match = regExp.match( localRetVal );
            if ( match.hasMatch() )
            {
                if ( seasonStr )
                    *seasonStr = smartTrim( match.captured( ( "season" ) ) );
                if ( titleStr )
                    localRetVal.replace( match.capturedStart( "seasonsuffix" ), match.capturedEnd( "seasonsuffix" ), "" );
                retVal = EMediaType::eTVSeason;
            }

            if ( titleStr )
                *titleStr = localRetVal;
            if ( retVal == EMediaType::eUnknownType )
                retVal = EMediaType::eMovie;
            return retVal;
        }

        void SSearchTMDBInfo::updateSearchCriteria( bool updateSearchBy )
        {
            QString extendedInfo;

            fSearchName = smartTrim( stripExistingExtraInfo( fInitSearchString, extendedInfo ) );
            fSearchName = smartTrim( stripKnownData( fSearchName ) );
            fSearchName = smartTrim( stripKnownExtendedData( fSearchName, extendedInfo ) );
            fSearchName = smartTrim( replaceKnownAbbreviations( fSearchName ) );

            fFoundExtendedInfo = extendedInfo;

            extractDiskNum();
            extractTVInfo();
            extractReleaseDate();
            extractTMDBID();

            fSearchName = smartTrim( fSearchName, true );

            if ( fSearchResultInfo )
            {
                fSearchName = fSearchResultInfo->fTitle;
            }

            if ( updateSearchBy )
                fSearchByName = fTMDBID.isEmpty();
        }

        int SSearchTMDBInfo::releaseYear( const QString & dateStr, bool * aOK )
        {
            bool lclAOK;
            auto retVal = dateStr.toInt( &lclAOK );

            if ( !lclAOK )
            {
                auto dt = NSABUtils::findDate( dateStr );
                lclAOK = dt.isValid();
                if ( lclAOK )
                    retVal = dt.year();
            }
            if ( aOK )
                *aOK = lclAOK;
            return retVal;
        }

        int SSearchTMDBInfo::releaseYear( bool * aOK ) const
        {
            return releaseYear( fReleaseDate, aOK );
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
            auto retVal = forDebug ? QString( "SSearchTMDBInfo(%1 (%2)-S%3E%4-%5-%6-%7)" ) : QString( "Search Name: '%1' - Release Date: %2 - Season: %3 - Episode: %4 - TMDB ID: %5 - Media Type: %6 Auto Determined: %7 - Exact Match Only: %8" );

            retVal = retVal
                .arg( searchName() )
                .arg( forDebug ? releaseDateString() : (releaseDateString().isEmpty() ? "<Not Set>" : releaseDateString()) )
                .arg( forDebug ? QString::number( season() ) : (season() == -1) ? "<Not Set>" : QString::number( season() ) )
                .arg( forDebug ? QString::number( episode() ) : (episode() == -1) ? "<Not Set>" : QString::number( episode() ) )
                .arg( forDebug ? tmdbIDString() : tmdbIDString().isEmpty() ? "<Not Set>" : tmdbIDString() )
                .arg( toEnumString( fMediaType.first ) ).arg( fMediaType.second ? "Yes" : "No" )
                .arg( forDebug ? QString("%1").arg( exactMatchOnly() ) :exactMatchOnly() ? "Yes" : "No" )
                ;

            return retVal;
        }

        bool SSearchTMDBInfo::isMatch( std::shared_ptr< STransformResult > searchResult ) const
        {
            return isMatch( searchResult->fReleaseDate, searchResult->fTMDBID, searchResult->getTitle(), searchResult->mediaType(), searchResult->getSeason(), searchResult->getEpisode() );
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
            if ( isTVMedia() ) // dont check for TV shows, as the TMDB could be the episode ID or season ID
                return true;

            if ( !tmdbIDSet() )
                return true;

            if ( tmdbid == -1 )
                return false;

            bool aOK;
            int myTmdbID = tmdbID( &aOK );
            if ( !aOK )
                return false;

            return tmdbid == myTmdbID;
        }

        bool SSearchTMDBInfo::isMatchingDate( const QString & releaseDate ) const
        {
            if ( !releaseDateSet() )
                return true;

            if ( releaseDate.isEmpty() ) // we are searching for a release date, and none was found
                return false;

            if ( !fExactMatchOnly )
                return true;

            bool searchAOK;
            auto searchReleaseYear = this->releaseYear( &searchAOK );
            bool foundAOK;
            auto foundReleaseYear = releaseYear( releaseDate, &foundAOK );
            if ( searchAOK != foundAOK )
                return false;
            return (searchReleaseYear == foundReleaseYear);
        }

        bool SSearchTMDBInfo::isMatchingName( const QString &name ) const
        {
            if ( !fSearchByName )
                return true;
            return NSABUtils::NStringUtils::isSimilar( name, fSearchName, fExactMatchOnly ); // if every word we are searching for is covered by name, we match.  For exact matches must be in same order
        }

        void SSearchTMDBInfo::extractReleaseDate()
        {
            //(?<fulltext>[\.\(]  (?<releaseDate>((\\d{2}){1,2}))(?:[\.\)]?|$))
            //(?<!\d)
            //(?<fulltext>[[|\(|\W|^](?<releaseDate>((\d{2}){1,2}))((?<suffix>\]|\))|\W|$))

            auto regExpStr = R"((?<fulltext>(([\(\[]|^)|(?<!(\d|t)))(?<releaseDate>\d{2}|\d{4})(\D|\)|\]|$)))";
            auto regExp = QRegularExpression( regExpStr );
            auto match = regExp.match( fSearchName );
            if ( match.hasMatch() )
            {
                auto releaseDate = smartTrim( match.captured( "releaseDate" ) );
                if ( releaseDate != smartTrim( fSearchName ) )
                {
                    fReleaseDate = releaseDate;
                    bool aOK = false;
                    int tmp = fReleaseDate.toInt( &aOK );
                    bool tooOld = false;
                    if ( aOK )
                    {
                        tooOld = (fReleaseDate.length() == 4) && (tmp < 1900);
                    }
                    if ( !tooOld )
                        fSearchName.replace( match.capturedStart( "fulltext" ), match.capturedLength( "fulltext" ), "" );
                    else
                        fReleaseDate.clear();
                }
            }
            if ( fSearchResultInfo )
                fReleaseDate = fSearchResultInfo->fReleaseDate;
        }

        bool SSearchTMDBInfo::isTVMedia() const
        {
            return isTVType( fMediaType.first );
        }

        bool SSearchTMDBInfo::isRippedFromMKV( const QFileInfo & fi, int * titleNum )
        {
            return isRippedFromMKV( fi.fileName(), titleNum );
        }
         
        bool SSearchTMDBInfo::isRippedFromMKV( const QString & name, int * titleNum )
        {
            auto regExpStr = "^.*_t(?<num>\\d+)\\.mkv$";
            auto regExp = QRegularExpression( regExpStr );
            auto match = regExp.match( name );
            bool aOK = false;
            if ( match.hasMatch() )
            {
                auto titleNumStr = match.captured( "num" );
                int lclTitleNum = titleNumStr.toInt( &aOK );
                if ( !aOK )
                    lclTitleNum = -1;
                if ( titleNum )
                    *titleNum = lclTitleNum;
            }
            return aOK;
        }

        bool SSearchTMDBInfo::hasDiskNumber( QString & searchString, int & diskNum, std::shared_ptr< STransformResult > searchResultInfo )
        {
            QString diskStr;
            auto regExpStr = "[^A-Za-z](?<fulltext>D(ISC|ISK)?_?(?<num>\\d+))(\\D|$)";
            auto regExp = QRegularExpression( regExpStr, QRegularExpression::CaseInsensitiveOption );
            auto match = regExp.match( searchString );
            if ( match.hasMatch() )
            {
                diskStr = match.captured( "num" );
                searchString.replace( match.capturedStart( "fulltext" ), match.capturedLength( "fulltext" ), "" );
            }
            if ( searchResultInfo && !searchResultInfo->fDiskNum.isEmpty() )
                diskStr = searchResultInfo->fDiskNum;
            if ( !diskStr.isEmpty() )
            {
                bool aOK;
                diskNum = diskStr.toInt( &aOK );
                if ( !aOK )
                    diskNum = -1;
                return aOK;
            }
            return false;
        }

        void SSearchTMDBInfo::extractDiskNum()
        {
            hasDiskNumber( fSearchName, fDiskNum, fSearchResultInfo );
        }

        void SSearchTMDBInfo::extractTVInfo()
        {
            QString seasonStr;
            QString episodeStr;
            if ( isTVMedia() )
            {
                looksLikeTVShow( fSearchName, &fSearchName, &seasonStr, &episodeStr, &fSubTitle );
                fSubTitle = smartTrim( fSubTitle, true );
            }
            if ( fSearchResultInfo )
            {
                episodeStr = fSearchResultInfo->fEpisode;
                seasonStr = fSearchResultInfo->fSeason;
                fSubTitle = fSearchResultInfo->fSubTitle;
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
        }

        void SSearchTMDBInfo::extractTMDBID()
        {
            auto regExp = QRegularExpression( R"((?<fulltext>\[tmdbid=(?<tmdbid>\d+)\]))" );
            auto match = regExp.match( fSearchName );
            if ( match.hasMatch() )
            {
                fTMDBID = smartTrim( match.captured( "tmdbid" ) );
                fSearchName.replace( match.capturedStart( "fulltext" ), match.capturedLength( "fulltext" ), "" );
            }
            if ( fSearchResultInfo )
                fTMDBID = fSearchResultInfo->fTMDBID;
        }

        std::optional< std::pair< QUrl, ESearchType > > SSearchTMDBInfo::getSearchURL() const
        {
            QUrl url;
            url.setScheme( "https" );
            url.setHost( "api.themoviedb.org" );
            if ( fSearchByName )
            {
                if ( isTVMedia() )
                    url.setPath( "/3/search/tv" );
                else
                    url.setPath( "/3/search/movie" );

                QUrlQuery query;
                query.addQueryItem( "api_key", CSearchTMDB::apiKeyV3() );

                query.addQueryItem( "include_adult", "true" );
                if ( !fReleaseDate.isEmpty() )
                    query.addQueryItem( "year", fReleaseDate );
                auto searchStrings = fSearchName.split( QRegularExpression( "[\\s\\.]" ), TSkipEmptyParts );

                if ( searchStrings.isEmpty() )
                    return {};

                query.addQueryItem( "query", searchStrings.join( "+" ) );
                url.setQuery( query );

                //qDebug() << url.toString();
                return std::make_pair( url, isTVMedia() ? ESearchType::eSearchTV : ESearchType::eSearchMovie );
            }
            else if ( isTVMedia() ) // by tmdbid
            {
                if ( fTMDBID.isEmpty() )
                    return {};

                url.setPath( QString( "/3/tv/%1" ).arg( fTMDBID ) );

                QUrlQuery query;
                query.addQueryItem( "api_key", CSearchTMDB::apiKeyV3() );

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
                query.addQueryItem( "api_key", CSearchTMDB::apiKeyV3() );

                url.setQuery( query );
                //qDebug() << url.toString();
                return std::make_pair( url, ESearchType::eGetMovie );
            }
        }
    }
}
