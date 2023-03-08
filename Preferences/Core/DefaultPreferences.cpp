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

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NCore
        {
            QString CPreferences::getDefaultSeasonDirPattern() const
            {
                return R"(Season <season>{ (<season_year>)}:<season_year>)";
            }

            QString CPreferences::getDefaultOutDirPattern( bool forTV ) const
            {
                if ( forTV )
                    return R"(<title>{ (<show_year>)}:<show_year>{ [tmdbid=<show_tmdbid>]}:<show_tmdbid>/<season_dir>)";
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
                static auto defaultValue = QStringList();
                return defaultValue;
            }

            bool CPreferences::getDefaultDeleteCustom() const
            {
                return !getDefaultCustomPathsToDelete().isEmpty();
            }

            QString CPreferences::getDefaultRippedWithMakeMKVRegEX() const
            {
                return R"(^.*_t(?<num>\d+)\.mkv$)";
            }

            bool CPreferences::getDefaultDeleteEXE() const
            {
                return true;
            }

            bool CPreferences::getDefaultDeleteNFO() const
            {
                return true;
            }

            bool CPreferences::getDefaultDeleteBAK() const
            {
                return true;
            }

            bool CPreferences::getDefaultDeleteImages() const
            {
                return true;
            }

            QStringList CPreferences::getDefaultKnownStrings() const
            {
                static auto defaultValue = QStringList(
                    { R"(1080p)",    R"(10bit)",  R"(2.0.h.264)", R"(2.0.h.265)", R"(2160p)",   R"(4K)",       R"(5.1)",         R"(7.1)",         R"(720p)",     R"(8bit)",    R"(AAC2.0.AVC)",    R"(AAC2.0)", R"(AAC5.1)",  R"(AAC)",
                      R"(Amazon)",   R"(AMZN)",   R"(APEX)",      R"(ARROW)",     R"(Atmos)",   R"(ATVP)",     R"(AVC)",         R"(B0MBARDiERS)", R"(BAE)",      R"(BluRay)",  R"(BRRip)",         R"(BTTF)",   R"(BTV)",     R"(CA)",
                      R"(CCBB)",     R"(CMRG)",   R"(CM)",        R"(CUPCAKES)",  R"(DC)",      R"(DD2.0)",    R"(DD5.1)",       R"(DDP5.1)",      R"(DDPA5.1)",  R"(DiN)",     R"(DL.AAC2.0.AVC)", R"(DL.DD+)", R"(DL.DD)",   R"(DSNP)",
                      R"(DTS-HD)",   R"(DTS)",    R"(DUAL)",      R"(DVDRip)",    R"(DVSUX)",   R"(DV)",       R"(EddieSmurfy)", R"(FGT)",         R"(FLUX)",     R"(h.264)",   R"(h.265)",         R"(h264)",   R"(h265)",    R"(HD4U)",
                      R"(HDRip)",    R"(HDR)",    R"(HEVC)",      R"(HMAX)",      R"(IAMABLE)", R"(IMAX)",     R"(iNTERNAL)",    R"(ion10)",       R"(Japhson)",  R"(KINO)",    R"(KNiVES)",        R"(KOGi)",   R"(KORSUB)",  R"(LCHD)",
                      R"(LiMiTED)",  R"(LION)",   R"(LT)",        R"(MA)",        R"(MKV)",     R"(MoviesFD)", R"(MT)",          R"(MX)",          R"(NAISU)",    R"(NF)",      R"(nogrp)",         R"(NTb)",    R"(PCOK)",    R"(PECULATE)",
                      R"(PEGASUS)",  R"(PiGNUS)", R"(PLEW)",      R"(PROPER)",    R"(rarbg)",   R"(rartv)",    R"(REMASTERED)",  R"(REMASTER)",    R"(REMUX)",    R"(ROKU)",    R"(RUMOUR)",        R"(RUSTED)", R"(SDR)",     R"(SHITBOX)",
                      R"(SHOUT)",    R"(SMURF)",  R"(SPARKS)",    R"(SURCODE)",   R"(SWTYBLZ)", R"(TBD)",      R"(TEPES)",       R"(TERMiNAL)",    R"(THUGLiNE)", R"(TrollHD)", R"(Troll)",         R"(TrueHD)", R"(TRUFFLE)", R"(UHD)",
                      R"(UMBRELLA)", R"(UNCUT)",  R"(WAKEUP)",    R"(WEB-DL)",    R"(WebRip)",  R"(WEB)",      R"(x264)",        R"(x265)",        R"(XviDTLF)",  R"(YIFY)",    R"(YTS.AG)",        R"(YTS.AM)", R"(YTS)" } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultKnownExtendedStrings() const
            {
                static auto defaultValue = QStringList( { R"(Extended)", R"(Directors Cut)", R"(Director's Cut)", R"(Director)", R"(Unrated)" } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultIgnoredPaths() const
            {
                static auto defaultValue = QStringList( { R"(.*-ignore)", R"(sub)", R"(subs)", R"(season \d+ \(\d{4}\))" } );
                return defaultValue;
            }

            QVariantMap CPreferences::getDefaultKnownAbbreviations() const
            {
                static auto defaultValue = QVariantMap( { { R"(Dont)", R"(Don't)" }, { R"(NY)", R"(New York)" } } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultKnownHyphenated() const
            {
                static auto defaultValue = QStringList( { R"(Obi-Wan)" } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultSkippedPaths( bool forMediaNaming ) const
            {
                if ( forMediaNaming )
                {
                    static auto defaultValue = QStringList(
                        { R"(.*-ignore)", R"(#recycle)", R"(#recycler)", R"(extra(s)?)", R"(special(s)?)", R"(short(s)?)", R"(scene(s)?)", R"(featurette(s)?)", R"(behind the scene(s)?)", R"(deleted scene(s)?)", R"(interview(s)?)",
                          R"(trailer(s)?)", R"(sample(s)?)" } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue = QStringList( { R"(.*-ignore)", R"(#recycle)", R"(#recycler)" } );
                    return defaultValue;
                }
            }
        }
    }
}