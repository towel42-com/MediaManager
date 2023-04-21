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

#include "SearchTMDBInfo.h"
#include "TransformResult.h"
#include "Preferences/Core/Preferences.h"
#include "SearchTMDB.h"

#include <QString>
#include "SABUtils/HashUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/StringUtils.h"
#include "SABUtils/utils.h"

#include <QRegularExpression>
#include <QDebug>
#include <QUrlQuery>
#include <QFileInfo>

namespace std
{
    template<>
    struct hash< std::tuple< QString, bool, bool > >
    {
        std::size_t operator()( const std::tuple< QString, bool, bool > &key ) const { return NSABUtils::HashCombine( key ); }
    };

    template<>
    struct hash< std::pair< QString, bool > >
    {
        std::size_t operator()( const std::pair< QString, bool > &key ) const { return NSABUtils::HashCombine( key ); }
    };
}

namespace NMediaManager
{
    namespace NCore
    {
        SSearchTMDBInfo::SSearchTMDBInfo( const QString &text, std::shared_ptr< CTransformResult > searchResult )
        {
            fSearchResult = searchResult;
            fInitSearchString = text;
            fMediaType = std::make_pair( looksLikeTVShow( text, nullptr ), true );
            updateSearchCriteria( true );
        }

        SSearchTMDBInfo::SSearchTMDBInfo() :
            fMediaType( std::make_pair( EMediaType::eUnknownType, true ) )
        {
        }

        QString SSearchTMDBInfo::replaceKnownAbbreviations( const QString &string )
        {
            QString retVal = string;
            auto knownAbbreviations = NPreferences::NCore::CPreferences::instance()->getKnownAbbreviations();
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
            auto regExs = NPreferences::NCore::CPreferences::instance()->getKnownStringRegExs();
            for ( auto &&ii : regExs )
            {
                auto regEx = QRegularExpression( ii, QRegularExpression::CaseInsensitiveOption );
                auto match = regEx.match( retVal );
                while ( match.hasMatch() )
                {
                    auto start = match.capturedStart( "prefix" );
                    if ( start == -1 )
                        start = match.capturedStart( "word" );
                    auto end = match.capturedEnd( "suffix" );
                    if ( end == -1 )
                        end = match.capturedEnd( "word" );

                    retVal.remove( start, end - start );

                    match = regEx.match( retVal, start );
                }
            }
            return retVal;
        }

        QString SSearchTMDBInfo::stripExistingExtraInfo( const QString &string, QString &extendedData )
        {
            auto regExStr = R"([\)\]]\s*(?<total>( - )(?<extendedData>[^\[\(]+))$)";
            auto regEx = QRegularExpression( regExStr );
            QString retVal = string;
            auto match = regEx.match( retVal );
            if ( match.hasMatch() )
            {
                extendedData = match.captured( "extendedData" );
                retVal.remove( match.capturedStart( "total" ), match.capturedLength( "total" ) );
            }
            return retVal;
        }

        QString SSearchTMDBInfo::stripKnownExtendedData( const QString &string, QString &extendedData )
        {
            QString retVal = string;
            auto knownStrings = NPreferences::NCore::CPreferences::instance()->getKnownExtendedStrings();
            for ( auto &&knownString : knownStrings )
            {
                knownString = QRegularExpression::escape( knownString );
                knownString = knownString.replace( R"(\ )", "." );

                auto regExpStr = "\\W(?<word>" + knownString + ")(\\W|$)";
                auto regExp = QRegularExpression( regExpStr, QRegularExpression::CaseInsensitiveOption );
                auto match = regExp.match( retVal );
                if ( match.hasMatch() )
                {
                    retVal.remove( match.capturedStart( "word" ), match.capturedLength( "word" ) );
                    extendedData = NSABUtils::NStringUtils::transformTitle( match.captured( "word" ), true );
                    break;
                }
            }
            return retVal;
        }

