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

#ifndef __TRANSFORMRESULT_H
#define __TRANSFORMRESULT_H

#include <QString>
#include <QPixmap>
#include <memory>
#include <list>

namespace NMediaManager
{
    namespace NCore
    {
        struct SSearchTMDBInfo;
        enum class ETitleInfo
        {
            eTitle,
            eReleaseDate,
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
            eMovie,
            eTVShow,
            eTVSeason,
            eTVEpisode
        };
        QString toEnumString( EMediaType infoType );
        bool isTVType( EMediaType infoType );


        struct STransformResult
        {
            STransformResult( EMediaType type );

            EMediaType mediaType() const { return fMediaType; }
            bool isTVShow() const { return fMediaType != EMediaType::eMovie; } // tvshow, season or episode are all not movie
            bool isDeleteResult() const { return fMediaType == EMediaType::eDeleteFileType; } // tvshow, season or episode are all not movie
            QString getTitle() const;
            QString getYear() const;
            QString getInitialYear() const; // if its a TV episode, get the show description year
            QString getReleaseDate() const;
            QString getSubTitle() const;
            QString getTMDBID() const;
            bool isSeasonOnly() const { return fSeasonOnly; }
            void setSeasonOnly( bool value ) { fSeasonOnly = value; }
            QString getSeason() const;
            QString getEpisode() const;

            void removeChild( std::shared_ptr< STransformResult > info );

            QString toString( bool forDebug ) const;
            [[nodiscard]] QString getMyText( ETitleInfo which ) const;
            [[nodiscard]] QString getText( ETitleInfo which, bool forceTop = false ) const;

            bool isBetterMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr<STransformResult> rhs ) const;

            const STransformResult * getTVShowInfo() const; // not to be saved, only used and ignored

            QString fTitle;
            QString fReleaseDate;
            QString fTMDBID;
            QString fSeasonTMDBID;
            QString fEpisodeTMDBID;
            QString fSeason;
            bool fSeasonOnly{ false };
            QString fEpisode;
            QString fSubTitle;
            QString fExtraInfo;
            QString fDiskNum;

            QString fDescription;
            QPixmap fPixmap;

            std::weak_ptr < STransformResult > fParent;
            std::list< std::shared_ptr< STransformResult > > fChildren;
            EMediaType fMediaType;

        private:
            bool isBetterTitleMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr<STransformResult> rhs ) const;
            bool isBetterSeasonMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< STransformResult > rhs ) const;
            bool isBetterEpisodeMatch( std::shared_ptr< SSearchTMDBInfo > searchInfo, std::shared_ptr< STransformResult > rhs ) const;
        };
    }
}
#endif // 
