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
namespace NMediaManager
{
    namespace NCore
    {
        struct SSearchResult;

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
            SSearchTMDBInfo( const QString &searchString, std::shared_ptr< SSearchResult > titleInfo );

            static bool looksLikeTVShow( const QString &searchString, QString *titleStr, QString *seasonStr=nullptr, QString *episodeStr = nullptr, QString * extraStr = nullptr );

            void updateSearchCriteria( bool updateSearchBy );

            std::optional< std::pair< QUrl, ESearchType > > getSearchURL() const;

            void setSearchName( const QString &searchName ) { fSearchName = searchName; }
            QString searchName() const { return fSearchName; }
            QString episodeTitle() const { return fEpisodeTitle; }

            void setIsTVShow( bool isTVShow ) { fIsTVShow = isTVShow; }
            bool isTVShow() const { return fIsTVShow; }

            void setExactMatchOnly( bool exactMatchOnly ) { fExactMatchOnly = exactMatchOnly; }
            bool exactMatchOnly() const { return fExactMatchOnly; }

            void setSearchByName( bool searchByName ) { fSearchByName = searchByName; }
            bool searchByName() const { return fSearchByName; }

            void setReleaseDate( const QString &releaseDate ) { fReleaseDate = releaseDate; }
            QString releaseDateString() const { return fReleaseDate; }
            int releaseDate( bool *aOK = nullptr ) const;
            bool releaseDateSet()  const { return !fReleaseDate.isEmpty(); }

            void setTMDBID( const QString &tmdbID ) { fTMDBID = tmdbID; }
            QString tmdbIDString() const { return fTMDBID; }
            int tmdbID( bool *aOK = nullptr ) const;
            bool tmdbIDSet()  const { return !fTMDBID.isEmpty(); }

            int season() const { return fSeason; }
            void setSeason( int value ) { fSeason = value; }
            int episode() const { return fEpisode; }
            void setEpisode( int value ) { fEpisode = value; }

            QString getExtendedInfo() const { return fFoundExtendedInfo; }
            QString toString() const;

            bool isMatch( std::shared_ptr< SSearchResult > searchResult ) const;


            template< typename T >
            bool isMatch( const QString &releaseDate, const T &tmdbid, const QString &name ) const
            {
                bool retVal = isMatchingDate( releaseDate )
                    && isMatchingTMDBID( tmdbid )
                    && isMatchingName( name )
                    ;
                return retVal;
            }

            template< typename T >
            bool isMatch( const QString &releaseDate, const T &tmdbid, const QString &name, bool isTVShow, const T &season, const T &episode ) const
            {
                bool retVal = isMatch( releaseDate, tmdbid, name ) && ( fIsTVShow && isTVShow );

                if ( fIsTVShow && retVal )
                    retVal = retVal && isSeasonMatch( season ) && isEpisodeMatch( episode );
                return retVal;
            }

            bool isSeasonMatch( int seasonToMatch ) const;
            bool isSeasonMatch( const QString &seasonToMatch ) const;
            bool isEpisodeMatch( int episodeToMatch ) const;
            bool isEpisodeMatch( const QString &episodeToMatch ) const;
        private:
            bool isMatchingDate( const QString &releaseDate ) const;
            bool isMatchingTMDBID( int tmdbid ) const;
            bool isMatchingTMDBID( const QString &tmdbd ) const;
            bool isMatchingName( const QString &name ) const;


            static QString stripKnownExtendedData(const QString & string, QString & extended );
            static QString stripKnownData( const QString &string );
            static QString smartTrim( const QString &string, bool stripInnerPeriods = false );
            static QStringList stripOutPositions( const QString &inString, const std::list< std::pair< int, int > > &positions );

            QString fSearchName;
            QString fReleaseDate;
            int fSeason{ -1 };
            int fEpisode{ -1 };
            QString fTMDBID;
            bool fIsTVShow{ false };
            bool fExactMatchOnly{ false };
            bool fSearchByName{ false };

            // not search criteria
            QString fEpisodeTitle;
            QString fDescription;

            QString fFoundExtendedInfo;

            QString fInitSearchString;
            std::shared_ptr< SSearchResult > fSearchResultInfo;
        };
    }
}
QDebug operator<<( QDebug debug, const NMediaManager::NCore::SSearchTMDBInfo &info );

#endif 
