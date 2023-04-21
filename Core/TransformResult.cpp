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

#include "TransformResult.h"
#include "SearchTMDBInfo.h"
#include "PatternInfo.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/utils.h"

#include <QRegularExpression>
#include <QFileInfo>

namespace NMediaManager
{
    namespace NCore
    {
        CTransformResult::CTransformResult( EMediaType type ) :
            fMediaType( type )
        {
        }

        void CTransformResult::mergeEpisodeResults( const std::shared_ptr< CTransformResult > &rhs )
        {
            if ( isTVShow() != rhs->isTVShow() )
                return;
            if ( fShowTMDBID != rhs->fShowTMDBID )
                return;
            if ( fSeasonTMDBID != rhs->fSeasonTMDBID )
                return;
            if ( fEpisode == rhs->fEpisode )
                return;

            fExtraEpisodes.push_back( rhs );
        }

        QString CTransformResult::getTitle() const
        {
            if ( isDeleteResult() )
                return getDeleteThis();
            if ( isNotFoundResult() )
                return getNoMatch();
            return NSABUtils::NStringUtils::transformTitle( title() );
        }

        std::pair< QDate, QString > CTransformResult::getMovieReleaseDate() const
        {
            if ( fMovieReleaseDate.second.isEmpty() || !fMovieReleaseDate.first.isValid() )
            {
                auto parent = fParent.lock();
                if ( parent )
                    return parent->getMovieReleaseDate();
            }
            return fMovieReleaseDate;
        }

        std::pair< QDate, QString > CTransformResult::getShowFirstAirDate() const
        {
            if ( fShowFirstAirDate.second.isEmpty() || !fShowFirstAirDate.first.isValid() )
            {
                auto parent = fParent.lock();
                if ( parent )
                    return parent->getShowFirstAirDate();
            }
            return fShowFirstAirDate;
        }

        std::pair< QDate, QString > CTransformResult::getSeasonStartDate() const
        {
            if ( fSeasonStartDate.second.isEmpty() || !fSeasonStartDate.first.isValid() )
            {
                auto parent = fParent.lock();
                if ( parent )
                    return parent->getSeasonStartDate();
            }
            return fSeasonStartDate;
        }

        std::pair< QDate, QString > CTransformResult::getEpisodeAirDate() const
        {
            if ( fEpisodeAirDate.second.isEmpty() || !fEpisodeAirDate.first.isValid() )
            {
                auto parent = fParent.lock();
                if ( parent )
                    return parent->getEpisodeAirDate();
            }
            return fEpisodeAirDate;
        }

        std::pair< QDate, QString > CTransformResult::getDate() const
        {
            switch ( mediaType() )
            {
                case EMediaType::eMovie:
                    return getMovieReleaseDate();
                case EMediaType::eTVShow:
                    return getShowFirstAirDate();
                case EMediaType::eTVSeason:
                    return getSeasonStartDate();
                case EMediaType::eTVEpisode:
                    return getEpisodeAirDate();
                default:
                    return {};
            }
        }
        //QString CTransformResult::getInitialYear() const
        //{
        //    if ( ( fMediaType != EMediaType::eTVEpisode )
        //         && ( fMediaType != EMediaType::eTVSeason ) )
        //    {
        //        return getYear();
        //    }

        //    auto tvShowInfo = getTVShowInfo();
        //    if ( !tvShowInfo )
        //        return getYear();
        //    return tvShowInfo->getYear();
        //}

        const CTransformResult *CTransformResult::getTVShowInfo() const
        {
            if ( ( mediaType() == EMediaType::eTVEpisode ) || ( mediaType() == EMediaType::eTVSeason ) )
            {
                auto parent = fParent.lock();
                if ( parent )
                    return parent->getTVShowInfo();
                else
                    return nullptr;
            }
            return this;
        }

        void CTransformResult::setMovieReleaseDate( const QString &date )
        {
            fMovieReleaseDate = { NSABUtils::getDate( date ), date };
        }

        void CTransformResult::setShowFirstAirDate( const QString &date )
        {
            fShowFirstAirDate = { NSABUtils::getDate( date ), date };
        }

        void CTransformResult::setSeasonStartDate( const QString &date )
        {
            fSeasonStartDate = { NSABUtils::getDate( date ), date };
        }

        void CTransformResult::setEpisodeAirDate( const QString &date )
        {
            fEpisodeAirDate = { NSABUtils::getDate( date ), date };
        }

