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

#include "TransformResult.h"
#include "SearchTMDBInfo.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"

namespace NMediaManager
{
    namespace NCore
    {
        STransformResult::STransformResult( EResultInfoType type ) :
            fInfoType( type )
        {

        }

        QString STransformResult::getTitle() const
        {
            if ( isDeleteResult() )
                return "<DELETE THIS>";
            return NSABUtils::NStringUtils::transformTitle( fTitle );
        }

        QString STransformResult::getReleaseDate() const
        {
            if ( fReleaseDate.isEmpty() )
            {
                auto parent = fParent.lock();
                if ( parent )
                    return parent->getReleaseDate();
            }
            return fReleaseDate;
        }

        QString STransformResult::getInitialYear() const
        {
            if ( (fInfoType != EResultInfoType::eTVEpisode)
              && (fInfoType != EResultInfoType::eTVSeason) )
            {
                return getYear();
            }
            
            auto tvShowInfo = getTVShowInfo();
            if ( !tvShowInfo )
                return getYear();
            return tvShowInfo->getYear();
        }

        const STransformResult * STransformResult::getTVShowInfo() const
        {
            if (  ( fInfoType == EResultInfoType::eTVEpisode )
               || (fInfoType == EResultInfoType::eTVSeason) )
            {
                auto parent = fParent.lock();
                if ( parent )
                    return parent->getTVShowInfo();
                else
                    return nullptr;
            }
            return this;
        }


        QString STransformResult::getYear() const
        {
            auto date = getReleaseDate();
            auto dt = NSABUtils::findDate( date );
            if ( !dt.isValid() )
                return QString();
            return QString::number( dt.year() );
        }

        QString STransformResult::getEpisodeTitle() const
        {
            return NSABUtils::NStringUtils::transformTitle( fEpisodeTitle );
        }

        QString STransformResult::getTMDBID() const
        {
            auto retVal = fTMDBID;
            if ( !fSeasonTMDBID.isEmpty() )
                retVal = fSeasonTMDBID;
            if ( !fEpisodeTMDBID.isEmpty() )
                retVal = fEpisodeTMDBID;
            return retVal;
        }


        QString STransformResult::getSeason() const
        {
            if ( !isTVShow() )
                return QString();
            if ( ( fInfoType != EResultInfoType::eTVSeason ) && ( fInfoType != EResultInfoType::eTVEpisode ) )
                return QString();
            return fSeason;
        }


        QString STransformResult::getEpisode() const
        {
            if ( !isTVShow() )
                return QString();
            if ( fInfoType != EResultInfoType::eTVEpisode )
                return QString();
            return fSeason;
        }

        void STransformResult::removeChild( std::shared_ptr< STransformResult > info )
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

        QString STransformResult::toString( bool forDebug ) const
        {
            QStringList tmp;
            if ( forDebug )
            {
                QStringList tmp;
                tmp << "InfoType: '" + NMediaManager::NCore::toEnumString( fInfoType ) + "'"
                    << "Title: '" + fTitle + "'"
                    << "ReleaseDate: '" + fReleaseDate + "'"
                    << "TMDBID: '" + fTMDBID + "'"
                    << "Season TMBDID: '" + fSeasonTMDBID + "'"
                    << "Episode TMDBID: '" + fEpisodeTMDBID + "'"
                    << "Season: '" + fSeason + "'"
                    << QString( " Season Only? %1" ).arg( fSeasonOnly ? "Yes" : "No" )
                    << "Episode: '" + fEpisode + "'"
                    << "EpisodeTitle: '" + fEpisodeTitle + "'"
                    << "ExtraInfo: '" + fExtraInfo + "'"
                    << "Description: '" + fDescription + "'"
                    << QString( "Has Pixmap? %1" ).arg( fPixmap.isNull() ? "No" : "Yes" )
                    ;
                QStringList children = { " - Children(" };
                for ( auto && ii : fChildren )
                {
                    children << ii->toString( true );
                }
                tmp << children;
            }
            else
            {
                tmp << NMediaManager::NCore::toEnumString( fInfoType ) + " -"
                    << "Title: '" + fTitle + "'"
                    << "ReleaseDate: '" + fReleaseDate + "'"
                    << "TMDBID: '" + fTMDBID + "'"
                    ;

                if ( fInfoType != EResultInfoType::eMovie )
                {
                    if ( !fSeason.isEmpty() )
                        tmp << "Season TMBDID: '" + fSeasonTMDBID + "'";
                    if ( !fEpisode.isEmpty() )
                        tmp << "Episode TMDBID: '" + fEpisodeTMDBID + "'";


                    if ( !fSeason.isEmpty() )
                        tmp << "Season: '" + fSeason + "'";

                    if ( !fEpisode.isEmpty() )
                        tmp << "Episode: '" + fEpisode + "'";
                    if ( fEpisodeTitle.isEmpty() )
                        tmp << "EpisodeTitle: '" + fEpisodeTitle + "'";
                }
                
                tmp << "ExtraInfo: '" + fExtraInfo + "'"
                    ;
            }
            QString retVal = QString( forDebug ? "STitleInfo(%1)" : "%1" ).arg( tmp.join( " " ) );
            return retVal;
        }

