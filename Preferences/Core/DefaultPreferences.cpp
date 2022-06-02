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

#include "Preferences.h"
#include <QTextStream>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NCore
        {
            QString CPreferences::getDefaultOutDirPattern( bool forTV ) const
            {
                if ( forTV )
                    return R"(<title>{ (<show_year>)}:<show_year>/Season <season>)";
                else
                    return R"(<title>{ (<year>)}:<year>{ [tmdbid=<tmdbid>]}:<tmdbid>{ - <extra_info>}:<extra_info>)";
            }

            QString CPreferences::getDefaultOutFilePattern( bool forTV ) const
            {
                if ( forTV )
                    return R"(<title> - S<season>E<episode>{ - <episode_title>}:<episode_title>{ - <extra_info>}:<extra_info>)";
                else
                    return R"(<title>)";
            }

            QStringList CPreferences::getDefaultCustomPathsToDelete() const
            {
                static auto defaultValue =
                    QStringList( {
                                 } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultKnownStrings() const
            {
                static auto defaultValue =
                    QStringList( {
                         R"(2160p)"
                        ,R"(1080p)"
                        ,R"(720p)"
                        ,R"(Amazon)"
                        ,R"(DDP5.1)"
                        ,R"(DL.DD+)"
                        ,R"(DL.DD)"
                        ,R"(DL.AAC2.0.AVC)"
                        ,R"(AAC2.0.AVC)"
                        ,R"(AAC2.0)"
                        ,R"(AVC)"
                        ,R"(AMZN)"
                        ,R"(WebRip)"
                        ,R"(WEB-DL)"
                        ,R"(WEB)"
                        ,R"(SDR)"
                        ,R"(Atmos)"
                        ,R"(RUMOUR)"
                        ,R"(PECULATE)"
                        ,R"(2.0.h.264)"
                        ,R"(2.0.h.265)"
                        ,R"(h.264)"
                        ,R"(h.265)"
                        ,R"(x264)"
                        ,R"(x265)"
                        ,R"(h264)"
                        ,R"(h265)"
                        ,R"(rarbg)"
                        ,R"(BluRay)"
                        ,R"(ion10)"
                        ,R"(LiMiTED)"
                        ,R"(DVDRip)"
                        ,R"(XviDTLF)"
                        ,R"(TrollHD)"
                        ,R"(Troll)"
                        ,R"(nogrp)"
                        ,R"(CM)"
                        ,R"(NF)"
                        ,R"(REMASTERED)"
                        ,R"(PROPER)"
                        ,R"(DC)"
                        ,R"(AAC)"
                        ,R"(DSNP)"
                        ,R"(10bit)"
                        ,R"(HDR)"
                        ,R"(DTS-HD)"
                        ,R"(MA)"
                        ,R"(5.1)"
                        ,R"(SWTYBLZ)"
                        ,R"(YIFY)"
                        ,R"(ATVP)"
                        ,R"(NAISU)"
                        ,R"(CUPCAKES)"
                        ,R"(MoviesFD)"
                        ,R"(SPARKS)"
                        ,R"(DD5.1)"
                        ,R"(REMUX)"
                        ,R"(BRRip)"
                        ,R"(PLEW)"
                        ,R"(Japhson)"
                        ,R"(DTS)"
                        ,R"(FGT)"
                        ,R"(CCBB)"
                        ,R"(IAMABLE)"
                        ,R"(UHD)"
                        ,R"(TrueHD)"
                        ,R"(7.1)"
                        ,R"(TERMiNAL)"
                        ,R"(TEPES)"
                        ,R"(HMAX)"
                        ,R"(BTTF)"
                        ,R"(B0MBARDiERS)"
                        ,R"(THUGLiNE)"
                        ,R"(LCHD)"
                        ,R"(PiGNUS)"
                        ,R"(YTS.AM)"
                        ,R"(HEVC)"
                        ,R"(HD4U)"
                        ,R"(CMRG)"
                        ,R"(DD2.0)"
                        ,R"(NTb)"
                        ,R"(MT)"
                        ,R"(YTS)"
                        ,R"(MX)"
                        ,R"(SHITBOX)"
                        ,R"(TBD)"
                        ,R"(HDRip)"
                        ,R"(KORSUB)"
                        ,R"(8bit)"
                        ,R"(rartv)"
                        ,R"(LT)"
                                 } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultKnownExtendedStrings() const
            {
                static auto defaultValue =
                    QStringList( {
                         R"(Extended)"
                        ,R"(Directors Cut)"
                        ,R"(Director's Cut)"
                        ,R"(Director)"
                        ,R"(Unrated)"
                                 } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultIgnoredPaths() const
            {
                static auto defaultValue =
                    QStringList( {
                         R"(.*-ignore)"
                        ,R"(sub)"
                        ,R"(subs)"
                        ,R"(season \d+)"
                                 } );
                return defaultValue;
            }

            QVariantMap CPreferences::getDefaultKnownAbbreviations() const
            {
                static auto defaultValue =
                    QVariantMap( {
                         { R"(Dont)", R"(Don't)" }
                        ,{ R"(NY)", R"(New York)" }
                                 } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultKnownHyphenated() const
            {
                static auto defaultValue =
                    QStringList( {
                         R"(Obi-Wan)"
                                 } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultSkippedPaths() const
            {
                static auto defaultValue =
                    QStringList( {
                         R"(.*-ignore)"
                        ,R"(#recycle)"
                        ,R"(#recycler)"
                        ,R"(extra(s)?)"
                        ,R"(trailer(s)?)"
                        ,R"(deleted scene(s)?)"
                        ,R"(interview(s)?)"
                        ,R"(featurette(s)?)"
                        ,R"(sample(s)?)"
                                 } );
                return defaultValue;
            }
        }
    }
}