        QString CTransformResult::getYear() const
        {
            switch ( mediaType() )
            {
                case EMediaType::eMovie:
                    return getMovieReleaseYear();
                case EMediaType::eTVShow:
                    return getShowFirstAirYear();
                case EMediaType::eTVSeason:
                    return getSeasonStartYear();
                case EMediaType::eTVEpisode:
                    return getEpisodeAirYear();
                default:
                    return {};
            }
        }

        QString CTransformResult::getMovieReleaseYear() const
        {
            auto dt = getMovieReleaseDate().first;
            if ( !dt.isValid() )
                return {};
            return QString::number( dt.year() );
        }

        QString CTransformResult::getShowFirstAirYear() const
        {
            auto dt = getShowFirstAirDate().first;
            if ( !dt.isValid() )
                return {};
            return QString::number( dt.year() );
        }

        QString CTransformResult::getSeasonStartYear() const
        {
            auto dt = getSeasonStartDate().first;
            if ( !dt.isValid() )
                return {};
            return QString::number( dt.year() );
        }

        QString CTransformResult::getEpisodeAirYear() const
        {
            auto dt = getEpisodeAirDate().first;
            if ( !dt.isValid() )
                return {};
            return QString::number( dt.year() );
        }

        QString CTransformResult::getSubTitle() const
        {
            auto subTitles = QStringList( { subTitle() } );
            for ( auto &&ii : fExtraEpisodes )
                subTitles << ii->subTitle();

            return NSABUtils::NStringUtils::transformTitle( subTitles.join( "-" ) );
        }

        QString CTransformResult::getTMDBID() const
        {
            auto retVal = tmdbID();
            if ( !seasonTMDBID().isEmpty() )
                retVal = seasonTMDBID();
            if ( !episodeTMDBID().isEmpty() )
                retVal = episodeTMDBID();
            return retVal;
        }

        QString CTransformResult::getSeason() const
        {
            if ( !isTVShow() )
                return {};
            if ( ( mediaType() != EMediaType::eTVSeason ) && ( mediaType() != EMediaType::eTVEpisode ) )
                return {};
            return season();
        }

        QString CTransformResult::getEpisode() const
        {
            if ( !isTVShow() )
                return {};
            if ( mediaType() != EMediaType::eTVEpisode )
                return {};
            std::list< int > episodes;
            episodes.push_back( episode().toInt() );
            for ( auto &&ii : fExtraEpisodes )
                episodes.push_back( ii->episode().toInt() );

            auto groupedEpisodes = NSABUtils::group( episodes );
            QStringList episodeList;
            for ( auto &&ii : groupedEpisodes )
            {
                if ( ii.size() == 1 )
                {
                    episodeList << QString( "E%1" ).arg( ii.front(), 2, 10, QChar( '0' ) );
                }
                else
                {
                    auto curr = QString( "E%1%3E%2" ).arg( ii.front(), 2, 10, QChar( '0' ) ).arg( ii.back(), 2, 10, QChar( '0' ) );
                    curr = curr.arg( ( std::abs( ii.back() - ii.front() ) == 1 ) ? "" : "-" );
                    episodeList << curr;
                }
            }

            auto retVal = episodeList.join( "," );
            if ( retVal.startsWith( 'E' ) )
                retVal = retVal.mid( 1 );
            return retVal;
        }

        // do not include <> in the capture name
        QString replaceCapture( const QString &captureName, const QString &returnPattern, const QString &value )
        {
            if ( captureName.isEmpty() )
                return returnPattern;

            // see if the capture name exists in the return pattern
            auto capRegEx = QString( "\\<%1\\>" ).arg( captureName );
            auto regExp = QRegularExpression( capRegEx );

            int start = -1;
            int replLength = -1;

            auto match = regExp.match( returnPattern );
            if ( !match.hasMatch() )
                return returnPattern;
            else
            {
                start = match.capturedStart( 0 );
                replLength = match.capturedLength( 0 );
            }

            // its in there..now lets see if its optional
            auto optRegExStr = QString( R"(\{(?<replText>[^{}]+)\}\:%1)" ).arg( capRegEx );
            regExp = QRegularExpression( optRegExStr );
            match = regExp.match( returnPattern );
            bool optional = match.hasMatch();
            QString replText = value;
            if ( optional )
            {
                start = match.capturedStart( 0 );
                replLength = match.capturedLength( 0 );

                replText = match.captured( "replText" );
                if ( value.isEmpty() )
                    replText.clear();
                else
                {
                    replText = replaceCapture( captureName, replText, value );
                    ;
                }
            }
            auto retVal = returnPattern;
            retVal.replace( start, replLength, replText );
            return retVal;
        }

