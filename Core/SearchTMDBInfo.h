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

#ifndef _SEARCHTMDBINFO_H
#define _SEARCHTMDBINFO_H

#include <QUrl>
#include <QString>
#include <QDate>
#include <optional>
#include <memory>
class QFileInfo;

namespace NMediaManager
{
    namespace NCore
    {
        class CTransformResult;

        enum class EMediaType;
        enum class ESearchType
        {
            eSearchTV,
            eSearchMovie,
            eGetMovie,
            eGetTVShow
        };

        struct SSearchTMDBInfo
        {
            SSearchTMDBInfo();
            SSearchTMDBInfo( const QString & searchString, std::shared_ptr< CTransformResult > titleInfo );

            bool canSearch() const;


            static bool hasDiskNumber( QString & searchString, int & diskNum, std::shared_ptr< CTransformResult > searchResult );
            static EMediaType looksLikeTVShow( const QString & searchString, QString * titleStr, QString * seasonStr = nullptr, QString * episodeStr = nullptr, QString * extraStr = nullptr );
            static bool isRippedFromMKV( const QString & name, int * titleNum=nullptr );
            static bool isRippedFromMKV( const QFileInfo & fi, int * titleNum = nullptr );

            void updateSearchCriteria( bool updateSearchBy );

            void extractTVInfo();
            void extractTMDBID();
            void extractDiskNum();
            void extractReleaseDate();

            std::optional< std::pair< QUrl, ESearchType > > getSearchURL() const;

            void setSearchName( const QString & searchName ) { fSearchName = searchName; }
            QString searchName() const { return fSearchName; }
            QString subTitle() const { return fSubTitle; }

            bool isTVMedia() const;
            void setMediaType( EMediaType searchType ) { fMediaType = std::make_pair( searchType, false ); }
            EMediaType mediaType() const { return fMediaType.first; }
            bool mediaTypeAutoDetermined() const { return fMediaType.second; }

            void setExactMatchOnly( bool exactMatchOnly ) { fExactMatchOnly = exactMatchOnly; }
            bool exactMatchOnly() const { return fExactMatchOnly; }

            void setSearchByName( bool searchByName ) { fSearchByName = searchByName; }
            bool searchByName() const { return fSearchByName; }

            void setPageNumber( int pageNumber );
            void setReleaseDate( const QString & releaseDate );
            std::pair< QDate, QString > releaseDate() const { return fReleaseDate; }
            int releaseYear( bool * aOK = nullptr ) const;
            static int releaseYear( const QString & dateStr, bool * aOK = nullptr );
            bool releaseDateSet()  const { return !fReleaseDate.second.isEmpty(); }

            void setTMDBID( const QString & tmdbID ) { fTMDBID = tmdbID; }
            QString tmdbIDString() const { return fTMDBID; }
            int tmdbID( bool * aOK = nullptr ) const;
            bool tmdbIDSet()  const { return !fTMDBID.isEmpty(); }

            int season() const { return fSeason; }
            void setSeason( int value ) { fSeason = value; }
            int episode() const { return fEpisode; }
            void setEpisode( int value ) { fEpisode = value; }

            QString getExtendedInfo() const { return fFoundExtendedInfo; }
            QString toString( bool forDebug ) const;

            bool isMatch( std::shared_ptr< CTransformResult > searchResult ) const;

            template< typename T >
            bool isMatch( const std::pair< QDate, QString > & releaseDate, const T & tmdbid, const QString & name ) const
            {
                auto retVal = ( tmdbIDSet() && isMatchingTMDBID( tmdbid ) )
                    || ( isMatchingDate( releaseDate )
                         && isMatchingTMDBID( tmdbid )
                         && isMatchingName( name ) )
                    ;
                return retVal;
            }

            template< typename T >
            bool isMatch( const QString & releaseDate, const T & tmdbid, const QString & name ) const
            {
                return isMatch( { NSABUtils::getDate( releaseDate ), releaseDate }, tmdbid, name );
            }

            template< typename T >
            bool isMatch( const std::pair< QDate, QString > & releaseDate, const T & tmdbid, const QString & name, EMediaType mediaType, const T & season, const T & episode ) const
            {
                bool retVal = isMatch( releaseDate, tmdbid, name ) && (fMediaType.first == mediaType);

                if ( isTVMedia() && retVal )
                    retVal = retVal && isSeasonMatch( season ) && isEpisodeMatch( episode );
                return retVal;
            }

            template< typename T >
            bool isMatch( const QString & releaseDate, const T & tmdbid, const QString & name, EMediaType mediaType, const T & season, const T & episode ) const
            {
                return isMatch( { NSABUtils::getDate( releaseDate ), releaseDate }, tmdbid, name, mediaType, season, episode );
            }

            bool isSeasonMatch( int seasonToMatch ) const;
            bool isSeasonMatch( const QString & seasonToMatch ) const;
            bool isEpisodeMatch( int episodeToMatch ) const;
            bool isEpisodeMatch( const QString & episodeToMatch ) const;
        private:
            QStringList getSearchStrings() const;

            bool isMatchingDate( const std::pair< QDate, QString > & releaseDate ) const;
            bool isMatchingTMDBID( int tmdbid ) const;
            bool isMatchingTMDBID( const QString & tmdbd ) const;
            bool isMatchingName( const QString & name ) const;


            static QString stripExistingExtraInfo( const QString & string, QString & extended );
            static QString stripKnownExtendedData( const QString & string, QString & extended );
            static QString stripKnownData( const QString & string );
            static QString replaceKnownAbbreviations( const QString & string );

            static QString smartTrim( const QString & string, bool stripInnerSeparators = false, bool checkForKnownHyphens = false );
            static QStringList stripOutPositions( const QString & inString, const std::list< std::pair< int, int > > & positions );

            QString fSearchName;
            std::pair< QDate, QString > fReleaseDate;
            std::optional< int > fPageNumber;
            int fSeason{ -1 };
            int fEpisode{ -1 };
            int fDiskNum{ -1 };
            QString fTMDBID;
            QString fShowTMDBID;
            std::pair< EMediaType, bool > fMediaType;
            bool fExactMatchOnly{ false };
            bool fSearchByName{ false };

            // not search criteria
            QString fSubTitle;
            QString fDescription;

            QString fFoundExtendedInfo;

            QString fInitSearchString;
            std::shared_ptr< CTransformResult > fSearchResult;
        };
    }
}
QDebug operator<<( QDebug debug, const NMediaManager::NCore::SSearchTMDBInfo & info );

#endif 
