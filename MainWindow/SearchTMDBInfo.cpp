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

SSearchTMDBInfo::SSearchTMDBInfo( const QString & text, std::shared_ptr< STitleInfo > titleInfo )
{
    fTitleInfo = titleInfo;
    fInitSearchString = text;
    updateSearchCriteria( true );
}

QString SSearchTMDBInfo::stripKnownData( const QString & string ) const
{
    QString retVal = string;
    auto separators = QStringList() << "1080p" << "720p" << "AMZN" << "WebRip" << "WEB" << "-RUMOUR" << "-PECULATE" << "x264" << "x265" << "h264" << "h265" << "rarbg" << "BluRay" << "ion10" << "-";
    for ( auto &&separator : separators )
    {
        retVal.replace( "[" + separator + "]", "", Qt::CaseSensitivity::CaseInsensitive );
        retVal.replace( separator, "", Qt::CaseSensitivity::CaseInsensitive );
    }
    return retVal;
}

QString SSearchTMDBInfo::smartTrim( const QString & string, bool stripInnerPeriods ) const
{
    auto retVal = string;
    auto pos = retVal.indexOf( QRegularExpression( "[^\\.\\s\\-]" ) );
    if ( pos != -1 )
        retVal = retVal.mid( pos );

    pos = retVal.lastIndexOf( QRegularExpression( "[^\\.\\s\\-]" ) );
    if ( pos != -1 )
        retVal = retVal.left( pos + 1 );
    if ( stripInnerPeriods )
        retVal.replace( ".", " " );
    return retVal;
}

void SSearchTMDBInfo::updateSearchCriteria( bool updateSearchBy )
{
    fSearchName = smartTrim( stripKnownData( fInitSearchString ) );
    QString seasonStr;
    QString episodeStr;

    auto regExp = QRegularExpression( "[\\.\\(](?<releaseDate>\\d{2,4})(?:[\\.\\)]?|$)" );
    auto match = regExp.match( fSearchName );
    if ( match.hasMatch() )
    {
        fReleaseDate = smartTrim( match.captured( "releaseDate" ) );
        fSearchName.replace( match.capturedStart( "releaseDate" ), match.capturedLength( "releaseDate" ), "" );
    }

    regExp = QRegularExpression( "\\[tmdbid=(?<tmdbid>\\d+)\\]" );
    match = regExp.match( fSearchName );
    if ( match.hasMatch() )
    {
        fTMDBID = smartTrim( match.captured( "tmdbid" ) );
        fSearchName.replace( match.capturedStart( "tmdbid" ), match.capturedLength( "tmdbid" ), "" );
    }

    if ( !fIsMovie )
    {
        QString dataBeforeSeason;
        auto regExp = QRegularExpression( "S(?<season>\\d+)" );
        auto match = regExp.match( fSearchName );
        if ( match.hasMatch() )
        {
            seasonStr = smartTrim( match.captured( "season" ) );
            dataBeforeSeason = smartTrim( fSearchName.left( match.capturedStart( "season" ) - 1 ) );
            fSearchName = fSearchName.mid( match.capturedStart( "season" ) - 1 );
            fSearchName.replace( match.capturedStart( "season" ), match.capturedLength( "season" ), "" );
        }

        QString dataAfterEpisode;
        regExp = QRegularExpression( "E(?<episode>\\d+)" );
        match = regExp.match( fSearchName );
        if ( match.hasMatch() )
        {
            episodeStr = smartTrim( match.captured( "episode" ) );
            dataAfterEpisode = smartTrim( fSearchName.mid( match.capturedEnd( "episode" ) ) );
            fSearchName.replace( match.capturedStart( "episode" ), match.capturedLength( "episode" ), "" );
        }

        if ( !dataBeforeSeason.isEmpty() )
            fSearchName = dataBeforeSeason;
        if ( !dataAfterEpisode.isEmpty() )
            fSearchName += " " + dataAfterEpisode;

    }

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
    return fReleaseDate.toInt( aOK );
}

