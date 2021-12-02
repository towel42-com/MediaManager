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

#ifndef _NETWORKREPLY_H
#define _NETWORKREPLY_H

#include <functional>
#include <memory>
#include <QString>
class QNetworkReply;
class QUrl;

namespace NMediaManager
{
    namespace NCore
    {
        enum class ERequestType
        {
            eUnknownRequest,
            eConfig,
            eTVSearch,
            eMovieSearch,
            eGetImage,
            eGetMovie,
            eGetTVShow,
            eSeasonInfo,
            eTVInfo
        };
        QString toString( ERequestType requestType );

        class CNetworkReply
        {
        public:
            CNetworkReply();
            CNetworkReply( ERequestType requestType, QNetworkReply *reply );
            CNetworkReply( ERequestType requestType, const QString &urlPath, const QByteArray &data );

            bool isValid() const;
            bool isCached() const { return !fReply; }

            static QString key( const QUrl &url );
            // errrors only available on real requests, not cached requests
            bool hasError() const;
            QString errorString() const;

            bool isReply( std::shared_ptr< CNetworkReply > otherReply ) const;
            bool isReply( const CNetworkReply *otherReply ) const;
            bool operator==( const CNetworkReply &otherReply ) const;

            bool isType( ERequestType type )const { return fRequestType == type; }

            QString urlPathKey() const { return fURLPathKey; }
            QString getTMDBIDFromURL() const;

            std::size_t hash() const;
            void reportUnhandled() const;

            QByteArray getData() const;

            QString toString() const;
            QNetworkReply *getNetworkReply() const { return fReply; }
        private:
            ERequestType fRequestType{ ERequestType::eUnknownRequest };
            QNetworkReply *fReply{ nullptr };
            QString fURLPathKey;
            mutable QByteArray fCachedData;
        };
        QDebug operator<<( QDebug debug, const CNetworkReply &reply );

        inline bool operator==( const std::shared_ptr< CNetworkReply > &lhs, const std::shared_ptr< CNetworkReply > &rhs )
        {
            if ( !lhs || !rhs )
                return lhs.get() == rhs.get();
            return *lhs.get() == *rhs.get();
        }
    }
}

namespace std
{
    template <>
    struct hash<NMediaManager::NCore::CNetworkReply>
    {
        std::size_t operator()( const NMediaManager::NCore::CNetworkReply &ii ) const
        {
            return ii.hash();
        }
    };

    template <>
    struct hash< std::shared_ptr< NMediaManager::NCore::CNetworkReply > >
    {
        std::size_t operator()( std::shared_ptr< NMediaManager::NCore::CNetworkReply > ii ) const
        {
            if ( ii )
                return ii->hash();
            else
                return 0;
        }
    };
}

#endif 