        QString SSearchTMDBInfo::smartTrim( const QString &string, bool stripInnerSeparators, bool checkForKnownHyphens )
        {
            if ( string.isEmpty() )
                return string;

            static std::unordered_map< std::tuple< QString, bool, bool >, QString > sCache;

            auto cachePos = sCache.find( std::make_tuple( string, stripInnerSeparators, checkForKnownHyphens ) );
            if ( cachePos != sCache.end() )
            {
                return ( *cachePos ).second;
            }

            auto retVal = string;
            auto pos = retVal.indexOf( QRegularExpression( R"([^\.\s\-\_])" ) );
            if ( pos != -1 )
                retVal = retVal.mid( pos );

            pos = retVal.lastIndexOf( QRegularExpression( R"([^\.\s\-\_])" ) );
            if ( pos != -1 )
                retVal = retVal.left( pos + 1 );
            if ( stripInnerSeparators )
            {
                retVal.replace( QRegularExpression( R"(\:)" ), "" );

                QString prev;
                while ( prev != retVal )
                {
                    prev = retVal;
                    retVal.replace( QRegularExpression( checkForKnownHyphens ? R"(\s+|_|\.)" : R"(\s+|-|_|\.)" ), " " );
                }
                retVal = retVal.trimmed();

                if ( checkForKnownHyphens )
                {
                    auto knownHyphens = NPreferences::NCore::CPreferences::instance()->getKnownHyphenatedData();
                    int from = 0;
                    auto pos = retVal.indexOf( '-' );
                    while ( pos != -1 )
                    {
                        bool isKnown = false;
                        from = pos + 1;
                        for ( auto &&ii : knownHyphens )
                        {
                            auto currStr = retVal.mid( pos - ii.second, ii.first.length() );
                            if ( currStr.compare( ii.first, Qt::CaseInsensitive ) == 0 )
                            {
                                isKnown = true;
                                from = pos + ii.first.length();
                                break;
                            }
                        }
                        if ( !isKnown )
                            retVal.replace( pos, ' ' );
                        pos = retVal.indexOf( '-', from );
                    }
                }
            }
            sCache[ std::make_tuple( string, stripInnerSeparators, checkForKnownHyphens ) ] = retVal;
            return retVal;
        }

