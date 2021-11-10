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

#ifndef _SEARCHTMDBINFO_H
#define _SEARCHTMDBINFO_H

#include <QUrl>
#include <QString>
#include <optional>
#include <memory>
struct STitleInfo;

enum class ESearchType
{
    eSearchTV,
    eSearchMovie,
    eGetMovie,
    eGetTVShow
};

struct SSearchTMDBInfo
{
    SSearchTMDBInfo() {};
    SSearchTMDBInfo( const QString &searchString, std::shared_ptr< STitleInfo > titleInfo );

    void updateSearchCriteria( bool updateSearchBy );

    std::optional< std::pair< QUrl, ESearchType > > getSearchURL() const;

    void setSearchName( const QString &searchName ) { fSearchName = searchName; }
    QString searchName() const { return fSearchName; }
    QString episodeTitle() const { return fEpisodeTitle; }

    void setIsMovie( bool isMovie ) { fIsMovie = isMovie;  }
    bool isMovie() const { return fIsMovie; }

    void setExactMatchOnly( bool exactMatchOnly ) { fExactMatchOnly = exactMatchOnly; }
    bool exactMatchOnly() const { return fExactMatchOnly; }

    void setSearchByName( bool searchByName ) { fSearchByName = searchByName; }
    bool searchByName() const { return fSearchByName; }

    void setReleaseDate( const QString &releaseDate ) { fReleaseDate = releaseDate; }
    QString releaseDateString() const { return fReleaseDate;  }
    int releaseDate( bool *aOK = nullptr ) const;
    bool releaseDateSet()  const { return !fReleaseDate.isEmpty(); }

    void setTMDBID( const QString &tmdbID ) { fTMDBID = tmdbID; }
    QString tmdbIDString() const { return fTMDBID; }
    int tmdbID( bool *aOK = nullptr ) const;
    bool tmdbIDSet()  const { return !fTMDBID.isEmpty(); }

    int season() const { return fSeason; }
    int episode() const { return fEpisode; }
private:
    QString stripKnownData( const QString &string ) const;
    QString smartTrim( const QString &string, bool stripInnerPeriods = false ) const;

    QString fSearchName;
    QString fReleaseDate;
    QString fEpisodeTitle;
    QString fDescription;
    int fSeason{ -1 };
    int fEpisode{ -1 };
    QString fTMDBID;
    bool fIsMovie{ false };
    bool fExactMatchOnly{ false };
    bool fSearchByName{ false };

    QString fInitSearchString;
    std::shared_ptr< STitleInfo > fTitleInfo;
};

#endif 
