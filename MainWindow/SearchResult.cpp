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

#include "SearchResult.h"
#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"

SSearchResult::SSearchResult( EResultInfoType type ) :
    fInfoType( type )
{

}

QString SSearchResult::getTitle() const
{
    return NStringUtils::transformTitle( fTitle );
}

QString SSearchResult::getYear() const
{
    auto dt = NQtUtils::findDate( fReleaseDate );
    if ( !dt.isValid() )
        return QString();
    return QString::number( dt.year() );
}

QString SSearchResult::getEpisodeTitle() const
{
    return NStringUtils::transformTitle( fEpisodeTitle );
}

QString SSearchResult::getTMDBID() const
{
    auto retVal = fTMDBID;
    if ( !fSeasonTMDBID.isEmpty() )
        retVal = fSeasonTMDBID;
    if ( !fEpisodeTMDBID.isEmpty() )
        retVal = fEpisodeTMDBID;
    return retVal;
}


QString SSearchResult::getSeason() const
{
    if ( !isTVShow() )
        return QString();
    if ( ( fInfoType != EResultInfoType::eTVSeason ) && ( fInfoType != EResultInfoType::eTVEpisode ) )
        return QString();
    return fSeason;
}


QString SSearchResult::getEpisode() const
{
    if ( !isTVShow() )
        return QString();
    if ( fInfoType != EResultInfoType::eTVEpisode )
        return QString();
    return fSeason;
}

void SSearchResult::removeChild( std::shared_ptr< SSearchResult > info )
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

QString SSearchResult::toString() const
{
    QString retVal = "STitleInfo( ";
    QStringList tmp;
    tmp << "InfoType: '" + ::toEnumString( fInfoType ) + "'"
        << "Title: '" + fTitle + "'"
        << "ReleaseDate: '" + fReleaseDate + "'"
        << "TMDBID: '" + fTMDBID + "'"
        << "SeasonTMBDID: '" + fSeasonTMDBID + "'"
        << "EpisodeTMDBID: '" + fEpisodeTMDBID + "'"
        << "Season: '" + fSeason + "'" 
        << QString( " Season Only? %1" ).arg( fSeasonOnly ? "Yes" : "No" )
        << "Episode: '" + fEpisode + "'"
        << "EpisodeTitle: '" + fEpisodeTitle + "'"
        << "ExtraInfo: '" + fExtraInfo + "'"
        << "Description: '" + fDescription + "'"
        << QString( "Has Pixmap? %1" ).arg( fPixmap.isNull() ? "No" : "Yes" )
        ;
    QStringList children = { " - Children(" };
    for ( auto &&ii : fChildren )
    {
        children << ii->toString();
    }
    tmp << children;
    retVal += tmp.join( " " ) +  ")";
    return retVal;
}

QString SSearchResult::getText( ETitleInfo which, bool forceTop /*= false */ ) const
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

QString SSearchResult::getMyText( ETitleInfo which ) const
{
    switch( which )
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
    }
    return QString();
}