        QStringList SSearchTMDBInfo::stripOutPositions( const QString &inString, const std::list< std::pair< int, int > > &positions )
        {
            QStringList retVal;
            int posStart = 0;
            for ( const auto &position : positions )
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

        EMediaType SSearchTMDBInfo::looksLikeTVShow( const QString &searchString, QString *titleStr, QString *seasonStr, QString *episodeStr, QString *extraStr, bool assumeUnknownIsMovie )
        {
            static std::unordered_map< std::pair< QString, bool >, std::tuple< EMediaType, QString, QString, QString, QString > > sCache;
            auto pos = sCache.find( std::make_pair( searchString, assumeUnknownIsMovie ) );
            if ( pos != sCache.end() )
            {
                if ( titleStr )
                    *titleStr = std::get< 1 >( ( *pos ).second );
                if ( seasonStr )
                    *seasonStr = std::get< 2 >( ( *pos ).second );
                if ( episodeStr )
                    *episodeStr = std::get< 3 >( ( *pos ).second );
                if ( extraStr )
                    *extraStr = std::get< 4 >( ( *pos ).second );
                return std::get< 0 >( ( *pos ).second );
            }

            if ( titleStr )
                titleStr->clear();
            if ( seasonStr )
                seasonStr->clear();
            if ( episodeStr )
                episodeStr->clear();
            if ( extraStr )
                extraStr->clear();

            QString title = searchString;
            QString season;
            QString episode;
            QString extra;
            EMediaType retVal = EMediaType::eUnknownType;   // default is a movie

            auto regExpStr = QString( R"((^|[^A-Z])S(?<garbage>EASON(_?))?\s*(?<season>\d{1,4}))" );
            auto regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            auto match = regExp.match( title );
            std::list< std::pair< int, int > > positions;
            if ( match.hasMatch() )
            {
                season = smartTrim( match.captured( "season" ) );
                if ( match.capturedStart( "garbage" ) != -1 )
                    positions.emplace_back( match.capturedStart( "garbage" ) - 1, match.capturedLength( "garbage" ) + 1 );
                positions.emplace_back( match.capturedStart( "season" ) - 1, match.capturedLength( "season" ) + 1 );

                retVal = EMediaType::eTVSeason;
            }

            auto regExpStr1 = R"((^|[^A-Z])E(?<garbage1>PISODE)?(?<startEpisode>\d{1,4})(?<dash>\-)?E(?<garbage2>PISODE)?(?<endEpisode>\d{1,4}))";
            auto regExpStr2 = R"((^|[^A-Z])E(?<garbage>PISODE)?(?<episode>\d{1,4})(?![E-]))";
            regExpStr = QString( R"(((%1)|(%2)))" ).arg( regExpStr1 ).arg( regExpStr2 );
            regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            match = regExp.match( title );
            if ( match.hasMatch() )
            {
                if ( !match.captured( "episode" ).isEmpty() )
                {
                    episode = smartTrim( match.captured( "episode" ) );

                    if ( match.capturedStart( "garbage" ) != -1 )
                        positions.emplace_back( match.capturedStart( "garbage" ) - 1, match.capturedLength( "garbage" ) + 1 );

                    auto pos = std::make_pair( match.capturedStart( "episode" ) - 1, match.capturedLength( "episode" ) + 1 );

                    if ( positions.empty() || ( positions.front().first < match.capturedStart( "episode" ) ) )
                        positions.push_back( pos );
                    else
                        positions.push_front( pos );
                }
                else if ( !match.captured( "startEpisode" ).isEmpty() && !match.captured( "endEpisode" ).isEmpty() )
                {
                    episode = smartTrim( "E" + match.captured( "startEpisode" ) + match.captured( "dash" ) + "E" + match.captured( "endEpisode" ) );

                    if ( match.capturedStart( "garbage1" ) != -1 )
                        positions.emplace_back( match.capturedStart( "garbage1" ) - 1, match.capturedLength( "garbage1" ) + 1 );

                    auto pos = std::make_pair( match.capturedStart( "startEpisode" ) - 1, match.capturedLength( "startEpisode" ) + 1 );

                    if ( positions.empty() || ( positions.front().first < match.capturedStart( "startEpisode" ) ) )
                        positions.push_back( pos );
                    else
                        positions.push_front( pos );

                    if ( match.capturedStart( "garbage2" ) != -1 )
                        positions.emplace_back( match.capturedStart( "garbage2" ) - 1, match.capturedLength( "garbage2" ) + 1 );

                    pos = std::make_pair( match.capturedStart( "endEpisode" ) - 1, match.capturedLength( "endEpisode" ) + 1 );

                    if ( positions.empty() || ( positions.front().first < match.capturedStart( "endEpisode" ) ) )
                        positions.push_back( pos );
                    else
                        positions.push_front( pos );
                }
                retVal = EMediaType::eTVEpisode;
            }

            regExpStr = QString( R"((^|[^A-Z])(?<season>\d{1,4})[xX](?<episode>\d{1,4}))" );
            regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            match = regExp.match( title );
            if ( match.hasMatch() )
            {
                season = smartTrim( match.captured( "season" ) );
                episode = smartTrim( match.captured( "episode" ) );
                positions.emplace_back( match.capturedStart( "season" ) - 1, match.capturedLength( "season" ) + 1 );
                positions.emplace_back( match.capturedStart( "episode" ) - 1, match.capturedLength( "episode" ) + 1 );

                retVal = EMediaType::eTVSeason;
            }

            auto data = stripOutPositions( title, positions );
            if ( !data.isEmpty() )
            {
                title = data.front();
                data.pop_front();
                extra = data.join( " " );
            }

            regExpStr = R"(.*\s??(?<seasonsuffix>-\s??Season\s?(?<season>\d+)))";
            regExp = QRegularExpression( regExpStr, QRegularExpression::PatternOption::CaseInsensitiveOption );
            match = regExp.match( title );
            if ( match.hasMatch() )
            {
                season = smartTrim( match.captured( ( "season" ) ) );
                title.replace( match.capturedStart( "seasonsuffix" ), match.capturedEnd( "seasonsuffix" ), "" );
                retVal = EMediaType::eTVSeason;
            }

            if ( titleStr )
                *titleStr = title;

            if ( assumeUnknownIsMovie && ( retVal == EMediaType::eUnknownType ) )
                retVal = EMediaType::eMovie;
            if ( seasonStr )
                *seasonStr = season;
            if ( episodeStr )
                *episodeStr = episode;
            if ( extraStr )
                *extraStr = extra;

            sCache[ std::make_pair( searchString, assumeUnknownIsMovie ) ] = std::make_tuple( retVal, title, season, episode, extra );
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

            fSearchName = smartTrim( fSearchName, true, true );

            if ( fSearchResult && !fSearchResult->isAutoSetText() )
            {
                fSearchName = fSearchResult->getTitle();
            }

            if ( updateSearchBy )
                fSearchByName = fTMDBID.isEmpty();
        }

        void SSearchTMDBInfo::setPageNumber( int pageNumber )
        {
            if ( pageNumber == -1 )
                fPageNumber.reset();
            else
                fPageNumber = pageNumber;
        }

        void SSearchTMDBInfo::setReleaseDate( const QString &releaseDate )
        {
            fReleaseDate.second = releaseDate;
            NSABUtils::SDateSearchOptions options;
            options.fAllowYearOnly = true;
            options.fAllowMonthYearOnly = true;
            fReleaseDate.first = NSABUtils::getDate( releaseDate, options );
        }

        int SSearchTMDBInfo::releaseYear( const QString &dateStr, bool *aOK )
        {
            NSABUtils::SDateSearchOptions options;
            options.fAllowYearOnly = true;
            options.fAllowMonthYearOnly = true;

            auto dt = NSABUtils::getDate( dateStr, options );
            auto lclAOK = dt.isValid();
            int retVal = 0;
            if ( lclAOK )
                retVal = dt.year();

            if ( aOK )
                *aOK = aOK;
            return retVal;
        }

        int SSearchTMDBInfo::releaseYear( bool *aOK ) const
        {
            if ( aOK )
                *aOK = fReleaseDate.first.isValid();
            return fReleaseDate.first.year();
        }

        int SSearchTMDBInfo::tmdbID( bool *aOK ) const
        {
            return fTMDBID.toInt( aOK );
        }

        QString SSearchTMDBInfo::episodeString( bool forDebug ) const
        {
            if ( fEpisodes.empty() )
                return forDebug ? "<Not Set>" : QString();

            auto groupedEpisodes = NSABUtils::group( fEpisodes );
            QString retVal;
            //auto pos = fEpisodes.begin();
            return QString();
        }

        QDebug operator<<( QDebug debug, const SSearchTMDBInfo &info )
        {
            debug << info.toString( true );
            return debug;
        }

        QString SSearchTMDBInfo::toString( bool forDebug ) const
        {
            auto retVal =
                forDebug ? QString( "SSearchTMDBInfo(%1 (%2)-S%3E%4-%5-%6-%7)" ) : QString( "Search Name: '%1' - Release Date: %2 - Season: %3 - Episode: %4 - TMDB ID: %5 - Media Type: %6 Auto Determined: %7 - Exact Match Only: %8" );

            retVal = retVal.arg( searchName() )
                         .arg( forDebug ? releaseDate().second : ( releaseDate().second.isEmpty() ? "<Not Set>" : releaseDate().second ) )
                         .arg(
                             forDebug             ? QString::number( season() )
                             : ( season() == -1 ) ? "<Not Set>"
                                                  : QString::number( season() ) )
                         .arg( episodeString( forDebug ) )
                         .arg(
                             forDebug                   ? tmdbIDString()
                             : tmdbIDString().isEmpty() ? "<Not Set>"
                                                        : tmdbIDString() )
                         .arg( toEnumString( fMediaType.first ) )
                         .arg( fMediaType.second ? "Yes" : "No" )
                         .arg(
                             forDebug           ? QString( "%1" ).arg( exactMatchOnly() )
                             : exactMatchOnly() ? "Yes"
                                                : "No" );

            return retVal;
        }

        bool SSearchTMDBInfo::isMatch( std::shared_ptr< CTransformResult > searchResult ) const
        {
            if ( isTVMedia() )
            {
                return isMatch( searchResult->getShowFirstAirDate(), searchResult->getTMDBID(), searchResult->getTitle(), searchResult->mediaType(), searchResult->getSeason(), searchResult->getEpisode() )
                       || isMatch( searchResult->getSeasonStartDate(), searchResult->getTMDBID(), searchResult->getTitle(), searchResult->mediaType(), searchResult->getSeason(), searchResult->getEpisode() )
                       || isMatch( searchResult->getEpisodeAirDate(), searchResult->getTMDBID(), searchResult->getTitle(), searchResult->mediaType(), searchResult->getSeason(), searchResult->getEpisode() );
            }
            else
                return isMatch( searchResult->getMovieReleaseDate(), searchResult->getTMDBID(), searchResult->getTitle(), searchResult->mediaType(), searchResult->getSeason(), searchResult->getEpisode() );
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
            bool aOK = false;
            auto season = seasonMatch.toInt( &aOK );
            if ( !aOK )
                return false;
            return isSeasonMatch( season );
        }

        bool SSearchTMDBInfo::isEpisodeMatch( const std::list< int > &episodeMatch ) const
        {
            if ( episodeMatch.empty() )
                return fEpisodes.empty();
            if ( fEpisodes.empty() )
                return false;
            return episodeMatch == fEpisodes;
        }

        bool SSearchTMDBInfo::isEpisodeMatch( const QString &episodeStr ) const
        {
            if ( episodeStr.isEmpty() )
                return fEpisodes.empty();

            bool aOK = false;
            auto episodes = episodesFromString( episodeStr, aOK );
            if ( !aOK )
                episodes.clear();
            return isEpisodeMatch( episodes );
        }

        bool SSearchTMDBInfo::isMatchingTMDBID( const QString &inTMDBID ) const
        {
            if ( inTMDBID.isEmpty() )
                return false;

            bool aOK = false;
            int tmdbid = inTMDBID.toInt( &aOK );
            if ( !aOK )
                tmdbid = -1;

            return isMatchingTMDBID( tmdbid );
        }

        bool SSearchTMDBInfo::isMatchingTMDBID( int tmdbid ) const
        {
            if ( isTVMedia() )   // dont check for TV shows, as the TMDB could be the episode ID or season ID
                return true;

            if ( !tmdbIDSet() )
                return true;

            if ( tmdbid == -1 )
                return false;

            bool aOK = false;
            int myTmdbID = tmdbID( &aOK );
            if ( !aOK )
                return false;

            return tmdbid == myTmdbID;
        }

        bool SSearchTMDBInfo::isMatchingDate( const std::pair< QDate, QString > &releaseDate ) const
        {
            if ( !releaseDateSet() )
                return true;

            if ( fReleaseDate.first.isValid() != releaseDate.first.isValid() )
                return false;

            if ( !fReleaseDate.first.isValid() )
                return true;

            if ( ( ( fReleaseDate.first.month() == 1 ) && ( fReleaseDate.first.day() == 1 ) ) || ( ( releaseDate.first.month() == 1 ) && ( releaseDate.first.day() == 1 ) ) )
            {
                return fReleaseDate.first.year() == releaseDate.first.year();
            }

            if ( fExactMatchOnly )
            {
                return fReleaseDate.first.year() == releaseDate.first.year() && fReleaseDate.first.month() == releaseDate.first.month();
            }

            return true;
        }

        bool SSearchTMDBInfo::isMatchingName( const QString &name ) const
        {
            if ( !fSearchByName )
                return true;
            return NSABUtils::NStringUtils::isSimilar( name, fSearchName, fExactMatchOnly );   // if every word we are searching for is covered by name, we match.  For exact matches must be in same order
        }

        void SSearchTMDBInfo::extractReleaseDate()
        {
            //basically capture anything inside parens that doesnt start with imdb
            auto regExpStr1 = R"((?<releaseDate1>\d{2}|\d{4}))";
            auto regExpStr = QString( R"((?<fulltext>\(%1\)))" ).arg( regExpStr1 );
            auto regExp = QRegularExpression( regExpStr );
            Q_ASSERT( regExp.isValid() );
            auto match = regExp.match( fSearchName );

            if ( !match.hasMatch() )
            {
                auto regExpStr2 = R"((([\(\[])\s*(?<!(tv|im|tm)dbid\=))(?<releaseDate2>[^\(\[\)\]]+)\s*(\)|\]))";
                auto regExpStr3 = R"((([\(\[]|^)|(?<!(\d|t)))(?<releaseDate3>\d{2}|\d{4})([^0-9sS]|\)|\]|$))";
                regExpStr = QString( "(?<fulltext>(%2|%3))" ).arg( regExpStr2 ).arg( regExpStr3 );

                regExp = QRegularExpression( regExpStr );
                Q_ASSERT( regExp.isValid() );
                match = regExp.match( fSearchName );
            }

            if ( match.hasMatch() )
            {
                auto releaseDate = smartTrim( match.captured( "releaseDate1" ) );
                if ( releaseDate.isEmpty() )
                    releaseDate = smartTrim( match.captured( "releaseDate2" ) );
                if ( releaseDate.isEmpty() )
                    releaseDate = smartTrim( match.captured( "releaseDate3" ) );

                if ( releaseDate != smartTrim( fSearchName ) )
                {
                    NSABUtils::SDateSearchOptions options;
                    options.fAllowYearOnly = true;
                    options.fAllowMonthYearOnly = true;
                    auto date = NSABUtils::getDate( releaseDate, options );
                    if ( date.isValid() )
                    {
                        if ( date.year() < 1900 )
                            fReleaseDate = {};
                        else
                        {
                            fReleaseDate = { date, releaseDate };
                            fSearchName.replace( match.capturedStart( "fulltext" ), match.capturedLength( "fulltext" ), "" );
                        }
                    }
                }
            }
            if ( fSearchResult )
                fReleaseDate = fSearchResult->getDate();
        }

        bool SSearchTMDBInfo::isTVMedia() const
        {
            return isTVType( fMediaType.first );
        }

        bool SSearchTMDBInfo::isRippedWithMKV( const QFileInfo &fi, int *titleNum )
        {
            return isRippedWithMKV( fi.fileName(), titleNum );
        }

        bool SSearchTMDBInfo::isRippedWithMKV( const QString &name, int *titleNum )
        {
            static std::unordered_map< QString, std::pair< bool, int > > sCache;
            auto pos = sCache.find( name );
            if ( pos != sCache.end() )
            {
                if ( titleNum )
                    *titleNum = ( *pos ).second.second;
                return ( *pos ).second.first;
            }

            auto regExpStr = NPreferences::NCore::CPreferences::instance()->getRippedWithMakeMKVRegEX();
            auto regExp = QRegularExpression( regExpStr );
            auto match = regExp.match( name );
            bool aOK = false;
            int lclTitleNum = -1;
            if ( match.hasMatch() )
            {
                auto titleNumStr = match.captured( "num" );
                lclTitleNum = titleNumStr.toInt( &aOK );
                if ( !aOK )
                    lclTitleNum = -1;
                if ( titleNum )
                    *titleNum = lclTitleNum;
            }
            sCache[ name ] = std::make_pair( aOK, lclTitleNum );
            return aOK;
        }

        bool SSearchTMDBInfo::hasDiskNumber( QString &searchString, int &diskNum, std::shared_ptr< CTransformResult > searchResult )
        {
            //  insert cache cache
            static std::unordered_map< QString, std::tuple< bool, QString, int > > sCache;
            auto pos = sCache.find( searchString );
            if ( pos != sCache.end() )
            {
                searchString = std::get< 1 >( ( *pos ).second );
                diskNum = std::get< 2 >( ( *pos ).second );
                return std::get< 0 >( ( *pos ).second );
            }

            QString diskStr;
            auto regExpStr = "[^A-Za-z](?<fulltext>D(ISC|ISK)?_?(?<num>\\d+))(\\D|$)";
            auto regExp = QRegularExpression( regExpStr, QRegularExpression::CaseInsensitiveOption );
            auto match = regExp.match( searchString );
            if ( match.hasMatch() )
            {
                diskStr = match.captured( "num" );
                searchString.replace( match.capturedStart( "fulltext" ), match.capturedLength( "fulltext" ), "" );
            }
            if ( searchResult && !searchResult->diskNum().isEmpty() )
                diskStr = searchResult->diskNum();

            bool aOK = false;
            if ( !diskStr.isEmpty() )
            {
                diskNum = diskStr.toInt( &aOK );
                if ( !aOK )
                    diskNum = -1;
            }

            sCache[ searchString ] = std::make_tuple( aOK, searchString, diskNum );
            return aOK;
        }

        void SSearchTMDBInfo::extractDiskNum()
        {
            hasDiskNumber( fSearchName, fDiskNum, fSearchResult );
        }

        void SSearchTMDBInfo::extractTVInfo()
        {
            QString seasonStr;
            QString episodeStr;
            if ( isTVMedia() )
            {
                auto tmpSearchString = fSearchName;
                looksLikeTVShow( fSearchName, &tmpSearchString, &seasonStr, &episodeStr, &fSubTitle );
                fSearchName = tmpSearchString;
                fSubTitle = smartTrim( fSubTitle, true );
            }

            if ( fSearchResult && !fSearchResult->isNoMatch() )
            {
                episodeStr = fSearchResult->getEpisode();
                seasonStr = fSearchResult->getSeason();
                fSubTitle = fSearchResult->getSubTitle();
            }

            if ( !episodeStr.isEmpty() )
            {
                bool aOK = false;
                fEpisodes = episodesFromString( episodeStr, aOK );
                if ( !aOK )
                    fEpisodes.clear();
            }

            if ( !seasonStr.isEmpty() )
            {
                bool aOK = false;
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
            if ( fSearchResult )
                fTMDBID = fSearchResult->getTMDBID();
        }

        bool SSearchTMDBInfo::canSearch() const
        {
            if ( CSearchTMDB::apiKeyV3().isEmpty() )
                return false;

            if ( fSearchByName )
                return !getSearchStrings().isEmpty();
            else
                return !fTMDBID.isEmpty();
        }

        std::optional< std::pair< QUrl, ESearchType > > SSearchTMDBInfo::getSearchURL() const
        {
            QUrl url;
            url.setScheme( "https" );
            url.setHost( "api.themoviedb.org" );
            if ( fSearchByName )
            {
                auto searchStrings = getSearchStrings();
                if ( searchStrings.isEmpty() )
                    return {};

                if ( isTVMedia() )
                    url.setPath( "/3/search/tv" );
                else
                    url.setPath( "/3/search/movie" );

                QUrlQuery query;
                query.addQueryItem( "api_key", CSearchTMDB::apiKeyV3() );

                query.addQueryItem( "include_adult", "true" );
                if ( fReleaseDate.first.isValid() )
                    query.addQueryItem( "year", QString::number( fReleaseDate.first.year() ) );
                if ( fPageNumber.has_value() )
                    query.addQueryItem( "page", QString::number( fPageNumber.value() ) );

                query.addQueryItem( "query", searchStrings.join( "+" ) );
                url.setQuery( query );

                //qDebug() << url.toString();
                return std::make_pair( url, isTVMedia() ? ESearchType::eSearchTV : ESearchType::eSearchMovie );
            }
            else if ( isTVMedia() )   // by tmdbid
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

        QStringList SSearchTMDBInfo::getSearchStrings() const
        {
            return fSearchName.split( QRegularExpression( R"([\s\.])" ), NSABUtils::NStringUtils::TSkipEmptyParts );
        }

        std::list< int > SSearchTMDBInfo::episodesFromString( const QString &episodeStr, bool &aOK ) const
        {
            return NSABUtils::intsFromString( episodeStr, QString( R"((E|Episode\s*)?)" ), true, &aOK );
        }
    }
}