        QString STransformResult::getText( ETitleInfo which, bool forceTop /*= false */ ) const
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


        bool STransformResult::isBetterTitleMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr<STransformResult> rhs ) const
        {
            if ( !rhs )
                return true;

            if ( searchInfo->searchName() == fTitle && ( searchInfo->searchName() != rhs->fTitle ) )
                return true;

            return false;
        }

        bool STransformResult::isBetterSeasonMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< STransformResult > rhs ) const
        {
            if ( searchInfo->season() != -1 )
            {
                if ( !rhs )
                    return true;

                if ( fSeason != rhs->fSeason )
                {
                    if ( searchInfo->isSeasonMatch( fSeason ) )
                        return true;
                    if ( searchInfo->isSeasonMatch( rhs->fSeason ) )
                        return false;
                }
            }

            return isBetterTitleMatch( searchInfo, rhs );
        }

        bool STransformResult::isBetterEpisodeMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< STransformResult > rhs ) const
        {
            if ( !rhs )
                return true;

            if ( isBetterSeasonMatch( searchInfo, rhs ) )
                return true;

            if ( searchInfo->episode() != -1 )
            {
                if ( fEpisode != rhs->fEpisode )
                {
                    if ( searchInfo->isEpisodeMatch( fEpisode ) )
                        return true;
                    if ( searchInfo->isEpisodeMatch( rhs->fEpisode ) )
                        return false;
                }
            }

            return isBetterTitleMatch( searchInfo, rhs );
        }

        bool STransformResult::isBetterMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr<STransformResult> rhs ) const
        {
            if ( fInfoType == EResultInfoType::eTVSeason )
                return isBetterSeasonMatch( searchInfo, rhs );
            else if ( fInfoType == EResultInfoType::eTVEpisode )
                return isBetterSeasonMatch( searchInfo, rhs );
            else
                return isBetterTitleMatch( searchInfo, rhs );
        }

        QString STransformResult::getMyText( ETitleInfo which ) const
        {
            switch ( which )
            {
                case ETitleInfo::eTitle: return getTitle();
                case ETitleInfo::eReleaseDate: return getYear();
                case ETitleInfo::eTMDBID: return fTMDBID;
                case ETitleInfo::eSeason: return fSeason;
                case ETitleInfo::eEpisode: return fEpisode;
                case ETitleInfo::eEpisodeTitle: return fEpisodeTitle;
                case ETitleInfo::eExtraInfo: return fExtraInfo;
                case ETitleInfo::eDescription: return fDescription;
            }
            return QString();
        }

        QString toEnumString( EResultInfoType infoType )
        {
            switch ( infoType )
            {
                case EResultInfoType::eMovie: return "Movie";
                case EResultInfoType::eTVShow: return "TV Show";
                case EResultInfoType::eTVSeason: return "TV Season";
                case EResultInfoType::eTVEpisode: return "TV Episode";
                case EResultInfoType::eDeleteFileType: return "Delete File";
            }
            return QString();
        }
    }
}
