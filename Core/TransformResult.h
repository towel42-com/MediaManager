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

#ifndef __TRANSFORMRESULT_H
#define __TRANSFORMRESULT_H

#include <QString>
#include <QPixmap>
#include <memory>
#include <list>
#include <QDate>
#include <functional>
class QFileInfo;

namespace NMediaManager
{
    namespace NCore
    {
        struct SPatternInfo;
        struct SSearchTMDBInfo;
        enum class ETitleInfo
        {
            eTitle,
            eYear,
            eTMDBID,
            eSeason,
            eEpisode,
            eEpisodeTitle,
            eExtraInfo,
            eDescription
        };

        enum class EMediaType
        {
            eUnknownType,
            eDeleteFileType,
            eNotFoundType,
            eMovie,
            eTVShow,
            eTVSeason,
            eTVEpisode
        };
        QString toEnumString( EMediaType infoType );
        bool isTVType( EMediaType infoType );
        bool isMovieType( EMediaType infoType );

        class CTransformResult
        {
        public:
            CTransformResult( EMediaType type );

            void mergeEpisodeResults( const std::shared_ptr< CTransformResult > &rhs );

            bool isTVShow() const { return mediaType() != EMediaType::eMovie; }   // tvshow, season or episode are all not movie
            bool isDeleteResult() const { return mediaType() == EMediaType::eDeleteFileType; }   // tvshow, season or episode are all not movie
            bool isNotFoundResult() const { return mediaType() == EMediaType::eNotFoundType; }   // search not found
            QString getTitle() const;
            QString getMovieReleaseYear() const;
            QString getShowFirstAirYear() const;
            QString getSeasonStartYear() const;
            QString getEpisodeAirYear() const;
            QString getYear() const;

            std::pair< QDate, QString > getMovieReleaseDate() const;
            std::pair< QDate, QString > getShowFirstAirDate() const;
            std::pair< QDate, QString > getSeasonStartDate() const;
            std::pair< QDate, QString > getEpisodeAirDate() const;
            std::pair< QDate, QString > getDate() const;

            QString getSubTitle() const;
            QString getTMDBID() const;
            bool isSeasonOnly() const { return fSeasonOnly; }
            void setSeasonOnly( bool value ) { fSeasonOnly = value; }
            QString getSeason() const;
            QString getEpisode() const;

            bool isNoItems() const;
            bool isNoMatch() const;
            bool isDeleteThis() const;

            static bool isNoItems( const QString &text );
            static QString getNoItems();

            static bool isNoMatch( const QString &text );
            static QString getNoMatch();

            static bool isDeleteThis( const QString &text );
            static QString getDeleteThis();

            static bool isAutoSetText( const QString &text );
            bool isAutoSetText() const;

            QString transformedName( const QFileInfo &fileInfo, const SPatternInfo &info, bool titleOnly ) const;
            void removeChild( std::shared_ptr< CTransformResult > info );

            QString toString( bool forDebug ) const;
            [[nodiscard]] QString getMyText( ETitleInfo which ) const;
            [[nodiscard]] QString getText( ETitleInfo which, bool forceTop = false ) const;

            bool isBetterMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const;

            const CTransformResult *getTVShowInfo() const;   // not to be saved, only used and ignored

            void setMovieReleaseDate( const QString &date );
            void setShowFirstAirDate( const QString &date );
            void setSeasonStartDate( const QString &date );
            void setEpisodeAirDate( const QString &date );

            void setMovieReleaseDate( const std::pair< QDate, QString > &date ) { fMovieReleaseDate = date; }
            void setShowFirstAirDate( const std::pair< QDate, QString > &date ) { fShowFirstAirDate = date; }
            void setSeasonStartDate( const std::pair< QDate, QString > &date ) { fSeasonStartDate = date; }
            void setEpisodeAirDate( const std::pair< QDate, QString > &date ) { fEpisodeAirDate = date; }

            [[nodiscard]] static QString cleanFileName( const QString &inFile, bool isDir );
            [[nodiscard]] static QString cleanFileName( const QFileInfo &fi );

            bool operator==( const CTransformResult &rhs ) const;

            void setParent( std::shared_ptr< CTransformResult > parent ) { fParent = parent; }

            bool hasChildren() const { return !fChildren.empty(); }
            void addChild( std::shared_ptr< CTransformResult > child ) { fChildren.push_back( child ); }

            void onAllChildren( std::function< void( std::shared_ptr< CTransformResult > child ) > func, std::function< bool() > stopFunc );

            QString diskNum() const { return fDiskNum; }
            //void setDiskNum( const QString & val ) { fDiskNum = val; }

            std::weak_ptr< NMediaManager::NCore::CTransformResult > parent() const { return fParent; }
            //void setParent( std::weak_ptr<NMediaManager::NCore::CTransformResult> val ) { fParent = val; }

            QString description() const { return fDescription; }
            void setDescription( const QString &val ) { fDescription = val; }

            QString title() const { return fTitle; }
            void setTitle( const QString &val );

            QString tmdbID() const { return fTMDBID; }
            void setTMDBID( const QString &val ) { fTMDBID = val; }

            QString showTMDBID() const { return fShowTMDBID; }
            void setShowTMDBID( const QString &val ) { fShowTMDBID = val; }

            QString seasonTMDBID() const { return fSeasonTMDBID; }
            void setSeasonTMDBID( const QString &val ) { fSeasonTMDBID = val; }

            QString episodeTMDBID() const { return fEpisodeTMDBID; }
            void setEpisodeTMDBID( const QString &val ) { fEpisodeTMDBID = val; }

            QString extraInfo() const { return fExtraInfo; }
            void setExtraInfo( const QString &val ) { fExtraInfo = val; }

            QString season() const { return fSeason; }
            void setSeason( const QString &val ) { fSeason = val; }

            QString episode() const { return fEpisode; }
            void setEpisode( const QString &val ) { fEpisode = val; }

            QString subTitle() const { return fSubTitle; }
            void setSubTitle( const QString &val ) { fSubTitle = val; }

            QPixmap pixmap() const { return fPixmap; }
            void setPixmap( const QPixmap &val ) { fPixmap = val; }
            void setPixmapPath( const QString &path ) { fPixmapPath = path; }
            bool pixmapFinished() const { return !fPixmap.isNull() || fPixmapPath.isEmpty(); }

            NMediaManager::NCore::EMediaType mediaType() const { return fMediaType; }
            void setMediaType( NMediaManager::NCore::EMediaType val ) { fMediaType = val; }

        private:
            QString fTitle;
            QString fTMDBID;
            QString fShowTMDBID;
            QString fSeasonTMDBID;
            QString fEpisodeTMDBID;

            std::pair< QDate, QString > fMovieReleaseDate;
            std::pair< QDate, QString > fShowFirstAirDate;
            std::pair< QDate, QString > fSeasonStartDate;
            std::pair< QDate, QString > fEpisodeAirDate;
            QString fSeason;
            bool fSeasonOnly{ false };
            QString fEpisode;
            QString fSubTitle;
            QString fExtraInfo;
            QString fDiskNum;

            QString fDescription;
            QPixmap fPixmap;
            QString fPixmapPath;

            EMediaType fMediaType;

            std::weak_ptr< CTransformResult > fParent;
            std::list< std::shared_ptr< CTransformResult > > fChildren;
            std::list< std::shared_ptr< CTransformResult > > fExtraEpisodes;

        private:
            bool isBetterTitleMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const;
            bool isBetterSeasonMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const;
            bool isBetterEpisodeMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< CTransformResult > rhs ) const;
        };
    }
}
#endif   //