        QString CTransformResult::cleanFileName( const QString &inFile, bool isDir )
        {
            if ( inFile.isEmpty() )
                return inFile;

            QString retVal = inFile;
            retVal.replace( QRegularExpression( R"(^(([A-Za-z]\:)|(\/)|(\\))+)" ), "" );

            auto regExStr = QString( "(?<hours>\\d{1,2}):(?<minutes>\\d{2})" );
            retVal.replace( QRegularExpression( regExStr ), "\\1\\2" );

            regExStr = R"(\s*\:\s*)";
            retVal.replace( QRegularExpression( regExStr ), "- " );

            regExStr = R"([\:\<\>\"\|\?\*)";
            if ( !isDir )
                regExStr += R"(\/\\)";
            regExStr += "]";
            retVal.replace( QRegularExpression( regExStr ), "" );
            return retVal;
        }

        QString CTransformResult::cleanFileName( const QFileInfo &fi )
        {
            return cleanFileName( fi.completeBaseName(), fi.isDir() );
        }

        void CTransformResult::onAllChildren( std::function< void( std::shared_ptr< CTransformResult > child ) > func, std::function< bool() > stopFunc )
        {
            for ( auto &&ii : fChildren )
            {
                func( ii );
                if ( stopFunc() )
                    break;
            }
        }

        void CTransformResult::setTitle( const QString &val )
        {
            fTitle = val;

            auto regEx = QRegularExpression( R"((?<prefix>)\s*\:\s*(?<suffix>)\s*)" );
            auto match = regEx.match( fTitle );
            if ( match.hasMatch() )
            {
                fTitle = fTitle.mid( 0, match.capturedStart() ) + match.captured( "prefix" ) + "- " + match.captured( "suffix" ) + fTitle.mid( match.capturedEnd() );
            }
        }

        bool CTransformResult::operator==( const CTransformResult &rhs ) const
        {
            return ( title() == rhs.title() ) && ( fMovieReleaseDate == rhs.fMovieReleaseDate ) && ( fShowFirstAirDate == rhs.fShowFirstAirDate ) && ( fSeasonStartDate == rhs.fSeasonStartDate ) && ( fEpisodeAirDate == rhs.fEpisodeAirDate )
                   && ( tmdbID() == rhs.tmdbID() ) && ( seasonTMDBID() == rhs.seasonTMDBID() ) && ( episodeTMDBID() == rhs.episodeTMDBID() ) && ( season() == rhs.season() ) && ( fSeasonOnly == rhs.fSeasonOnly )
                   && ( episode() == rhs.episode() ) && ( subTitle() == rhs.subTitle() ) && ( extraInfo() == rhs.extraInfo() ) && ( diskNum() == rhs.diskNum() ) && ( description() == rhs.description() );
            // pixmap, parent, children and mediatype do not count
        }

        QString CTransformResult::transformedName( const QFileInfo &fileInfo, const SPatternInfo &patternInfo, bool titleOnly ) const
        {
            auto title = getTitle();
            auto releaseYear = getMovieReleaseYear();
            auto showYear = getShowFirstAirYear();
            auto seasonYear = getSeasonStartYear();
            auto episodeYear = getEpisodeAirYear();
            auto tmdbid = tmdbID();
            auto showTMDBID = this->showTMDBID();
            auto season = this->season();
            auto episode = this->getEpisode();
            auto extraInfo = this->extraInfo();
            auto episodeTitle = getSubTitle();

            QString retVal = fileInfo.isDir() ? patternInfo.dirPattern() : patternInfo.filePattern();
            retVal = replaceCapture( "title", retVal, title );
            retVal = replaceCapture( "year", retVal, releaseYear );
            retVal = replaceCapture( "show_year", retVal, showYear );
            retVal = replaceCapture( "season_year", retVal, seasonYear );
            retVal = replaceCapture( "episode_year", retVal, episodeYear );
            retVal = replaceCapture( "tmdbid", retVal, tmdbid );
            retVal = replaceCapture( "show_tmdbid", retVal, showTMDBID );
            retVal = replaceCapture( "season", retVal, QString( "%1" ).arg( season, fileInfo.isDir() ? 1 : 2, QChar( '0' ) ) );
            retVal = replaceCapture( "episode", retVal, episode );
            retVal = replaceCapture( "episode_title", retVal, episodeTitle );
            retVal = replaceCapture( "extra_info", retVal, extraInfo );

            retVal = cleanFileName( retVal, fileInfo.isDir() );
            if ( !titleOnly && !fileInfo.isDir() )
                retVal += "." + fileInfo.suffix();
            return retVal;
        }

