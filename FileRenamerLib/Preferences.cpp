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

#include "Preferences.h"

#include <QSettings>
#include <QProgressDialog>
#include <QStringListModel>
#include <QInputDialog>

#include "SABUtils/ButtonEnabler.h"

namespace NFileRenamerLib
{
    CPreferences *NFileRenamerLib::CPreferences::instance()
    {
        static CPreferences retVal;
        return &retVal;
    }

    CPreferences::CPreferences()
    {
    }

    CPreferences::~CPreferences()
    {
    }

    //QString CPreferences::getDefaultInPattern( bool forTV ) const
    //{
    //    if ( forTV )
    //        return "(?<title>.+)\\.([Ss](?<season>\\d+))([Ee](?<episode>\\d+))(\\.(?<episode_title>.*))?\\.(1080|720|2160)(p|i)?.*";
    //    else
    //        return "(?<title>.+)\\.(?<year>\\d{2,4})\\..*";
    //}


    QString CPreferences::getDefaultOutDirPattern( bool forTV )
    {
        if ( forTV )
            return "<title> - Season <season>";
        else
            return "<title> (<year>)( [tmdbid=<tmdbid>]):<tmdbid>( - <extra_info>):<extra_info>";
    }

    QString CPreferences::getDefaultOutFilePattern( bool forTV )
    {
        if ( forTV )
            return "<title> - S<season>E<episode>( - <episode_title>):<episode_title>( - <extra_info>):<extra_info>";
        else
            return "<title>";
    }

    void CPreferences::setTreatAsTVShowByDefault( bool value )
    {
        QSettings settings;
        return settings.setValue( "TreatAsTVShowByDefault", value );
    }

    bool CPreferences::getTreatAsTVShowByDefault()
    {
        QSettings settings;
        return settings.value( "TreatAsTVShowByDefault", false ).toBool();
    }

    void CPreferences::setExactMatchesOnly( bool value )
    {
        QSettings settings;
        return settings.setValue( "ExactMatchesOnly", value );
    }

    bool CPreferences::getExactMatchesOnly()
    {
        QSettings settings;
        return settings.value( "ExactMatchesOnly", true ).toBool();
    }

    void CPreferences::setMediaDirectory( const QString &dir )
    {
        QSettings settings;
        settings.setValue( "Directory", dir );
    }

    QString CPreferences::getMediaDirectory()
    {
        QSettings settings;
        return settings.value( "Directory", QString() ).toString();
    }

    void CPreferences::setTVOutFilePattern( const QString &value )
    {
        QSettings settings;
        settings.setValue( "OutFilePattern", value );
    }

    QString CPreferences::getTVOutFilePattern()
    {
        QSettings settings;
        settings.beginGroup( "ForTV" );

        return settings.value( "OutFilePattern", getDefaultOutFilePattern( true ) ).toString();
    }

    void CPreferences::setTVOutDirPattern( const QString &value )
    {
        QSettings settings;
        settings.setValue( "OutDirPattern", value );
    }

    QString CPreferences::getTVOutDirPattern()
    {
        QSettings settings;
        settings.beginGroup( "ForTV" );

        return settings.value( "OutDirPattern", getDefaultOutDirPattern( true ) ).toString();
    }

    void CPreferences::setMovieOutFilePattern( const QString &value )
    {
        QSettings settings;
        settings.setValue( "OutFilePattern", value );
    }

    QString CPreferences::getMovieOutFilePattern()
    {
        QSettings settings;
        settings.beginGroup( "ForMovies" );

        return settings.value( "OutFilePattern", getDefaultOutFilePattern( false ) ).toString();
    }

    void CPreferences::setMovieOutDirPattern( const QString &value )
    {
        QSettings settings;
        settings.setValue( "OutDirPattern", value );
    }

    QString CPreferences::getMovieOutDirPattern()
    {
        QSettings settings;
        settings.beginGroup( "ForMovies" );

        return settings.value( "OutDirPattern", getDefaultOutDirPattern( false ) ).toString();
    }

    void CPreferences::setSearchExtensions( const QString &value )
    {
        QSettings settings;
        settings.setValue( "Extensions", value );

    }

    void CPreferences::setSearchExtensions( const QStringList &value )
    {
        setSearchExtensions( value.join( ";" ) );
    }

    QStringList  CPreferences::getSearchExtensions()
    {
        QSettings settings;
        return settings.value( "Extensions", QString( "*.mkv;*.mp4;*.avi;*.mov;*.wmv;*.mpg;*.mpg2;*.idx;*.sub;*.srt" ) ).toString().split( ";" );
    }

    void CPreferences::setKnownStrings( const QStringList &value )
    {
        QSettings settings;
        settings.setValue( "KnownStrings", value );
    }

    QStringList  CPreferences::getKnownStrings()
    {
        auto knownStrings =
            QStringList()
            << "2160p"
            << "1080p"
            << "720p"
            //<< "10bit"
            << "Amazon"
            << "DDP5.1"
            << "DL.DD+"
            << "DL.DD"
            << "DL.AAC2.0.AVC"
            << "AAC2.0.AVC"
            << "AAC2.0"
            << "AVC"
            << "AMZN"
            << "WebRip"
            << "WEB-DL"
            << "WEB"
            << "SDR"
            << "Atmos"
            << "-RUMOUR"
            << "-PECULATE"
            << "2.0.h.264"
            << "2.0.h.265"
            << "h.264"
            << "h.265"
            << "x264"
            << "x265"
            << "h264"
            << "h265"
            << "rarbg"
            << "BluRay"
            << "ion10"
            << "LiMiTED"
            << "DVDRip"
            << "XviDTLF"
            << "TrollHD"
            << "Troll"
            << "nogrp"
            ;

        QSettings settings;
        return settings.value( "KnownStrings", knownStrings ).toStringList();
    }

}