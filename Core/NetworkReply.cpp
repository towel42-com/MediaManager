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

#include "NetworkReply.h"
#include "SearchTMDBInfo.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"

#include <QUrl>
#include <QUrlQuery>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QImage>
#include <QPixmap>
#include "SABUtils/HashUtils.h"

namespace NMediaManager
{
    namespace NCore
    {
        QString toString( ERequestType requestType )
        {
            switch ( requestType )
            {
                case ERequestType::eUnknownRequest:
                    return "Unknown";
                case ERequestType::eConfig:
                    return "Config";
                case ERequestType::eTVSearch:
                    return "TV Search";
                case ERequestType::eMovieSearch:
                    return "Movie Search";
                case ERequestType::eGetImage:
                    return "Get Image";
                case ERequestType::eGetMovie:
                    return "Get Movie";
                case ERequestType::eGetTVShow:
                    return "Get TV Show";
                case ERequestType::eSeasonInfo:
                    return "Get Season Info";
                case ERequestType::eTVInfo:
                    return "Get TV Info";
            }
            return QString();
        };

        CNetworkReply::CNetworkReply()
        {
        }

        CNetworkReply::CNetworkReply( ERequestType requestType, QNetworkReply *reply ) :
            fRequestType( requestType ),
            fReply( reply )
        {
            fTMDBID = tmdbFromUrl( reply->url() );
            fURLPathKey = key( reply->url(), requestType, fTMDBID );
        }

        CNetworkReply::CNetworkReply( ERequestType requestType, const QString &urlPathKey, const QByteArray &data ) :
            fRequestType( requestType ),
            fURLPathKey( urlPathKey ),
            fCachedData( data )
        {
            fTMDBID = tmdbFromKey( fURLPathKey );
        }

        QString CNetworkReply::tmdbID() const
        {
            return fTMDBID;
        }

        QString CNetworkReply::tmdbFromKey( const QString &key )
        {
            auto tmp = key.split( "__" );
            if ( tmp.length() != 3 )
                return QString();
            return tmp[ 2 ];
        }

        QString CNetworkReply::tmdbFromUrl( const QUrl &url )
        {
            auto path = url.path();
            auto tmdbid = path.mid( path.lastIndexOf( '/' ) + 1 );
            return tmdbid;
        }

        bool CNetworkReply::isValid() const
        {
            return fReply || ( fRequestType != ERequestType::eUnknownRequest );
        }

        QString CNetworkReply::key( const QUrl &url, ERequestType type, const QString &tmdb )
        {
            return NCore::toString( type ) + "__" + url.toString( QUrl::PrettyDecoded | QUrl::RemoveScheme | QUrl::RemoveAuthority ) + "__" + tmdb;
        }

        QString CNetworkReply::key( const QNetworkRequest &request, ERequestType type, const QString &tmdb )
        {
            return key( request.url(), type, tmdb );
        }

        QString CNetworkReply::key( const QNetworkRequest &request, ERequestType type )
        {
            return key( request.url(), type, tmdbFromUrl( request.url() ) );
        }

        QString CNetworkReply::key( const QUrl &url, ERequestType type )
        {
            return key( url, type, tmdbFromUrl( url ) );
        }

        QString CNetworkReply::key() const
        {
            return fURLPathKey;
        }

        bool CNetworkReply::hasError() const
        {
            return fReply && ( fReply->error() != QNetworkReply::NoError );
        }

        QString CNetworkReply::errorString() const
        {
            return fReply ? fReply->errorString() : QString();
        }

        bool CNetworkReply::isReply( std::shared_ptr< CNetworkReply > otherReply ) const
        {
            return isReply( otherReply.get() );
        }

        bool CNetworkReply::isReply( const CNetworkReply *otherReply ) const
        {
            if ( !otherReply )
                return false;
            return *this == *otherReply;
        }

        bool CNetworkReply::operator==( const CNetworkReply &otherReply ) const
        {
            if ( fReply == otherReply.fReply )
                return true;
            if ( fRequestType != otherReply.fRequestType )
                return false;
            if ( fURLPathKey != otherReply.fURLPathKey )
                return false;
            if ( fTMDBID != otherReply.fTMDBID )
                return false;

            return true;
        }

        std::size_t CNetworkReply::hash() const
        {
            auto h1 = std::hash< ERequestType >()( fRequestType );
            auto h2 = std::hash< QString >()( fURLPathKey );
            auto h3 = std::hash< QString >()( fTMDBID );
            return NSABUtils::HashCombine( { h1, h2, h3 } );   // as a key, it only depends on the URL and type
        }

        void CNetworkReply::reportUnhandled() const
        {
            //if ( fReply )
            //    qDebug().noquote().nospace() << "Reply not handled: " << NMediaManager::NCore::toString( fRequestType ) << " - 0x" << Qt::hex << reinterpret_cast<uintptr_t>( fReply ) << " - " << fURLPathKey;
            //else
            //    qDebug().noquote().nospace() << "Reply not handled: " << NMediaManager::NCore::toString( fRequestType ) << " - Cached -" << fURLPathKey;
        }

        QByteArray CNetworkReply::getData() const
        {
            if ( fReply && fCachedData.isEmpty() )
                fCachedData = fReply->readAll();
            return fCachedData;
        }

        QString CNetworkReply::toString() const
        {
            auto reply = fReply ? QString( "0x%1" ).arg( reinterpret_cast< uintptr_t >( fReply ), 0, 16 ) : QString( "CACHED" );
            auto retVal = QString( "CNetworkReply( %1 - %2 - %3)" ).arg( NMediaManager::NCore::toString( fRequestType ) ).arg( reply ).arg( fURLPathKey );
            return retVal;
        }
    }
}

QDebug operator<<( QDebug debug, const NMediaManager::NCore::CNetworkReply &reply )
{
    debug.noquote().nospace() << reply.toString();
    return debug;
}
