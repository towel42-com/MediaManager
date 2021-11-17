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

#include "SearchTMDBInfo.h"
#include "TitleInfo.h"
#include <QRegularExpression>
#include <QDebug>

SSearchTMDBInfo::SSearchTMDBInfo( const QString &text, std::shared_ptr< STitleInfo > titleInfo )
{
    fTitleInfo = titleInfo;
    fInitSearchString = text;
    fIsTVShow = looksLikeTVShow( text ).first;
    updateSearchCriteria( true );
}

QString SSearchTMDBInfo::stripKnownData( const QString & string )
{
    QString retVal = string;
    auto separators = QStringList() << "1080p" << "720p" << "AMZN" << "WebRip" << "WEB" << "-RUMOUR" << "-PECULATE" << "x264" << "x265" << "h264" << "h265" << "rarbg" << "BluRay" << "ion10" << "LiMiTED" << "DVDRip" <<  "XviDTLF";
    for ( auto &&separator : separators )
    {
        retVal.replace( "[" + separator + "]", "", Qt::CaseSensitivity::CaseInsensitive );
        retVal.replace( separator, "", Qt::CaseSensitivity::CaseInsensitive );
    }
    return retVal;
}

QString SSearchTMDBInfo::smartTrim( const QString & string, bool stripInnerPeriods )
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
        retVal.replace( QRegularExpression( "\\.|(\\s{2,})|-|\\:"), " " );
        retVal = retVal.trimmed();
    }
    return retVal;
}

std::pair< bool, QString > SSearchTMDBInfo::looksLikeTVShow( const QString & searchString, QString * seasonStr, QString * episodeStr )
{
    QString retVal = searchString;

    bool isTV = false;
    QString dataBeforeSeason;
    auto regExpStr = QString( "S(?<season>\\d+)" );
    auto regExp = QRegularExpression( regExpStr );
    auto match = regExp.match( retVal );
    if ( match.hasMatch() )
    {
        if ( seasonStr )
            *seasonStr = smartTrim( match.captured( "season" ) );
        dataBeforeSeason = smartTrim( retVal.left( match.capturedStart( "season" ) - 1 ) );
        retVal = retVal.mid( match.capturedStart( "season" ) - 1 );
        retVal.replace( match.capturedStart( "season" ), match.capturedLength( "season" ), "" );
        isTV = true;
    }

    QString dataAfterEpisode;
    regExpStr = "E(?<episode>\\d+)";
    regExp = QRegularExpression( regExpStr );
    match = regExp.match( retVal );
    if ( match.hasMatch() )
    {
        if ( episodeStr )
            *episodeStr = smartTrim( match.captured( "episode" ) );
        dataAfterEpisode = smartTrim( retVal.mid( match.capturedEnd( "episode" ) ) );
        retVal.replace( match.capturedStart( "episode" ), match.capturedLength( "episode" ), "" );
        isTV = true;
    }

    if ( !dataBeforeSeason.isEmpty() )
        retVal = dataBeforeSeason;
    if ( !dataAfterEpisode.isEmpty() )
        retVal += " " + dataAfterEpisode;

    regExpStr = ".*\\s??(?<seasonsuffix>-\\s??Season\\s?(?<season>\\d+))";
    regExp = QRegularExpression( regExpStr );
    match = regExp.match( retVal );
    if ( match.hasMatch() )
    {
        if ( seasonStr )
            *seasonStr = smartTrim( match.captured( ( "season" ) ) );
        retVal.replace( match.capturedStart( "seasonsuffix" ), match.capturedEnd( "seasonsuffix" ), "" );
        isTV = true;
    }

    return std::make_pair( isTV, retVal );
}

void SSearchTMDBInfo::updateSearchCriteria( bool updateSearchBy )
{
    fSearchName = smartTrim( stripKnownData( fInitSearchString ) );
    QString seasonStr;
    QString episodeStr;

    auto regExp = QRegularExpression( "(?<fulltext>[\\.\\(](?<releaseDate>\\d{2,4})(?:[\\.\\)]?|$))" );
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

    if ( fIsTVShow )
        fSearchName = looksLikeTVShow( fSearchName, &seasonStr, &episodeStr ).second;

    fSearchName = smartTrim( fSearchName, true );

    if ( fTitleInfo )
    {
        fSearchName = fTitleInfo->fTitle;
        episodeStr = fTitleInfo->fEpisode;
        seasonStr = fTitleInfo->fSeason;
        fReleaseDate = fTitleInfo->fReleaseDate;
        fTMDBID = fTitleInfo->fTMDBID;// always get the main one
        fEpisodeTitle = fTitleInfo->fEpisodeTitle;
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

int SSearchTMDBInfo::releaseDate( bool * aOK ) const
{
    return fReleaseDate.toInt( aOK );
}

int SSearchTMDBInfo::tmdbID( bool *aOK ) const
{
    return fTMDBID.toInt( aOK );
}

QDebug operator<<( QDebug debug, const SSearchTMDBInfo &info )
{
    debug << info.toString();
    return debug;
}

QString SSearchTMDBInfo::toString() const
{
    QString retVal = QString( "SSearchTMDBInfo(%1 (%2)-S%3E%4-%5-%6-%7-%8)" ).arg( searchName() ).arg( releaseDateString() ).arg( season() ).arg( episode() ).arg( tmdbIDString() ).arg( isTVShow() ).arg( exactMatchOnly() ).arg( searchName() );
    return retVal;
}