        void CTransformResult::removeChild( std::shared_ptr< CTransformResult > info )
        {
            for ( auto &&ii = fChildren.begin(); ii != fChildren.end(); ++ii )
            {
                if ( ( *ii ).get() == info.get() )
                {
                    fChildren.erase( ii );
                    return;
                }
            }
        }

        QString CTransformResult::toString( bool forDebug ) const
        {
            QStringList tmp;
            if ( forDebug )
            {
                QStringList tmp;
                tmp << "InfoType: '" + NMediaManager::NCore::toEnumString( mediaType() ) + "'"
                    << "Title: '" + title() + "'"
                    << "Movie ReleaseDate: '" + fMovieReleaseDate.second + "'"
                    << "ShowFirstAirDate: '" + fShowFirstAirDate.second + "'"
                    << "SeasonStartDate: '" + fSeasonStartDate.second + "'"
                    << "EpisodeAirDate: '" + fEpisodeAirDate.second + "'"
                    << "TMDBID: '" + tmdbID() + "'"
                    << "Season TMBDID: '" + seasonTMDBID() + "'"
                    << "Episode TMDBID: '" + episodeTMDBID() + "'"
                    << "Season: '" + season() + "'" << QString( " Season Only? %1" ).arg( fSeasonOnly ? "Yes" : "No" ) << "Episode: '" + episode() + "'"
                    << "Sub Title: '" + subTitle() + "'"
                    << "ExtraInfo: '" + extraInfo() + "'"
                    << "Description: '" + description() + "'" << QString( "Has Pixmap? %1" ).arg( pixmap().isNull() ? "No" : "Yes" );
                QStringList children = { " - Children(" };
                for ( auto &&ii : fChildren )
                {
                    children << ii->toString( true );
                }
                tmp << children;
            }
            else
            {
                tmp << NMediaManager::NCore::toEnumString( mediaType() ) + " -"
                    << "Title: '" + title() + "'";
                switch ( mediaType() )
                {
                    case EMediaType::eMovie:
                        tmp << "Release Date: '" + fMovieReleaseDate.second + "'";
                        break;
                    case EMediaType::eTVEpisode:
                        tmp << "Episode Air Date: '" + fEpisodeAirDate.second + "'";
                    case EMediaType::eTVSeason:
                        tmp << "Season Start Date: '" + fSeasonStartDate.second + "'";
                    case EMediaType::eTVShow:
                        tmp << "Show First Air Date: '" + fShowFirstAirDate.second + "'";
                        break;
                    default:
                        break;
                }
                tmp << "TMDBID: '" + tmdbID() + "'";

                if ( mediaType() != EMediaType::eMovie )
                {
                    if ( !season().isEmpty() )
                        tmp << "Season TMBDID: '" + seasonTMDBID() + "'";
                    if ( !episode().isEmpty() )
                        tmp << "Episode TMDBID: '" + episodeTMDBID() + "'";

                    if ( !season().isEmpty() )
                        tmp << "Season: '" + season() + "'";

                    if ( !episode().isEmpty() )
                        tmp << "Episode: '" + episode() + "'";
                    if ( subTitle().isEmpty() )
                        tmp << "Sub Title: '" + subTitle() + "'";
                }

                tmp << "ExtraInfo: '" + extraInfo() + "'";
            }
            QString retVal = QString( forDebug ? "STitleInfo(%1)" : "%1" ).arg( tmp.join( " " ) );
            return retVal;
        }

        QString CTransformResult::getText( ETitleInfo which, bool forceTop /*= false */ ) const
        {
            auto parentPtr = fParent.lock();
            if ( forceTop )
            {
                if ( parentPtr )
                    return parentPtr->getText( which, forceTop );
                else
                    return QString();
            }

            auto text = getMyText( which );
            if ( text.isEmpty() && parentPtr )
            {
                text = parentPtr->getText( which, forceTop );
            }
            return text;
        }

        bool CTransformResult::isBetterTitleMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const
        {
            if ( !rhs )
                return true;

            if ( searchInfo->searchName() == title() && ( searchInfo->searchName() != rhs->title() ) )
                return true;

            return false;
        }

        bool CTransformResult::isBetterSeasonMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const
        {
            if ( searchInfo->season() != -1 )
            {
                if ( !rhs )
                    return true;

                if ( mediaType() != rhs->mediaType() )
                {
                    if ( searchInfo->mediaType() == mediaType() )
                        return true;
                    if ( searchInfo->mediaType() == rhs->mediaType() )
                        return false;
                }

                if ( season() != rhs->season() )
                {
                    if ( searchInfo->isSeasonMatch( season() ) )
                        return true;
                    if ( searchInfo->isSeasonMatch( rhs->season() ) )
                        return false;
                }
            }

            return isBetterTitleMatch( searchInfo, rhs );
        }

        bool CTransformResult::isBetterEpisodeMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const
        {
            if ( !rhs )
                return true;

            if ( isBetterSeasonMatch( searchInfo, rhs ) )
                return true;

            if ( !searchInfo->hasEpisodes() )
            {
                if ( episode() != rhs->episode() )
                {
                    if ( searchInfo->isEpisodeMatch( episode() ) )
                        return true;
                    if ( searchInfo->isEpisodeMatch( rhs->episode() ) )
                        return false;
                }
            }

            return isBetterTitleMatch( searchInfo, rhs );
        }

        bool CTransformResult::isBetterMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const
        {
            if ( mediaType() == EMediaType::eTVSeason )
                return isBetterSeasonMatch( searchInfo, rhs );
            else if ( mediaType() == EMediaType::eTVEpisode )
                return isBetterSeasonMatch( searchInfo, rhs );
            else
                return isBetterTitleMatch( searchInfo, rhs );
        }

        QString CTransformResult::getMyText( ETitleInfo which ) const
        {
            switch ( which )
            {
                case ETitleInfo::eTitle:
                    return getTitle();
                case ETitleInfo::eYear:
                    {
                        switch ( mediaType() )
                        {
                            case EMediaType::eMovie:
                                return getMovieReleaseYear();
                            case EMediaType::eTVShow:
                                return getShowFirstAirYear();
                            case EMediaType::eTVSeason:
                                return getSeasonStartYear();
                            case EMediaType::eTVEpisode:
                                return getEpisodeAirYear();
                            default:
                                break;
                        }
                    }
                    break;
                case ETitleInfo::eTMDBID:
                    return tmdbID();
                case ETitleInfo::eSeason:
                    return season();
                case ETitleInfo::eEpisode:
                    return episode();
                case ETitleInfo::eEpisodeTitle:
                    return subTitle();
                case ETitleInfo::eExtraInfo:
                    return extraInfo();
                case ETitleInfo::eDescription:
                    return description();
            }
            return {};
        }

        const QString kNoItems = "<NO ITEMS>";
        const QString kNoMatch = "<NO MATCH>";
        const QString kDeleteThis = "<DELETE THIS>";

        bool CTransformResult::isNoItems( const QString &text )
        {
            return text == kNoItems;
        }

        QString CTransformResult::getNoItems()
        {
            return kNoItems;
        }

        bool CTransformResult::isNoMatch( const QString &text )
        {
            return text == kNoMatch;
        }

        bool CTransformResult::isNoItems() const
        {
            return isNoItems( title() );
        }

        QString CTransformResult::getNoMatch()
        {
            return kNoMatch;
        }

        bool CTransformResult::isDeleteThis( const QString &text )
        {
            return text == kDeleteThis;
        }

        bool CTransformResult::isNoMatch() const
        {
            if ( mediaType() == EMediaType::eNotFoundType )
                return true;

            return isNoMatch( title() );
        }

        QString CTransformResult::getDeleteThis()
        {
            return kDeleteThis;
        }

        bool CTransformResult::isDeleteThis() const
        {
            return isDeleteThis( title() );
        }

        bool CTransformResult::isAutoSetText() const
        {
            if ( ( mediaType() == EMediaType::eDeleteFileType ) || ( mediaType() == EMediaType::eNotFoundType ) || ( mediaType() == EMediaType::eUnknownType ) )
                return true;

            return isAutoSetText( title() );
        }

        bool CTransformResult::isAutoSetText( const QString &text )
        {
            return isNoItems( text ) || isDeleteThis( text ) || isNoMatch( text );
        }

        QString toEnumString( EMediaType infoType )
        {
            switch ( infoType )
            {
                case EMediaType::eUnknownType:
                    return "Unknown";
                case EMediaType::eMovie:
                    return "Movie";
                case EMediaType::eTVShow:
                    return "TV Show";
                case EMediaType::eTVSeason:
                    return "TV Season";
                case EMediaType::eTVEpisode:
                    return "TV Episode";
                case EMediaType::eDeleteFileType:
                    return "Delete File";
                case EMediaType::eNotFoundType:
                    return "Not Found";
            }
            return {};
        }

        bool isTVType( EMediaType infoType )
        {
            switch ( infoType )
            {
                case EMediaType::eTVEpisode:
                case EMediaType::eTVSeason:
                case EMediaType::eTVShow:
                    return true;
                default:
                    return false;
            }
        }

        bool isMovieType( EMediaType infoType )
        {
            return infoType == EMediaType::eMovie;
        }

    }
}
