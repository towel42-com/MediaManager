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
#include "SABUtils/FFMpegFormats.h"

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
                static auto defaultValue =
                    QStringList(
                    {
                         R"(1080p)" //
                        ,R"(10bit)" //
                        ,R"(2.0.h.264)" //
                        ,R"(2.0.h.265)" //
                        ,R"(2160p)" //
                        ,R"(4K)" //
                        ,R"(5.1)" //
                        ,R"(7.1)" //
                        ,R"(720p)" //
                        ,R"(8bit)" //
                        ,R"(AAC2.0.AVC)" //
                        ,R"(AAC2.0)" //
                        ,R"(AAC5.1)" //
                        ,R"(AAC)" //
                        ,R"(Amazon)" //
                        ,R"(AMZN)" //
                        ,R"(APEX)" //
                        ,R"(ARROW)" //
                        ,R"(Atmos)" //
                        ,R"(ATVP)" //
                        ,R"(AVC)" //
                        ,R"(B0MBARDiERS)" //
                        ,R"(BAE)" //
                        ,R"(BDRip)" //
                        ,R"(BluRay)" //
                        ,R"(BRRip)" //
                        ,R"(BTTF)" //
                        ,R"(BTV)" //
                        ,R"(CA)" //
                        ,R"(CCBB)" //
                        ,R"(CMRG)" //
                        ,R"(CM)" //
                        ,R"(CUPCAKES)" //
                        ,R"(DC)" //
                        ,R"(DD2.0)" //
                        ,R"(DD5.1)" //
                        ,R"(DDP5.1)" //
                        ,R"(DL.AAC2.0.AVC)" //
                        ,R"(DL.DD+)" //
                        ,R"(DL.DD)" //
                        ,R"(DSNP)" //
                        ,R"(DTS-HD)" //
                        ,R"(DTS)" //
                        ,R"(DUAL)" //
                        ,R"(DVDRip)" //
                        ,R"(DVSUX)" //
                        ,R"(DV)" //
                        ,R"(EddieSmurfy)" //
                        ,R"(FGT)" //
                        ,R"(FLUX)" //
                        ,R"(h.264)" //
                        ,R"(h.265)" //
                        ,R"(h264)" //
                        ,R"(h265)" //
                        ,R"(HD4U)" //
                        ,R"(HDRip)" //
                        ,R"(HDR)" //
                        ,R"(HEVC)" //
                        ,R"(HMAX)" //
                        ,R"(IAMABLE)" //
                        ,R"(IMAX)" //
                        ,R"(iNTERNAL)" //
                        ,R"(ion10)" //
                        ,R"(Japhson)" //
                        ,R"(KNiVES)" //
                        ,R"(KOGi)" //
                        ,R"(KORSUB)" //
                        ,R"(LCHD)" //
                        ,R"(LiMiTED)" //
                        ,R"(LION)" //
                        ,R"(LT)" //
                        ,R"(MA)" //
                        ,R"(MKV)" //
                        ,R"(MoviesFD)" //
                        ,R"(MT)" //
                        ,R"(MX)" //
                        ,R"(NAISU)" //
                        ,R"(NF)" //
                        ,R"(nogrp)" //
                        ,R"(NTb)" //
                        ,R"(PCOK)" //
                        ,R"(PECULATE)" //
                        ,R"(PEGASUS)" //
                        ,R"(PiGNUS)" //
                        ,R"(PLEW)" //
                        ,R"(PROPER)" //
                        ,R"(rarbg)" //
                        ,R"(rartv)" //
                        ,R"(REMASTERED)" //
                        ,R"(REMASTER)" //
                        ,R"(REMUX)" //
                        ,R"(ROKU)" //
                        ,R"(RUMOUR)" //
                        ,R"(RUSTED)" //
                        ,R"(SDR)" //
                        ,R"(SHITBOX)" //
                        ,R"(SHOUT)" //
                        ,R"(SMURF)" //
                        ,R"(SN)" //
                        ,R"(SPARKS)" //
                        ,R"(SURCODE)" //
                        ,R"(SWTYBLZ)" //
                        ,R"(TBD)" //
                        ,R"(TEPES)" //
                        ,R"(TERMiNAL)" //
                        ,R"(THUGLiNE)" //
                        ,R"(TrollHD)" //
                        ,R"(Troll)" //
                        ,R"(TrueHD)" //
                        ,R"(TRUFFLE)" //
                        ,R"(UHD)" //
                        ,R"(UMBRELLA)" //
                        ,R"(UNCUT)" //
                        ,R"(WAKEUP)" //
                        ,R"(WEB-DL)" //
                        ,R"(WebRip)" //
                        ,R"(WEB)" //
                        ,R"(x264)" //
                        ,R"(x265)" //
                        ,R"(XviDTLF)" //
                        ,R"(XViD)" //
                        ,R"(YIFY)" //
                        ,R"(YTS.AG)" //
                        ,R"(YTS.AM)" //
                        ,R"(YTS)" //
                    } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultKnownExtendedStrings() const
            {
                static auto defaultValue =
                    QStringList(
                    {
                         R"(Extended)" //
                        ,R"(Directors Cut)" //
                        ,R"(Director's Cut)" //
                        ,R"(Director)" //
                        ,R"(Unrated)" //
                    } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultIgnoredPaths() const
            {
                static auto defaultValue =
                    QStringList(
                    {
                         R"(.*-ignore)" //
                        ,R"(sub)" //
                        ,R"(subs)" //
                        ,R"(season \d+ \(\d{4}\))" //
                    } );
                return defaultValue;
            }

            QVariantMap CPreferences::getDefaultKnownAbbreviations() const
            {
                static auto defaultValue =
                    QVariantMap(
                    {
                         { R"(Dont)", R"(Don't)" } //
                        ,{ R"(NY)", R"(New York)" } //
                    } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultKnownHyphenated() const
            {
                static auto defaultValue =
                    QStringList(
                    {
                         R"(Obi-Wan)" //
                    } );
                return defaultValue;
            }

            QStringList CPreferences::getDefaultSkippedPaths( bool forMediaNaming ) const
            {
                if ( forMediaNaming )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(.*-ignore)" //
                            ,R"(#recycle)" //
                            ,R"(#recycler)" //
                            ,R"(extra(s)?)" //
                            ,R"(special(s)?)" //
                            ,R"(short(s)?)" //
                            ,R"(scene(s)?)" //
                            ,R"(featurette(s)?)" //
                            ,R"(behind the scene(s)?)" //
                            ,R"(deleted scene(s)?)" //
                            ,R"(interview(s)?)" //
                            ,R"(trailer(s)?)" //
                            ,R"(sample(s)?)" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(.*-ignore)" //
                            ,R"(#recycle)" //
                            ,R"(#recycler)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableAudioEncodersDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(aac - AAC (Advanced Audio Coding))" //
                            ,R"(ac3 - ATSC A/52A (AC-3))" //
                            ,R"(ac3_fixed - ATSC A/52A (AC-3) (codec ac3))" //
                            ,R"(adpcm_adx - SEGA CRI ADX ADPCM)" //
                            ,R"(adpcm_argo - ADPCM Argonaut Games)" //
                            ,R"(g722 - G.722 ADPCM (codec adpcm_g722))" //
                            ,R"(g726 - G.726 ADPCM (codec adpcm_g726))" //
                            ,R"(g726le - G.726 little endian ADPCM ("right-justified") (codec adpcm_g726le))" //
                            ,R"(adpcm_ima_alp - ADPCM IMA High Voltage Software ALP)" //
                            ,R"(adpcm_ima_amv - ADPCM IMA AMV)" //
                            ,R"(adpcm_ima_apm - ADPCM IMA Ubisoft APM)" //
                            ,R"(adpcm_ima_qt - ADPCM IMA QuickTime)" //
                            ,R"(adpcm_ima_ssi - ADPCM IMA Simon & Schuster Interactive)" //
                            ,R"(adpcm_ima_wav - ADPCM IMA WAV)" //
                            ,R"(adpcm_ima_ws - ADPCM IMA Westwood)" //
                            ,R"(adpcm_ms - ADPCM Microsoft)" //
                            ,R"(adpcm_swf - ADPCM Shockwave Flash)" //
                            ,R"(adpcm_yamaha - ADPCM Yamaha)" //
                            ,R"(alac - ALAC (Apple Lossless Audio Codec))" //
                            ,R"(libopencore_amrnb - OpenCORE AMR-NB (Adaptive Multi-Rate Narrow-Band) (codec amr_nb))" //
                            ,R"(libvo_amrwbenc - Android VisualOn AMR-WB (Adaptive Multi-Rate Wide-Band) (codec amr_wb))" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth))" //
                            ,R"(comfortnoise - RFC 3389 comfort noise generator)" //
                            ,R"(eac3 - ATSC A/52 E-AC-3)" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec))" //
                            ,R"(g723_1 - G.723.1)" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2))" //
                            ,R"(mp2fixed - MP2 fixed point (MPEG audio layer 2) (codec mp2))" //
                            ,R"(libtwolame - libtwolame MP2 (MPEG audio layer 2) (codec mp2))" //
                            ,R"(libmp3lame - libmp3lame MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(libshine - libshine MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(nellymoser - Nellymoser Asao)" //
                            ,R"(libopus - libopus Opus (codec opus))" //
                            ,R"(pcm_alaw - PCM A-law / G.711 A-law)" //
                            ,R"(pcm_dvd - PCM signed 16|20|24-bit big-endian for DVD media)" //
                            ,R"(pcm_f32be - PCM 32-bit floating point big-endian)" //
                            ,R"(pcm_f32le - PCM 32-bit floating point little-endian)" //
                            ,R"(pcm_f64be - PCM 64-bit floating point big-endian)" //
                            ,R"(pcm_f64le - PCM 64-bit floating point little-endian)" //
                            ,R"(pcm_mulaw - PCM mu-law / G.711 mu-law)" //
                            ,R"(pcm_s16be - PCM signed 16-bit big-endian)" //
                            ,R"(pcm_s16be_planar - PCM signed 16-bit big-endian planar)" //
                            ,R"(pcm_s16le - PCM signed 16-bit little-endian)" //
                            ,R"(pcm_s16le_planar - PCM signed 16-bit little-endian planar)" //
                            ,R"(pcm_s24be - PCM signed 24-bit big-endian)" //
                            ,R"(pcm_s24daud - PCM D-Cinema audio signed 24-bit)" //
                            ,R"(pcm_s24le - PCM signed 24-bit little-endian)" //
                            ,R"(pcm_s24le_planar - PCM signed 24-bit little-endian planar)" //
                            ,R"(pcm_s32be - PCM signed 32-bit big-endian)" //
                            ,R"(pcm_s32le - PCM signed 32-bit little-endian)" //
                            ,R"(pcm_s32le_planar - PCM signed 32-bit little-endian planar)" //
                            ,R"(pcm_s64be - PCM signed 64-bit big-endian)" //
                            ,R"(pcm_s64le - PCM signed 64-bit little-endian)" //
                            ,R"(pcm_s8 - PCM signed 8-bit)" //
                            ,R"(pcm_s8_planar - PCM signed 8-bit planar)" //
                            ,R"(pcm_u16be - PCM unsigned 16-bit big-endian)" //
                            ,R"(pcm_u16le - PCM unsigned 16-bit little-endian)" //
                            ,R"(pcm_u24be - PCM unsigned 24-bit big-endian)" //
                            ,R"(pcm_u24le - PCM unsigned 24-bit little-endian)" //
                            ,R"(pcm_u32be - PCM unsigned 32-bit big-endian)" //
                            ,R"(pcm_u32le - PCM unsigned 32-bit little-endian)" //
                            ,R"(pcm_u8 - PCM unsigned 8-bit)" //
                            ,R"(pcm_vidc - PCM Archimedes VIDC)" //
                            ,R"(real_144 - RealAudio 1.0 (14.4K) (codec ra_144))" //
                            ,R"(roq_dpcm - id RoQ DPCM)" //
                            ,R"(sbc - SBC (low-complexity subband codec))" //
                            ,R"(libspeex - libspeex Speex (codec speex))" //
                            ,R"(tta - TTA (True Audio))" //
                            ,R"(libvorbis - libvorbis (codec vorbis))" //
                            ,R"(wavpack - WavPack)" //
                            ,R"(wmav1 - Windows Media Audio 1)" //
                            ,R"(wmav2 - Windows Media Audio 2)" //
                            ,R"(aac - AAC (Advanced Audio Coding))" //
                            ,R"(ac3 - ATSC A/52A (AC-3))" //
                            ,R"(ac3_fixed - ATSC A/52A (AC-3) (codec ac3))" //
                            ,R"(adpcm_adx - SEGA CRI ADX ADPCM)" //
                            ,R"(adpcm_argo - ADPCM Argonaut Games)" //
                            ,R"(g722 - G.722 ADPCM (codec adpcm_g722))" //
                            ,R"(g726 - G.726 ADPCM (codec adpcm_g726))" //
                            ,R"(g726le - G.726 little endian ADPCM ("right-justified") (codec adpcm_g726le))" //
                            ,R"(adpcm_ima_alp - ADPCM IMA High Voltage Software ALP)" //
                            ,R"(adpcm_ima_amv - ADPCM IMA AMV)" //
                            ,R"(adpcm_ima_apm - ADPCM IMA Ubisoft APM)" //
                            ,R"(adpcm_ima_qt - ADPCM IMA QuickTime)" //
                            ,R"(adpcm_ima_ssi - ADPCM IMA Simon & Schuster Interactive)" //
                            ,R"(adpcm_ima_wav - ADPCM IMA WAV)" //
                            ,R"(adpcm_ima_ws - ADPCM IMA Westwood)" //
                            ,R"(adpcm_ms - ADPCM Microsoft)" //
                            ,R"(adpcm_swf - ADPCM Shockwave Flash)" //
                            ,R"(adpcm_yamaha - ADPCM Yamaha)" //
                            ,R"(alac - ALAC (Apple Lossless Audio Codec))" //
                            ,R"(libopencore_amrnb - OpenCORE AMR-NB (Adaptive Multi-Rate Narrow-Band) (codec amr_nb))" //
                            ,R"(libvo_amrwbenc - Android VisualOn AMR-WB (Adaptive Multi-Rate Wide-Band) (codec amr_wb))" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth))" //
                            ,R"(comfortnoise - RFC 3389 comfort noise generator)" //
                            ,R"(eac3 - ATSC A/52 E-AC-3)" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec))" //
                            ,R"(g723_1 - G.723.1)" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2))" //
                            ,R"(mp2fixed - MP2 fixed point (MPEG audio layer 2) (codec mp2))" //
                            ,R"(libtwolame - libtwolame MP2 (MPEG audio layer 2) (codec mp2))" //
                            ,R"(libmp3lame - libmp3lame MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(libshine - libshine MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(nellymoser - Nellymoser Asao)" //
                            ,R"(libopus - libopus Opus (codec opus))" //
                            ,R"(pcm_alaw - PCM A-law / G.711 A-law)" //
                            ,R"(pcm_dvd - PCM signed 16|20|24-bit big-endian for DVD media)" //
                            ,R"(pcm_f32be - PCM 32-bit floating point big-endian)" //
                            ,R"(pcm_f32le - PCM 32-bit floating point little-endian)" //
                            ,R"(pcm_f64be - PCM 64-bit floating point big-endian)" //
                            ,R"(pcm_f64le - PCM 64-bit floating point little-endian)" //
                            ,R"(pcm_mulaw - PCM mu-law / G.711 mu-law)" //
                            ,R"(pcm_s16be - PCM signed 16-bit big-endian)" //
                            ,R"(pcm_s16be_planar - PCM signed 16-bit big-endian planar)" //
                            ,R"(pcm_s16le - PCM signed 16-bit little-endian)" //
                            ,R"(pcm_s16le_planar - PCM signed 16-bit little-endian planar)" //
                            ,R"(pcm_s24be - PCM signed 24-bit big-endian)" //
                            ,R"(pcm_s24daud - PCM D-Cinema audio signed 24-bit)" //
                            ,R"(pcm_s24le - PCM signed 24-bit little-endian)" //
                            ,R"(pcm_s24le_planar - PCM signed 24-bit little-endian planar)" //
                            ,R"(pcm_s32be - PCM signed 32-bit big-endian)" //
                            ,R"(pcm_s32le - PCM signed 32-bit little-endian)" //
                            ,R"(pcm_s32le_planar - PCM signed 32-bit little-endian planar)" //
                            ,R"(pcm_s64be - PCM signed 64-bit big-endian)" //
                            ,R"(pcm_s64le - PCM signed 64-bit little-endian)" //
                            ,R"(pcm_s8 - PCM signed 8-bit)" //
                            ,R"(pcm_s8_planar - PCM signed 8-bit planar)" //
                            ,R"(pcm_u16be - PCM unsigned 16-bit big-endian)" //
                            ,R"(pcm_u16le - PCM unsigned 16-bit little-endian)" //
                            ,R"(pcm_u24be - PCM unsigned 24-bit big-endian)" //
                            ,R"(pcm_u24le - PCM unsigned 24-bit little-endian)" //
                            ,R"(pcm_u32be - PCM unsigned 32-bit big-endian)" //
                            ,R"(pcm_u32le - PCM unsigned 32-bit little-endian)" //
                            ,R"(pcm_u8 - PCM unsigned 8-bit)" //
                            ,R"(pcm_vidc - PCM Archimedes VIDC)" //
                            ,R"(real_144 - RealAudio 1.0 (14.4K) (codec ra_144))" //
                            ,R"(roq_dpcm - id RoQ DPCM)" //
                            ,R"(sbc - SBC (low-complexity subband codec))" //
                            ,R"(libspeex - libspeex Speex (codec speex))" //
                            ,R"(tta - TTA (True Audio))" //
                            ,R"(libvorbis - libvorbis (codec vorbis))" //
                            ,R"(wavpack - WavPack)" //
                            ,R"(wmav1 - Windows Media Audio 1)" //
                            ,R"(wmav2 - Windows Media Audio 2)" //
                            ,R"(aac - AAC (Advanced Audio Coding))" //
                            ,R"(ac3 - ATSC A/52A (AC-3))" //
                            ,R"(ac3_fixed - ATSC A/52A (AC-3) (codec ac3))" //
                            ,R"(adpcm_adx - SEGA CRI ADX ADPCM)" //
                            ,R"(adpcm_argo - ADPCM Argonaut Games)" //
                            ,R"(g722 - G.722 ADPCM (codec adpcm_g722))" //
                            ,R"(g726 - G.726 ADPCM (codec adpcm_g726))" //
                            ,R"(g726le - G.726 little endian ADPCM ("right-justified") (codec adpcm_g726le))" //
                            ,R"(adpcm_ima_alp - ADPCM IMA High Voltage Software ALP)" //
                            ,R"(adpcm_ima_amv - ADPCM IMA AMV)" //
                            ,R"(adpcm_ima_apm - ADPCM IMA Ubisoft APM)" //
                            ,R"(adpcm_ima_qt - ADPCM IMA QuickTime)" //
                            ,R"(adpcm_ima_ssi - ADPCM IMA Simon & Schuster Interactive)" //
                            ,R"(adpcm_ima_wav - ADPCM IMA WAV)" //
                            ,R"(adpcm_ima_ws - ADPCM IMA Westwood)" //
                            ,R"(adpcm_ms - ADPCM Microsoft)" //
                            ,R"(adpcm_swf - ADPCM Shockwave Flash)" //
                            ,R"(adpcm_yamaha - ADPCM Yamaha)" //
                            ,R"(alac - ALAC (Apple Lossless Audio Codec))" //
                            ,R"(libopencore_amrnb - OpenCORE AMR-NB (Adaptive Multi-Rate Narrow-Band) (codec amr_nb))" //
                            ,R"(libvo_amrwbenc - Android VisualOn AMR-WB (Adaptive Multi-Rate Wide-Band) (codec amr_wb))" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth))" //
                            ,R"(comfortnoise - RFC 3389 comfort noise generator)" //
                            ,R"(eac3 - ATSC A/52 E-AC-3)" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec))" //
                            ,R"(g723_1 - G.723.1)" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2))" //
                            ,R"(mp2fixed - MP2 fixed point (MPEG audio layer 2) (codec mp2))" //
                            ,R"(libtwolame - libtwolame MP2 (MPEG audio layer 2) (codec mp2))" //
                            ,R"(libmp3lame - libmp3lame MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(libshine - libshine MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(nellymoser - Nellymoser Asao)" //
                            ,R"(libopus - libopus Opus (codec opus))" //
                            ,R"(pcm_alaw - PCM A-law / G.711 A-law)" //
                            ,R"(pcm_dvd - PCM signed 16|20|24-bit big-endian for DVD media)" //
                            ,R"(pcm_f32be - PCM 32-bit floating point big-endian)" //
                            ,R"(pcm_f32le - PCM 32-bit floating point little-endian)" //
                            ,R"(pcm_f64be - PCM 64-bit floating point big-endian)" //
                            ,R"(pcm_f64le - PCM 64-bit floating point little-endian)" //
                            ,R"(pcm_mulaw - PCM mu-law / G.711 mu-law)" //
                            ,R"(pcm_s16be - PCM signed 16-bit big-endian)" //
                            ,R"(pcm_s16be_planar - PCM signed 16-bit big-endian planar)" //
                            ,R"(pcm_s16le - PCM signed 16-bit little-endian)" //
                            ,R"(pcm_s16le_planar - PCM signed 16-bit little-endian planar)" //
                            ,R"(pcm_s24be - PCM signed 24-bit big-endian)" //
                            ,R"(pcm_s24daud - PCM D-Cinema audio signed 24-bit)" //
                            ,R"(pcm_s24le - PCM signed 24-bit little-endian)" //
                            ,R"(pcm_s24le_planar - PCM signed 24-bit little-endian planar)" //
                            ,R"(pcm_s32be - PCM signed 32-bit big-endian)" //
                            ,R"(pcm_s32le - PCM signed 32-bit little-endian)" //
                            ,R"(pcm_s32le_planar - PCM signed 32-bit little-endian planar)" //
                            ,R"(pcm_s64be - PCM signed 64-bit big-endian)" //
                            ,R"(pcm_s64le - PCM signed 64-bit little-endian)" //
                            ,R"(pcm_s8 - PCM signed 8-bit)" //
                            ,R"(pcm_s8_planar - PCM signed 8-bit planar)" //
                            ,R"(pcm_u16be - PCM unsigned 16-bit big-endian)" //
                            ,R"(pcm_u16le - PCM unsigned 16-bit little-endian)" //
                            ,R"(pcm_u24be - PCM unsigned 24-bit big-endian)" //
                            ,R"(pcm_u24le - PCM unsigned 24-bit little-endian)" //
                            ,R"(pcm_u32be - PCM unsigned 32-bit big-endian)" //
                            ,R"(pcm_u32le - PCM unsigned 32-bit little-endian)" //
                            ,R"(pcm_u8 - PCM unsigned 8-bit)" //
                            ,R"(pcm_vidc - PCM Archimedes VIDC)" //
                            ,R"(real_144 - RealAudio 1.0 (14.4K) (codec ra_144))" //
                            ,R"(roq_dpcm - id RoQ DPCM)" //
                            ,R"(sbc - SBC (low-complexity subband codec))" //
                            ,R"(libspeex - libspeex Speex (codec speex))" //
                            ,R"(tta - TTA (True Audio))" //
                            ,R"(libvorbis - libvorbis (codec vorbis))" //
                            ,R"(wavpack - WavPack)" //
                            ,R"(wmav1 - Windows Media Audio 1)" //
                            ,R"(wmav2 - Windows Media Audio 2)" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(aac)" //
                            ,R"(ac3)" //
                            ,R"(ac3_fixed)" //
                            ,R"(adpcm_adx)" //
                            ,R"(adpcm_argo)" //
                            ,R"(g722)" //
                            ,R"(g726)" //
                            ,R"(g726le)" //
                            ,R"(adpcm_ima_alp)" //
                            ,R"(adpcm_ima_amv)" //
                            ,R"(adpcm_ima_apm)" //
                            ,R"(adpcm_ima_qt)" //
                            ,R"(adpcm_ima_ssi)" //
                            ,R"(adpcm_ima_wav)" //
                            ,R"(adpcm_ima_ws)" //
                            ,R"(adpcm_ms)" //
                            ,R"(adpcm_swf)" //
                            ,R"(adpcm_yamaha)" //
                            ,R"(alac)" //
                            ,R"(libopencore_amrnb)" //
                            ,R"(libvo_amrwbenc)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(comfortnoise)" //
                            ,R"(eac3)" //
                            ,R"(flac)" //
                            ,R"(g723_1)" //
                            ,R"(mp2)" //
                            ,R"(mp2fixed)" //
                            ,R"(libtwolame)" //
                            ,R"(libmp3lame)" //
                            ,R"(libshine)" //
                            ,R"(nellymoser)" //
                            ,R"(libopus)" //
                            ,R"(pcm_alaw)" //
                            ,R"(pcm_dvd)" //
                            ,R"(pcm_f32be)" //
                            ,R"(pcm_f32le)" //
                            ,R"(pcm_f64be)" //
                            ,R"(pcm_f64le)" //
                            ,R"(pcm_mulaw)" //
                            ,R"(pcm_s16be)" //
                            ,R"(pcm_s16be_planar)" //
                            ,R"(pcm_s16le)" //
                            ,R"(pcm_s16le_planar)" //
                            ,R"(pcm_s24be)" //
                            ,R"(pcm_s24daud)" //
                            ,R"(pcm_s24le)" //
                            ,R"(pcm_s24le_planar)" //
                            ,R"(pcm_s32be)" //
                            ,R"(pcm_s32le)" //
                            ,R"(pcm_s32le_planar)" //
                            ,R"(pcm_s64be)" //
                            ,R"(pcm_s64le)" //
                            ,R"(pcm_s8)" //
                            ,R"(pcm_s8_planar)" //
                            ,R"(pcm_u16be)" //
                            ,R"(pcm_u16le)" //
                            ,R"(pcm_u24be)" //
                            ,R"(pcm_u24le)" //
                            ,R"(pcm_u32be)" //
                            ,R"(pcm_u32le)" //
                            ,R"(pcm_u8)" //
                            ,R"(pcm_vidc)" //
                            ,R"(real_144)" //
                            ,R"(roq_dpcm)" //
                            ,R"(sbc)" //
                            ,R"(libspeex)" //
                            ,R"(tta)" //
                            ,R"(libvorbis)" //
                            ,R"(wavpack)" //
                            ,R"(wmav1)" //
                            ,R"(wmav2)" //
                            ,R"(aac)" //
                            ,R"(ac3)" //
                            ,R"(ac3_fixed)" //
                            ,R"(adpcm_adx)" //
                            ,R"(adpcm_argo)" //
                            ,R"(g722)" //
                            ,R"(g726)" //
                            ,R"(g726le)" //
                            ,R"(adpcm_ima_alp)" //
                            ,R"(adpcm_ima_amv)" //
                            ,R"(adpcm_ima_apm)" //
                            ,R"(adpcm_ima_qt)" //
                            ,R"(adpcm_ima_ssi)" //
                            ,R"(adpcm_ima_wav)" //
                            ,R"(adpcm_ima_ws)" //
                            ,R"(adpcm_ms)" //
                            ,R"(adpcm_swf)" //
                            ,R"(adpcm_yamaha)" //
                            ,R"(alac)" //
                            ,R"(libopencore_amrnb)" //
                            ,R"(libvo_amrwbenc)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(comfortnoise)" //
                            ,R"(eac3)" //
                            ,R"(flac)" //
                            ,R"(g723_1)" //
                            ,R"(mp2)" //
                            ,R"(mp2fixed)" //
                            ,R"(libtwolame)" //
                            ,R"(libmp3lame)" //
                            ,R"(libshine)" //
                            ,R"(nellymoser)" //
                            ,R"(libopus)" //
                            ,R"(pcm_alaw)" //
                            ,R"(pcm_dvd)" //
                            ,R"(pcm_f32be)" //
                            ,R"(pcm_f32le)" //
                            ,R"(pcm_f64be)" //
                            ,R"(pcm_f64le)" //
                            ,R"(pcm_mulaw)" //
                            ,R"(pcm_s16be)" //
                            ,R"(pcm_s16be_planar)" //
                            ,R"(pcm_s16le)" //
                            ,R"(pcm_s16le_planar)" //
                            ,R"(pcm_s24be)" //
                            ,R"(pcm_s24daud)" //
                            ,R"(pcm_s24le)" //
                            ,R"(pcm_s24le_planar)" //
                            ,R"(pcm_s32be)" //
                            ,R"(pcm_s32le)" //
                            ,R"(pcm_s32le_planar)" //
                            ,R"(pcm_s64be)" //
                            ,R"(pcm_s64le)" //
                            ,R"(pcm_s8)" //
                            ,R"(pcm_s8_planar)" //
                            ,R"(pcm_u16be)" //
                            ,R"(pcm_u16le)" //
                            ,R"(pcm_u24be)" //
                            ,R"(pcm_u24le)" //
                            ,R"(pcm_u32be)" //
                            ,R"(pcm_u32le)" //
                            ,R"(pcm_u8)" //
                            ,R"(pcm_vidc)" //
                            ,R"(real_144)" //
                            ,R"(roq_dpcm)" //
                            ,R"(sbc)" //
                            ,R"(libspeex)" //
                            ,R"(tta)" //
                            ,R"(libvorbis)" //
                            ,R"(wavpack)" //
                            ,R"(wmav1)" //
                            ,R"(wmav2)" //
                            ,R"(aac)" //
                            ,R"(ac3)" //
                            ,R"(ac3_fixed)" //
                            ,R"(adpcm_adx)" //
                            ,R"(adpcm_argo)" //
                            ,R"(g722)" //
                            ,R"(g726)" //
                            ,R"(g726le)" //
                            ,R"(adpcm_ima_alp)" //
                            ,R"(adpcm_ima_amv)" //
                            ,R"(adpcm_ima_apm)" //
                            ,R"(adpcm_ima_qt)" //
                            ,R"(adpcm_ima_ssi)" //
                            ,R"(adpcm_ima_wav)" //
                            ,R"(adpcm_ima_ws)" //
                            ,R"(adpcm_ms)" //
                            ,R"(adpcm_swf)" //
                            ,R"(adpcm_yamaha)" //
                            ,R"(alac)" //
                            ,R"(libopencore_amrnb)" //
                            ,R"(libvo_amrwbenc)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(comfortnoise)" //
                            ,R"(eac3)" //
                            ,R"(flac)" //
                            ,R"(g723_1)" //
                            ,R"(mp2)" //
                            ,R"(mp2fixed)" //
                            ,R"(libtwolame)" //
                            ,R"(libmp3lame)" //
                            ,R"(libshine)" //
                            ,R"(nellymoser)" //
                            ,R"(libopus)" //
                            ,R"(pcm_alaw)" //
                            ,R"(pcm_dvd)" //
                            ,R"(pcm_f32be)" //
                            ,R"(pcm_f32le)" //
                            ,R"(pcm_f64be)" //
                            ,R"(pcm_f64le)" //
                            ,R"(pcm_mulaw)" //
                            ,R"(pcm_s16be)" //
                            ,R"(pcm_s16be_planar)" //
                            ,R"(pcm_s16le)" //
                            ,R"(pcm_s16le_planar)" //
                            ,R"(pcm_s24be)" //
                            ,R"(pcm_s24daud)" //
                            ,R"(pcm_s24le)" //
                            ,R"(pcm_s24le_planar)" //
                            ,R"(pcm_s32be)" //
                            ,R"(pcm_s32le)" //
                            ,R"(pcm_s32le_planar)" //
                            ,R"(pcm_s64be)" //
                            ,R"(pcm_s64le)" //
                            ,R"(pcm_s8)" //
                            ,R"(pcm_s8_planar)" //
                            ,R"(pcm_u16be)" //
                            ,R"(pcm_u16le)" //
                            ,R"(pcm_u24be)" //
                            ,R"(pcm_u24le)" //
                            ,R"(pcm_u32be)" //
                            ,R"(pcm_u32le)" //
                            ,R"(pcm_u8)" //
                            ,R"(pcm_vidc)" //
                            ,R"(real_144)" //
                            ,R"(roq_dpcm)" //
                            ,R"(sbc)" //
                            ,R"(libspeex)" //
                            ,R"(tta)" //
                            ,R"(libvorbis)" //
                            ,R"(wavpack)" //
                            ,R"(wmav1)" //
                            ,R"(wmav2)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableVideoEncodersDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(a64multi - Multicolor charset for Commodore 64 (codec a64_multi))" //
                            ,R"(a64multi5 - Multicolor charset for Commodore 64, extended with 5th color (colram) (codec a64_multi5))" //
                            ,R"(alias_pix - Alias/Wavefront PIX image)" //
                            ,R"(amv - AMV Video)" //
                            ,R"(apng - APNG (Animated Portable Network Graphics) image)" //
                            ,R"(asv1 - ASUS V1)" //
                            ,R"(asv2 - ASUS V2)" //
                            ,R"(avrp - Avid 1:1 10-bit RGB Packer)" //
                            ,R"(ayuv - Uncompressed packed MS 4:4:4:4)" //
                            ,R"(bitpacked - Bitpacked)" //
                            ,R"(bmp - BMP (Windows and OS/2 bitmap))" //
                            ,R"(cfhd - GoPro CineForm HD)" //
                            ,R"(cinepak - Cinepak)" //
                            ,R"(cljr - Cirrus Logic AccuPak)" //
                            ,R"(vc2 - SMPTE VC-2 (codec dirac))" //
                            ,R"(dnxhd - VC3/DNxHD)" //
                            ,R"(dpx - DPX (Digital Picture Exchange) image)" //
                            ,R"(dvvideo - DV (Digital Video))" //
                            ,R"(exr - OpenEXR image)" //
                            ,R"(ffv1 - FFmpeg video codec #1)" //
                            ,R"(ffvhuff - Huffyuv FFmpeg variant)" //
                            ,R"(fits - Flexible Image Transport System)" //
                            ,R"(flashsv - Flash Screen Video)" //
                            ,R"(flashsv2 - Flash Screen Video Version 2)" //
                            ,R"(flv - FLV / Sorenson Spark / Sorenson H.263 (Flash Video) (codec flv1))" //
                            ,R"(gif - GIF (Graphics Interchange Format))" //
                            ,R"(h261 - H.261)" //
                            ,R"(h263 - H.263 / H.263-1996)" //
                            ,R"(h263p - H.263+ / H.263-1998 / H.263 version 2)" //
                            ,R"(libx264 - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264))" //
                            ,R"(libx264rgb - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 RGB (codec h264))" //
                            ,R"(h264_amf - AMD AMF H.264 Encoder (codec h264))" //
                            ,R"(h264_nvenc - NVIDIA NVENC H.264 encoder (codec h264))" //
                            ,R"(h264_qsv - H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (Intel Quick Sync Video acceleration) (codec h264))" //
                            ,R"(hap - Vidvox Hap)" //
                            ,R"(libx265 - libx265 H.265 / HEVC (codec hevc))" //
                            ,R"(hevc_amf - AMD AMF HEVC encoder (codec hevc))" //
                            ,R"(hevc_nvenc - NVIDIA NVENC hevc encoder (codec hevc))" //
                            ,R"(hevc_qsv - HEVC (Intel Quick Sync Video acceleration) (codec hevc))" //
                            ,R"(huffyuv - Huffyuv / HuffYUV)" //
                            ,R"(jpeg2000 - JPEG 2000)" //
                            ,R"(libopenjpeg - OpenJPEG JPEG 2000 (codec jpeg2000))" //
                            ,R"(jpegls - JPEG-LS)" //
                            ,R"(ljpeg - Lossless JPEG)" //
                            ,R"(magicyuv - MagicYUV video)" //
                            ,R"(mjpeg - MJPEG (Motion JPEG))" //
                            ,R"(mjpeg_qsv - MJPEG (Intel Quick Sync Video acceleration) (codec mjpeg))" //
                            ,R"(mpeg1video - MPEG-1 video)" //
                            ,R"(mpeg2video - MPEG-2 video)" //
                            ,R"(mpeg2_qsv - MPEG-2 video (Intel Quick Sync Video acceleration) (codec mpeg2video))" //
                            ,R"(mpeg4 - MPEG-4 part 2)" //
                            ,R"(libxvid - libxvidcore MPEG-4 part 2 (codec mpeg4))" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2)" //
                            ,R"(msmpeg4 - MPEG-4 part 2 Microsoft variant version 3 (codec msmpeg4v3))" //
                            ,R"(msvideo1 - Microsoft Video-1)" //
                            ,R"(pam - PAM (Portable AnyMap) image)" //
                            ,R"(pbm - PBM (Portable BitMap) image)" //
                            ,R"(pcx - PC Paintbrush PCX image)" //
                            ,R"(pfm - PFM (Portable FloatMap) image)" //
                            ,R"(pgm - PGM (Portable GrayMap) image)" //
                            ,R"(pgmyuv - PGMYUV (Portable GrayMap YUV) image)" //
                            ,R"(png - PNG (Portable Network Graphics) image)" //
                            ,R"(ppm - PPM (Portable PixelMap) image)" //
                            ,R"(prores - Apple ProRes)" //
                            ,R"(prores_aw - Apple ProRes (codec prores))" //
                            ,R"(prores_ks - Apple ProRes (iCodec Pro) (codec prores))" //
                            ,R"(qtrle - QuickTime Animation (RLE) video)" //
                            ,R"(r10k - AJA Kona 10-bit RGB Codec)" //
                            ,R"(r210 - Uncompressed RGB 10-bit)" //
                            ,R"(rawvideo - raw video)" //
                            ,R"(roqvideo - id RoQ video (codec roq))" //
                            ,R"(rpza - QuickTime video (RPZA))" //
                            ,R"(rv10 - RealVideo 1.0)" //
                            ,R"(rv20 - RealVideo 2.0)" //
                            ,R"(sgi - SGI image)" //
                            ,R"(smc - QuickTime Graphics (SMC))" //
                            ,R"(snow - Snow)" //
                            ,R"(speedhq - NewTek SpeedHQ)" //
                            ,R"(sunrast - Sun Rasterfile image)" //
                            ,R"(svq1 - Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1)" //
                            ,R"(targa - Truevision Targa image)" //
                            ,R"(libtheora - libtheora Theora (codec theora))" //
                            ,R"(tiff - TIFF image)" //
                            ,R"(utvideo - Ut Video)" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit)" //
                            ,R"(v308 - Uncompressed packed 4:4:4)" //
                            ,R"(v408 - Uncompressed packed QT 4:4:4:4)" //
                            ,R"(v410 - Uncompressed 4:4:4 10-bit)" //
                            ,R"(libvpx - libvpx VP8 (codec vp8))" //
                            ,R"(libvpx-vp9 - libvpx VP9 (codec vp9))" //
                            ,R"(vp9_qsv - VP9 video (Intel Quick Sync Video acceleration) (codec vp9))" //
                            ,R"(libwebp_anim - libwebp WebP image (codec webp))" //
                            ,R"(libwebp - libwebp WebP image (codec webp))" //
                            ,R"(wmv1 - Windows Media Video 7)" //
                            ,R"(wmv2 - Windows Media Video 8)" //
                            ,R"(wrapped_avframe - AVFrame to AVPacket passthrough)" //
                            ,R"(xbm - XBM (X BitMap) image)" //
                            ,R"(xface - X-face image)" //
                            ,R"(xwd - XWD (X Window Dump) image)" //
                            ,R"(y41p - Uncompressed YUV 4:1:1 12-bit)" //
                            ,R"(yuv4 - Uncompressed packed 4:2:0)" //
                            ,R"(zlib - LCL (LossLess Codec Library) ZLIB)" //
                            ,R"(zmbv - Zip Motion Blocks Video)" //
                            ,R"(a64multi - Multicolor charset for Commodore 64 (codec a64_multi))" //
                            ,R"(a64multi5 - Multicolor charset for Commodore 64, extended with 5th color (colram) (codec a64_multi5))" //
                            ,R"(alias_pix - Alias/Wavefront PIX image)" //
                            ,R"(amv - AMV Video)" //
                            ,R"(apng - APNG (Animated Portable Network Graphics) image)" //
                            ,R"(asv1 - ASUS V1)" //
                            ,R"(asv2 - ASUS V2)" //
                            ,R"(avrp - Avid 1:1 10-bit RGB Packer)" //
                            ,R"(ayuv - Uncompressed packed MS 4:4:4:4)" //
                            ,R"(bitpacked - Bitpacked)" //
                            ,R"(bmp - BMP (Windows and OS/2 bitmap))" //
                            ,R"(cfhd - GoPro CineForm HD)" //
                            ,R"(cinepak - Cinepak)" //
                            ,R"(cljr - Cirrus Logic AccuPak)" //
                            ,R"(vc2 - SMPTE VC-2 (codec dirac))" //
                            ,R"(dnxhd - VC3/DNxHD)" //
                            ,R"(dpx - DPX (Digital Picture Exchange) image)" //
                            ,R"(dvvideo - DV (Digital Video))" //
                            ,R"(exr - OpenEXR image)" //
                            ,R"(ffv1 - FFmpeg video codec #1)" //
                            ,R"(ffvhuff - Huffyuv FFmpeg variant)" //
                            ,R"(fits - Flexible Image Transport System)" //
                            ,R"(flashsv - Flash Screen Video)" //
                            ,R"(flashsv2 - Flash Screen Video Version 2)" //
                            ,R"(flv - FLV / Sorenson Spark / Sorenson H.263 (Flash Video) (codec flv1))" //
                            ,R"(gif - GIF (Graphics Interchange Format))" //
                            ,R"(h261 - H.261)" //
                            ,R"(h263 - H.263 / H.263-1996)" //
                            ,R"(h263p - H.263+ / H.263-1998 / H.263 version 2)" //
                            ,R"(libx264 - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264))" //
                            ,R"(libx264rgb - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 RGB (codec h264))" //
                            ,R"(h264_amf - AMD AMF H.264 Encoder (codec h264))" //
                            ,R"(h264_nvenc - NVIDIA NVENC H.264 encoder (codec h264))" //
                            ,R"(h264_qsv - H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (Intel Quick Sync Video acceleration) (codec h264))" //
                            ,R"(hap - Vidvox Hap)" //
                            ,R"(libx265 - libx265 H.265 / HEVC (codec hevc))" //
                            ,R"(hevc_amf - AMD AMF HEVC encoder (codec hevc))" //
                            ,R"(hevc_nvenc - NVIDIA NVENC hevc encoder (codec hevc))" //
                            ,R"(hevc_qsv - HEVC (Intel Quick Sync Video acceleration) (codec hevc))" //
                            ,R"(huffyuv - Huffyuv / HuffYUV)" //
                            ,R"(jpeg2000 - JPEG 2000)" //
                            ,R"(libopenjpeg - OpenJPEG JPEG 2000 (codec jpeg2000))" //
                            ,R"(jpegls - JPEG-LS)" //
                            ,R"(ljpeg - Lossless JPEG)" //
                            ,R"(magicyuv - MagicYUV video)" //
                            ,R"(mjpeg - MJPEG (Motion JPEG))" //
                            ,R"(mjpeg_qsv - MJPEG (Intel Quick Sync Video acceleration) (codec mjpeg))" //
                            ,R"(mpeg1video - MPEG-1 video)" //
                            ,R"(mpeg2video - MPEG-2 video)" //
                            ,R"(mpeg2_qsv - MPEG-2 video (Intel Quick Sync Video acceleration) (codec mpeg2video))" //
                            ,R"(mpeg4 - MPEG-4 part 2)" //
                            ,R"(libxvid - libxvidcore MPEG-4 part 2 (codec mpeg4))" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2)" //
                            ,R"(msmpeg4 - MPEG-4 part 2 Microsoft variant version 3 (codec msmpeg4v3))" //
                            ,R"(msvideo1 - Microsoft Video-1)" //
                            ,R"(pam - PAM (Portable AnyMap) image)" //
                            ,R"(pbm - PBM (Portable BitMap) image)" //
                            ,R"(pcx - PC Paintbrush PCX image)" //
                            ,R"(pfm - PFM (Portable FloatMap) image)" //
                            ,R"(pgm - PGM (Portable GrayMap) image)" //
                            ,R"(pgmyuv - PGMYUV (Portable GrayMap YUV) image)" //
                            ,R"(png - PNG (Portable Network Graphics) image)" //
                            ,R"(ppm - PPM (Portable PixelMap) image)" //
                            ,R"(prores - Apple ProRes)" //
                            ,R"(prores_aw - Apple ProRes (codec prores))" //
                            ,R"(prores_ks - Apple ProRes (iCodec Pro) (codec prores))" //
                            ,R"(qtrle - QuickTime Animation (RLE) video)" //
                            ,R"(r10k - AJA Kona 10-bit RGB Codec)" //
                            ,R"(r210 - Uncompressed RGB 10-bit)" //
                            ,R"(rawvideo - raw video)" //
                            ,R"(roqvideo - id RoQ video (codec roq))" //
                            ,R"(rpza - QuickTime video (RPZA))" //
                            ,R"(rv10 - RealVideo 1.0)" //
                            ,R"(rv20 - RealVideo 2.0)" //
                            ,R"(sgi - SGI image)" //
                            ,R"(smc - QuickTime Graphics (SMC))" //
                            ,R"(snow - Snow)" //
                            ,R"(speedhq - NewTek SpeedHQ)" //
                            ,R"(sunrast - Sun Rasterfile image)" //
                            ,R"(svq1 - Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1)" //
                            ,R"(targa - Truevision Targa image)" //
                            ,R"(libtheora - libtheora Theora (codec theora))" //
                            ,R"(tiff - TIFF image)" //
                            ,R"(utvideo - Ut Video)" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit)" //
                            ,R"(v308 - Uncompressed packed 4:4:4)" //
                            ,R"(v408 - Uncompressed packed QT 4:4:4:4)" //
                            ,R"(v410 - Uncompressed 4:4:4 10-bit)" //
                            ,R"(libvpx - libvpx VP8 (codec vp8))" //
                            ,R"(libvpx-vp9 - libvpx VP9 (codec vp9))" //
                            ,R"(vp9_qsv - VP9 video (Intel Quick Sync Video acceleration) (codec vp9))" //
                            ,R"(libwebp_anim - libwebp WebP image (codec webp))" //
                            ,R"(libwebp - libwebp WebP image (codec webp))" //
                            ,R"(wmv1 - Windows Media Video 7)" //
                            ,R"(wmv2 - Windows Media Video 8)" //
                            ,R"(wrapped_avframe - AVFrame to AVPacket passthrough)" //
                            ,R"(xbm - XBM (X BitMap) image)" //
                            ,R"(xface - X-face image)" //
                            ,R"(xwd - XWD (X Window Dump) image)" //
                            ,R"(y41p - Uncompressed YUV 4:1:1 12-bit)" //
                            ,R"(yuv4 - Uncompressed packed 4:2:0)" //
                            ,R"(zlib - LCL (LossLess Codec Library) ZLIB)" //
                            ,R"(zmbv - Zip Motion Blocks Video)" //
                            ,R"(a64multi - Multicolor charset for Commodore 64 (codec a64_multi))" //
                            ,R"(a64multi5 - Multicolor charset for Commodore 64, extended with 5th color (colram) (codec a64_multi5))" //
                            ,R"(alias_pix - Alias/Wavefront PIX image)" //
                            ,R"(amv - AMV Video)" //
                            ,R"(apng - APNG (Animated Portable Network Graphics) image)" //
                            ,R"(asv1 - ASUS V1)" //
                            ,R"(asv2 - ASUS V2)" //
                            ,R"(avrp - Avid 1:1 10-bit RGB Packer)" //
                            ,R"(ayuv - Uncompressed packed MS 4:4:4:4)" //
                            ,R"(bitpacked - Bitpacked)" //
                            ,R"(bmp - BMP (Windows and OS/2 bitmap))" //
                            ,R"(cfhd - GoPro CineForm HD)" //
                            ,R"(cinepak - Cinepak)" //
                            ,R"(cljr - Cirrus Logic AccuPak)" //
                            ,R"(vc2 - SMPTE VC-2 (codec dirac))" //
                            ,R"(dnxhd - VC3/DNxHD)" //
                            ,R"(dpx - DPX (Digital Picture Exchange) image)" //
                            ,R"(dvvideo - DV (Digital Video))" //
                            ,R"(exr - OpenEXR image)" //
                            ,R"(ffv1 - FFmpeg video codec #1)" //
                            ,R"(ffvhuff - Huffyuv FFmpeg variant)" //
                            ,R"(fits - Flexible Image Transport System)" //
                            ,R"(flashsv - Flash Screen Video)" //
                            ,R"(flashsv2 - Flash Screen Video Version 2)" //
                            ,R"(flv - FLV / Sorenson Spark / Sorenson H.263 (Flash Video) (codec flv1))" //
                            ,R"(gif - GIF (Graphics Interchange Format))" //
                            ,R"(h261 - H.261)" //
                            ,R"(h263 - H.263 / H.263-1996)" //
                            ,R"(h263p - H.263+ / H.263-1998 / H.263 version 2)" //
                            ,R"(libx264 - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264))" //
                            ,R"(libx264rgb - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 RGB (codec h264))" //
                            ,R"(h264_amf - AMD AMF H.264 Encoder (codec h264))" //
                            ,R"(h264_nvenc - NVIDIA NVENC H.264 encoder (codec h264))" //
                            ,R"(h264_qsv - H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (Intel Quick Sync Video acceleration) (codec h264))" //
                            ,R"(hap - Vidvox Hap)" //
                            ,R"(libx265 - libx265 H.265 / HEVC (codec hevc))" //
                            ,R"(hevc_amf - AMD AMF HEVC encoder (codec hevc))" //
                            ,R"(hevc_nvenc - NVIDIA NVENC hevc encoder (codec hevc))" //
                            ,R"(hevc_qsv - HEVC (Intel Quick Sync Video acceleration) (codec hevc))" //
                            ,R"(huffyuv - Huffyuv / HuffYUV)" //
                            ,R"(jpeg2000 - JPEG 2000)" //
                            ,R"(libopenjpeg - OpenJPEG JPEG 2000 (codec jpeg2000))" //
                            ,R"(jpegls - JPEG-LS)" //
                            ,R"(ljpeg - Lossless JPEG)" //
                            ,R"(magicyuv - MagicYUV video)" //
                            ,R"(mjpeg - MJPEG (Motion JPEG))" //
                            ,R"(mjpeg_qsv - MJPEG (Intel Quick Sync Video acceleration) (codec mjpeg))" //
                            ,R"(mpeg1video - MPEG-1 video)" //
                            ,R"(mpeg2video - MPEG-2 video)" //
                            ,R"(mpeg2_qsv - MPEG-2 video (Intel Quick Sync Video acceleration) (codec mpeg2video))" //
                            ,R"(mpeg4 - MPEG-4 part 2)" //
                            ,R"(libxvid - libxvidcore MPEG-4 part 2 (codec mpeg4))" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2)" //
                            ,R"(msmpeg4 - MPEG-4 part 2 Microsoft variant version 3 (codec msmpeg4v3))" //
                            ,R"(msvideo1 - Microsoft Video-1)" //
                            ,R"(pam - PAM (Portable AnyMap) image)" //
                            ,R"(pbm - PBM (Portable BitMap) image)" //
                            ,R"(pcx - PC Paintbrush PCX image)" //
                            ,R"(pfm - PFM (Portable FloatMap) image)" //
                            ,R"(pgm - PGM (Portable GrayMap) image)" //
                            ,R"(pgmyuv - PGMYUV (Portable GrayMap YUV) image)" //
                            ,R"(png - PNG (Portable Network Graphics) image)" //
                            ,R"(ppm - PPM (Portable PixelMap) image)" //
                            ,R"(prores - Apple ProRes)" //
                            ,R"(prores_aw - Apple ProRes (codec prores))" //
                            ,R"(prores_ks - Apple ProRes (iCodec Pro) (codec prores))" //
                            ,R"(qtrle - QuickTime Animation (RLE) video)" //
                            ,R"(r10k - AJA Kona 10-bit RGB Codec)" //
                            ,R"(r210 - Uncompressed RGB 10-bit)" //
                            ,R"(rawvideo - raw video)" //
                            ,R"(roqvideo - id RoQ video (codec roq))" //
                            ,R"(rpza - QuickTime video (RPZA))" //
                            ,R"(rv10 - RealVideo 1.0)" //
                            ,R"(rv20 - RealVideo 2.0)" //
                            ,R"(sgi - SGI image)" //
                            ,R"(smc - QuickTime Graphics (SMC))" //
                            ,R"(snow - Snow)" //
                            ,R"(speedhq - NewTek SpeedHQ)" //
                            ,R"(sunrast - Sun Rasterfile image)" //
                            ,R"(svq1 - Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1)" //
                            ,R"(targa - Truevision Targa image)" //
                            ,R"(libtheora - libtheora Theora (codec theora))" //
                            ,R"(tiff - TIFF image)" //
                            ,R"(utvideo - Ut Video)" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit)" //
                            ,R"(v308 - Uncompressed packed 4:4:4)" //
                            ,R"(v408 - Uncompressed packed QT 4:4:4:4)" //
                            ,R"(v410 - Uncompressed 4:4:4 10-bit)" //
                            ,R"(libvpx - libvpx VP8 (codec vp8))" //
                            ,R"(libvpx-vp9 - libvpx VP9 (codec vp9))" //
                            ,R"(vp9_qsv - VP9 video (Intel Quick Sync Video acceleration) (codec vp9))" //
                            ,R"(libwebp_anim - libwebp WebP image (codec webp))" //
                            ,R"(libwebp - libwebp WebP image (codec webp))" //
                            ,R"(wmv1 - Windows Media Video 7)" //
                            ,R"(wmv2 - Windows Media Video 8)" //
                            ,R"(wrapped_avframe - AVFrame to AVPacket passthrough)" //
                            ,R"(xbm - XBM (X BitMap) image)" //
                            ,R"(xface - X-face image)" //
                            ,R"(xwd - XWD (X Window Dump) image)" //
                            ,R"(y41p - Uncompressed YUV 4:1:1 12-bit)" //
                            ,R"(yuv4 - Uncompressed packed 4:2:0)" //
                            ,R"(zlib - LCL (LossLess Codec Library) ZLIB)" //
                            ,R"(zmbv - Zip Motion Blocks Video)" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(a64multi)" //
                            ,R"(a64multi5)" //
                            ,R"(alias_pix)" //
                            ,R"(amv)" //
                            ,R"(apng)" //
                            ,R"(asv1)" //
                            ,R"(asv2)" //
                            ,R"(avrp)" //
                            ,R"(ayuv)" //
                            ,R"(bitpacked)" //
                            ,R"(bmp)" //
                            ,R"(cfhd)" //
                            ,R"(cinepak)" //
                            ,R"(cljr)" //
                            ,R"(vc2)" //
                            ,R"(dnxhd)" //
                            ,R"(dpx)" //
                            ,R"(dvvideo)" //
                            ,R"(exr)" //
                            ,R"(ffv1)" //
                            ,R"(ffvhuff)" //
                            ,R"(fits)" //
                            ,R"(flashsv)" //
                            ,R"(flashsv2)" //
                            ,R"(flv)" //
                            ,R"(gif)" //
                            ,R"(h261)" //
                            ,R"(h263)" //
                            ,R"(h263p)" //
                            ,R"(libx264)" //
                            ,R"(libx264rgb)" //
                            ,R"(h264_amf)" //
                            ,R"(h264_nvenc)" //
                            ,R"(h264_qsv)" //
                            ,R"(hap)" //
                            ,R"(libx265)" //
                            ,R"(hevc_amf)" //
                            ,R"(hevc_nvenc)" //
                            ,R"(hevc_qsv)" //
                            ,R"(huffyuv)" //
                            ,R"(jpeg2000)" //
                            ,R"(libopenjpeg)" //
                            ,R"(jpegls)" //
                            ,R"(ljpeg)" //
                            ,R"(magicyuv)" //
                            ,R"(mjpeg)" //
                            ,R"(mjpeg_qsv)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpeg2_qsv)" //
                            ,R"(mpeg4)" //
                            ,R"(libxvid)" //
                            ,R"(msmpeg4v2)" //
                            ,R"(msmpeg4)" //
                            ,R"(msvideo1)" //
                            ,R"(pam)" //
                            ,R"(pbm)" //
                            ,R"(pcx)" //
                            ,R"(pfm)" //
                            ,R"(pgm)" //
                            ,R"(pgmyuv)" //
                            ,R"(png)" //
                            ,R"(ppm)" //
                            ,R"(prores)" //
                            ,R"(prores_aw)" //
                            ,R"(prores_ks)" //
                            ,R"(qtrle)" //
                            ,R"(r10k)" //
                            ,R"(r210)" //
                            ,R"(rawvideo)" //
                            ,R"(roqvideo)" //
                            ,R"(rpza)" //
                            ,R"(rv10)" //
                            ,R"(rv20)" //
                            ,R"(sgi)" //
                            ,R"(smc)" //
                            ,R"(snow)" //
                            ,R"(speedhq)" //
                            ,R"(sunrast)" //
                            ,R"(svq1)" //
                            ,R"(targa)" //
                            ,R"(libtheora)" //
                            ,R"(tiff)" //
                            ,R"(utvideo)" //
                            ,R"(v210)" //
                            ,R"(v308)" //
                            ,R"(v408)" //
                            ,R"(v410)" //
                            ,R"(libvpx)" //
                            ,R"(libvpx-vp9)" //
                            ,R"(vp9_qsv)" //
                            ,R"(libwebp_anim)" //
                            ,R"(libwebp)" //
                            ,R"(wmv1)" //
                            ,R"(wmv2)" //
                            ,R"(wrapped_avframe)" //
                            ,R"(xbm)" //
                            ,R"(xface)" //
                            ,R"(xwd)" //
                            ,R"(y41p)" //
                            ,R"(yuv4)" //
                            ,R"(zlib)" //
                            ,R"(zmbv)" //
                            ,R"(a64multi)" //
                            ,R"(a64multi5)" //
                            ,R"(alias_pix)" //
                            ,R"(amv)" //
                            ,R"(apng)" //
                            ,R"(asv1)" //
                            ,R"(asv2)" //
                            ,R"(avrp)" //
                            ,R"(ayuv)" //
                            ,R"(bitpacked)" //
                            ,R"(bmp)" //
                            ,R"(cfhd)" //
                            ,R"(cinepak)" //
                            ,R"(cljr)" //
                            ,R"(vc2)" //
                            ,R"(dnxhd)" //
                            ,R"(dpx)" //
                            ,R"(dvvideo)" //
                            ,R"(exr)" //
                            ,R"(ffv1)" //
                            ,R"(ffvhuff)" //
                            ,R"(fits)" //
                            ,R"(flashsv)" //
                            ,R"(flashsv2)" //
                            ,R"(flv)" //
                            ,R"(gif)" //
                            ,R"(h261)" //
                            ,R"(h263)" //
                            ,R"(h263p)" //
                            ,R"(libx264)" //
                            ,R"(libx264rgb)" //
                            ,R"(h264_amf)" //
                            ,R"(h264_nvenc)" //
                            ,R"(h264_qsv)" //
                            ,R"(hap)" //
                            ,R"(libx265)" //
                            ,R"(hevc_amf)" //
                            ,R"(hevc_nvenc)" //
                            ,R"(hevc_qsv)" //
                            ,R"(huffyuv)" //
                            ,R"(jpeg2000)" //
                            ,R"(libopenjpeg)" //
                            ,R"(jpegls)" //
                            ,R"(ljpeg)" //
                            ,R"(magicyuv)" //
                            ,R"(mjpeg)" //
                            ,R"(mjpeg_qsv)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpeg2_qsv)" //
                            ,R"(mpeg4)" //
                            ,R"(libxvid)" //
                            ,R"(msmpeg4v2)" //
                            ,R"(msmpeg4)" //
                            ,R"(msvideo1)" //
                            ,R"(pam)" //
                            ,R"(pbm)" //
                            ,R"(pcx)" //
                            ,R"(pfm)" //
                            ,R"(pgm)" //
                            ,R"(pgmyuv)" //
                            ,R"(png)" //
                            ,R"(ppm)" //
                            ,R"(prores)" //
                            ,R"(prores_aw)" //
                            ,R"(prores_ks)" //
                            ,R"(qtrle)" //
                            ,R"(r10k)" //
                            ,R"(r210)" //
                            ,R"(rawvideo)" //
                            ,R"(roqvideo)" //
                            ,R"(rpza)" //
                            ,R"(rv10)" //
                            ,R"(rv20)" //
                            ,R"(sgi)" //
                            ,R"(smc)" //
                            ,R"(snow)" //
                            ,R"(speedhq)" //
                            ,R"(sunrast)" //
                            ,R"(svq1)" //
                            ,R"(targa)" //
                            ,R"(libtheora)" //
                            ,R"(tiff)" //
                            ,R"(utvideo)" //
                            ,R"(v210)" //
                            ,R"(v308)" //
                            ,R"(v408)" //
                            ,R"(v410)" //
                            ,R"(libvpx)" //
                            ,R"(libvpx-vp9)" //
                            ,R"(vp9_qsv)" //
                            ,R"(libwebp_anim)" //
                            ,R"(libwebp)" //
                            ,R"(wmv1)" //
                            ,R"(wmv2)" //
                            ,R"(wrapped_avframe)" //
                            ,R"(xbm)" //
                            ,R"(xface)" //
                            ,R"(xwd)" //
                            ,R"(y41p)" //
                            ,R"(yuv4)" //
                            ,R"(zlib)" //
                            ,R"(zmbv)" //
                            ,R"(a64multi)" //
                            ,R"(a64multi5)" //
                            ,R"(alias_pix)" //
                            ,R"(amv)" //
                            ,R"(apng)" //
                            ,R"(asv1)" //
                            ,R"(asv2)" //
                            ,R"(avrp)" //
                            ,R"(ayuv)" //
                            ,R"(bitpacked)" //
                            ,R"(bmp)" //
                            ,R"(cfhd)" //
                            ,R"(cinepak)" //
                            ,R"(cljr)" //
                            ,R"(vc2)" //
                            ,R"(dnxhd)" //
                            ,R"(dpx)" //
                            ,R"(dvvideo)" //
                            ,R"(exr)" //
                            ,R"(ffv1)" //
                            ,R"(ffvhuff)" //
                            ,R"(fits)" //
                            ,R"(flashsv)" //
                            ,R"(flashsv2)" //
                            ,R"(flv)" //
                            ,R"(gif)" //
                            ,R"(h261)" //
                            ,R"(h263)" //
                            ,R"(h263p)" //
                            ,R"(libx264)" //
                            ,R"(libx264rgb)" //
                            ,R"(h264_amf)" //
                            ,R"(h264_nvenc)" //
                            ,R"(h264_qsv)" //
                            ,R"(hap)" //
                            ,R"(libx265)" //
                            ,R"(hevc_amf)" //
                            ,R"(hevc_nvenc)" //
                            ,R"(hevc_qsv)" //
                            ,R"(huffyuv)" //
                            ,R"(jpeg2000)" //
                            ,R"(libopenjpeg)" //
                            ,R"(jpegls)" //
                            ,R"(ljpeg)" //
                            ,R"(magicyuv)" //
                            ,R"(mjpeg)" //
                            ,R"(mjpeg_qsv)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpeg2_qsv)" //
                            ,R"(mpeg4)" //
                            ,R"(libxvid)" //
                            ,R"(msmpeg4v2)" //
                            ,R"(msmpeg4)" //
                            ,R"(msvideo1)" //
                            ,R"(pam)" //
                            ,R"(pbm)" //
                            ,R"(pcx)" //
                            ,R"(pfm)" //
                            ,R"(pgm)" //
                            ,R"(pgmyuv)" //
                            ,R"(png)" //
                            ,R"(ppm)" //
                            ,R"(prores)" //
                            ,R"(prores_aw)" //
                            ,R"(prores_ks)" //
                            ,R"(qtrle)" //
                            ,R"(r10k)" //
                            ,R"(r210)" //
                            ,R"(rawvideo)" //
                            ,R"(roqvideo)" //
                            ,R"(rpza)" //
                            ,R"(rv10)" //
                            ,R"(rv20)" //
                            ,R"(sgi)" //
                            ,R"(smc)" //
                            ,R"(snow)" //
                            ,R"(speedhq)" //
                            ,R"(sunrast)" //
                            ,R"(svq1)" //
                            ,R"(targa)" //
                            ,R"(libtheora)" //
                            ,R"(tiff)" //
                            ,R"(utvideo)" //
                            ,R"(v210)" //
                            ,R"(v308)" //
                            ,R"(v408)" //
                            ,R"(v410)" //
                            ,R"(libvpx)" //
                            ,R"(libvpx-vp9)" //
                            ,R"(vp9_qsv)" //
                            ,R"(libwebp_anim)" //
                            ,R"(libwebp)" //
                            ,R"(wmv1)" //
                            ,R"(wmv2)" //
                            ,R"(wrapped_avframe)" //
                            ,R"(xbm)" //
                            ,R"(xface)" //
                            ,R"(xwd)" //
                            ,R"(y41p)" //
                            ,R"(yuv4)" //
                            ,R"(zlib)" //
                            ,R"(zmbv)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableSubtitleEncodersDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(ssa - ASS (Advanced SubStation Alpha) subtitle (codec ass))" //
                            ,R"(ass - ASS (Advanced SubStation Alpha) subtitle)" //
                            ,R"(dvbsub - DVB subtitles (codec dvb_subtitle))" //
                            ,R"(dvdsub - DVD subtitles (codec dvd_subtitle))" //
                            ,R"(mov_text - 3GPP Timed Text subtitle)" //
                            ,R"(srt - SubRip subtitle (codec subrip))" //
                            ,R"(subrip - SubRip subtitle)" //
                            ,R"(text - Raw text subtitle)" //
                            ,R"(ttml - TTML subtitle)" //
                            ,R"(webvtt - WebVTT subtitle)" //
                            ,R"(xsub - DivX subtitles (XSUB))" //
                            ,R"(ssa - ASS (Advanced SubStation Alpha) subtitle (codec ass))" //
                            ,R"(ass - ASS (Advanced SubStation Alpha) subtitle)" //
                            ,R"(dvbsub - DVB subtitles (codec dvb_subtitle))" //
                            ,R"(dvdsub - DVD subtitles (codec dvd_subtitle))" //
                            ,R"(mov_text - 3GPP Timed Text subtitle)" //
                            ,R"(srt - SubRip subtitle (codec subrip))" //
                            ,R"(subrip - SubRip subtitle)" //
                            ,R"(text - Raw text subtitle)" //
                            ,R"(ttml - TTML subtitle)" //
                            ,R"(webvtt - WebVTT subtitle)" //
                            ,R"(xsub - DivX subtitles (XSUB))" //
                            ,R"(ssa - ASS (Advanced SubStation Alpha) subtitle (codec ass))" //
                            ,R"(ass - ASS (Advanced SubStation Alpha) subtitle)" //
                            ,R"(dvbsub - DVB subtitles (codec dvb_subtitle))" //
                            ,R"(dvdsub - DVD subtitles (codec dvd_subtitle))" //
                            ,R"(mov_text - 3GPP Timed Text subtitle)" //
                            ,R"(srt - SubRip subtitle (codec subrip))" //
                            ,R"(subrip - SubRip subtitle)" //
                            ,R"(text - Raw text subtitle)" //
                            ,R"(ttml - TTML subtitle)" //
                            ,R"(webvtt - WebVTT subtitle)" //
                            ,R"(xsub - DivX subtitles (XSUB))" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(ssa)" //
                            ,R"(ass)" //
                            ,R"(dvbsub)" //
                            ,R"(dvdsub)" //
                            ,R"(mov_text)" //
                            ,R"(srt)" //
                            ,R"(subrip)" //
                            ,R"(text)" //
                            ,R"(ttml)" //
                            ,R"(webvtt)" //
                            ,R"(xsub)" //
                            ,R"(ssa)" //
                            ,R"(ass)" //
                            ,R"(dvbsub)" //
                            ,R"(dvdsub)" //
                            ,R"(mov_text)" //
                            ,R"(srt)" //
                            ,R"(subrip)" //
                            ,R"(text)" //
                            ,R"(ttml)" //
                            ,R"(webvtt)" //
                            ,R"(xsub)" //
                            ,R"(ssa)" //
                            ,R"(ass)" //
                            ,R"(dvbsub)" //
                            ,R"(dvdsub)" //
                            ,R"(mov_text)" //
                            ,R"(srt)" //
                            ,R"(subrip)" //
                            ,R"(text)" //
                            ,R"(ttml)" //
                            ,R"(webvtt)" //
                            ,R"(xsub)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableMediaFormatsDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(3g2 - 3GP2 (3GPP2 file format) (*.3g2))" //
                            ,R"(3gp - 3GP (3GPP file format) (*.3gp))" //
                            ,R"(a64 - a64 - video for Commodore 64 (*.a64))" //
                            ,R"(ac3 - raw AC-3 (*.ac3))" //
                            ,R"(adts - ADTS AAC (Advanced Audio Coding) (*.aac;*.adts))" //
                            ,R"(adx - CRI ADX (*.adx))" //
                            ,R"(aiff - Audio IFF (*.aif;*.aiff;*.afc;*.aifc))" //
                            ,R"(alaw - PCM A-law (*.al))" //
                            ,R"(alp - LEGO Racers ALP (*.tun;*.pcm))" //
                            ,R"(amr - 3GPP AMR (*.amr))" //
                            ,R"(amv - AMV (*.amv))" //
                            ,R"(apm - Ubisoft Rayman 2 APM (*.apm))" //
                            ,R"(apng - Animated Portable Network Graphics (*.apng))" //
                            ,R"(aptx - raw aptX (Audio Processing Technology for Bluetooth) (*.aptx))" //
                            ,R"(aptx_hd - raw aptX HD (Audio Processing Technology for Bluetooth) (*.aptxhd))" //
                            ,R"(argo_cvg - Argonaut Games CVG (*.cvg))" //
                            ,R"(asf - ASF (Advanced / Active Streaming Format) (*.asf;*.wmv;*.wma))" //
                            ,R"(asf_stream - ASF (Advanced / Active Streaming Format) (*.asf;*.wmv;*.wma))" //
                            ,R"(ass - SSA (SubStation Alpha) subtitle (*.ass;*.ssa))" //
                            ,R"(ast - AST (Audio Stream) (*.ast))" //
                            ,R"(au - Sun AU (*.au))" //
                            ,R"(avi - AVI (Audio Video Interleaved) (*.avi))" //
                            ,R"(avs2 - raw AVS2-P2/IEEE1857.4 video (*.avs;*.avs2))" //
                            ,R"(avs3 - AVS3-P2/IEEE1857.10 (*.avs3))" //
                            ,R"(bit - G.729 BIT file format (*.bit))" //
                            ,R"(caf - Apple CAF (Core Audio Format) (*.caf))" //
                            ,R"(cavsvideo - raw Chinese AVS (Audio Video Standard) video (*.cavs))" //
                            ,R"(codec2 - codec2 .c2 muxer (*.c2))" //
                            ,R"(dash - DASH Muxer (*.mpd))" //
                            ,R"(daud - D-Cinema audio (*.302))" //
                            ,R"(dirac - raw Dirac (*.drc;*.vc2))" //
                            ,R"(dnxhd - raw DNxHD (SMPTE VC-3) (*.dnxhd;*.dnxhr))" //
                            ,R"(dts - raw DTS (*.dts))" //
                            ,R"(dv - DV (Digital Video) (*.dv))" //
                            ,R"(dvd - MPEG-2 PS (DVD VOB) (*.dvd))" //
                            ,R"(eac3 - raw E-AC-3 (*.eac3))" //
                            ,R"(f4v - F4V Adobe Flash Video (*.f4v))" //
                            ,R"(film_cpk - Sega FILM / CPK (*.cpk))" //
                            ,R"(filmstrip - Adobe Filmstrip (*.flm))" //
                            ,R"(fits - Flexible Image Transport System (*.fits))" //
                            ,R"(flac - raw FLAC (*.flac))" //
                            ,R"(flv - FLV (Flash Video) (*.flv))" //
                            ,R"(g722 - raw G.722 (*.g722))" //
                            ,R"(g723_1 - raw G.723.1 (*.tco;*.rco))" //
                            ,R"(gif - CompuServe Graphics Interchange Format (GIF) (*.gif))" //
                            ,R"(gsm - raw GSM (*.gsm))" //
                            ,R"(gxf - GXF (General eXchange Format) (*.gxf))" //
                            ,R"(h261 - raw H.261 (*.h261))" //
                            ,R"(h263 - raw H.263 (*.h263))" //
                            ,R"(h264 - raw H.264 video (*.h264;*.264))" //
                            ,R"(hevc - raw HEVC video (*.hevc;*.h265;*.265))" //
                            ,R"(hls - Apple HTTP Live Streaming (*.m3u8))" //
                            ,R"(ico - Microsoft Windows ICO (*.ico))" //
                            ,R"(ilbc - iLBC storage (*.lbc))" //
                            ,R"(image2 - image2 sequence (*.bmp;*.dpx;*.exr;*.jls;*.jpeg;*.jpg;*.ljpg;*.pam;*.pbm;*.pcx;*.pfm;*.pgm;*.pgmyuv;*.png;*.ppm;*.sgi;*.tga;*.tif;*.tiff;*.jp2;*.j2c;*.j2k;*.xwd;*.sun;*.ras;*.rs;*.im1;*.im8;*.im24;*.sunras;*.xbm;*.xface;*.pix;*.y))" //
                            ,R"(ipod - iPod H.264 MP4 (MPEG-4 Part 14) (*.m4v;*.m4a;*.m4b))" //
                            ,R"(ircam - Berkeley/IRCAM/CARL Sound Format (*.sf;*.ircam))" //
                            ,R"(ismv - ISMV/ISMA (Smooth Streaming) (*.ismv;*.isma))" //
                            ,R"(ivf - On2 IVF (*.ivf))" //
                            ,R"(jacosub - JACOsub subtitle format (*.jss;*.js))" //
                            ,R"(kvag - Simon & Schuster Interactive VAG (*.vag))" //
                            ,R"(latm - LOAS/LATM (*.latm;*.loas))" //
                            ,R"(lrc - LRC lyrics (*.lrc))" //
                            ,R"(m4v - raw MPEG-4 video (*.m4v))" //
                            ,R"(matroska - Matroska (*.mkv))" //
                            ,R"(microdvd - MicroDVD subtitle format (*.sub))" //
                            ,R"(mjpeg - raw MJPEG video (*.mjpg;*.mjpeg))" //
                            ,R"(mlp - raw MLP (*.mlp))" //
                            ,R"(mmf - Yamaha SMAF (*.mmf))" //
                            ,R"(mov - QuickTime / MOV (*.mov))" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2) (*.mp2;*.m2a;*.mpa))" //
                            ,R"(mp3 - MP3 (MPEG audio layer 3) (*.mp3))" //
                            ,R"(mp4 - MP4 (MPEG-4 Part 14) (*.mp4))" //
                            ,R"(mpeg - MPEG-1 Systems / MPEG program stream (*.mpg;*.mpeg))" //
                            ,R"(mpeg1video - raw MPEG-1 video (*.mpg;*.mpeg;*.m1v))" //
                            ,R"(mpeg2video - raw MPEG-2 video (*.m2v))" //
                            ,R"(mpegts - MPEG-TS (MPEG-2 Transport Stream) (*.ts;*.m2t;*.m2ts;*.mts))" //
                            ,R"(mpjpeg - MIME multipart JPEG (*.mjpg))" //
                            ,R"(mulaw - PCM mu-law (*.ul))" //
                            ,R"(mxf - MXF (Material eXchange Format) (*.mxf))" //
                            ,R"(mxf_opatom - MXF (Material eXchange Format) Operational Pattern Atom (*.mxf))" //
                            ,R"(nut - NUT (*.nut))" //
                            ,R"(obu - AV1 low overhead OBU (*.obu))" //
                            ,R"(oga - Ogg Audio (*.oga))" //
                            ,R"(ogg - Ogg (*.ogg))" //
                            ,R"(ogv - Ogg Video (*.ogv))" //
                            ,R"(oma - Sony OpenMG audio (*.oma))" //
                            ,R"(opus - Ogg Opus (*.opus))" //
                            ,R"(psp - PSP MP4 (MPEG-4 Part 14) (*.mp4;*.psp))" //
                            ,R"(rawvideo - raw video (*.yuv;*.rgb))" //
                            ,R"(rm - RealMedia (*.rm;*.ra))" //
                            ,R"(roq - raw id RoQ (*.roq))" //
                            ,R"(rso - Lego Mindstorms RSO (*.rso))" //
                            ,R"(s16le - PCM signed 16-bit little-endian (*.sw))" //
                            ,R"(s8 - PCM signed 8-bit (*.sb))" //
                            ,R"(sbc - raw SBC (*.sbc;*.msbc))" //
                            ,R"(scc - Scenarist Closed Captions (*.scc))" //
                            ,R"(sox - SoX native (*.sox))" //
                            ,R"(spdif - IEC 61937 (used on S/PDIF - IEC958) (*.spdif))" //
                            ,R"(spx - Ogg Speex (*.spx))" //
                            ,R"(srt - SubRip subtitle (*.srt))" //
                            ,R"(sup - raw HDMV Presentation Graphic Stream subtitles (*.sup))" //
                            ,R"(svcd - MPEG-2 PS (SVCD) (*.vob))" //
                            ,R"(swf - SWF (ShockWave Flash) (*.swf))" //
                            ,R"(truehd - raw TrueHD (*.thd))" //
                            ,R"(tta - TTA (True Audio) (*.tta))" //
                            ,R"(ttml - TTML subtitle (*.ttml))" //
                            ,R"(u16le - PCM unsigned 16-bit little-endian (*.uw))" //
                            ,R"(u8 - PCM unsigned 8-bit (*.ub))" //
                            ,R"(vc1 - raw VC-1 video (*.vc1))" //
                            ,R"(vc1test - VC-1 test bitstream (*.rcv))" //
                            ,R"(vob - MPEG-2 PS (VOB) (*.vob))" //
                            ,R"(voc - Creative Voice (*.voc))" //
                            ,R"(w64 - Sony Wave64 (*.w64))" //
                            ,R"(wav - WAV / WAVE (Waveform Audio) (*.wav))" //
                            ,R"(webm - WebM (*.webm))" //
                            ,R"(webm_chunk - WebM Chunk Muxer (*.chk))" //
                            ,R"(webp - WebP (*.webp))" //
                            ,R"(webvtt - WebVTT subtitle (*.vtt))" //
                            ,R"(wsaud - Westwood Studios audio (*.aud))" //
                            ,R"(wtv - Windows Television (WTV) (*.wtv))" //
                            ,R"(wv - raw WavPack (*.wv))" //
                            ,R"(yuv4mpegpipe - YUV4MPEG pipe (*.y4m))" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(3g2)" //
                            ,R"(3gp)" //
                            ,R"(a64)" //
                            ,R"(ac3)" //
                            ,R"(adts)" //
                            ,R"(adx)" //
                            ,R"(aiff)" //
                            ,R"(alaw)" //
                            ,R"(alp)" //
                            ,R"(amr)" //
                            ,R"(amv)" //
                            ,R"(apm)" //
                            ,R"(apng)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(argo_cvg)" //
                            ,R"(asf)" //
                            ,R"(asf_stream)" //
                            ,R"(ass)" //
                            ,R"(ast)" //
                            ,R"(au)" //
                            ,R"(avi)" //
                            ,R"(avs2)" //
                            ,R"(avs3)" //
                            ,R"(bit)" //
                            ,R"(caf)" //
                            ,R"(cavsvideo)" //
                            ,R"(codec2)" //
                            ,R"(dash)" //
                            ,R"(daud)" //
                            ,R"(dirac)" //
                            ,R"(dnxhd)" //
                            ,R"(dts)" //
                            ,R"(dv)" //
                            ,R"(dvd)" //
                            ,R"(eac3)" //
                            ,R"(f4v)" //
                            ,R"(film_cpk)" //
                            ,R"(filmstrip)" //
                            ,R"(fits)" //
                            ,R"(flac)" //
                            ,R"(flv)" //
                            ,R"(g722)" //
                            ,R"(g723_1)" //
                            ,R"(gif)" //
                            ,R"(gsm)" //
                            ,R"(gxf)" //
                            ,R"(h261)" //
                            ,R"(h263)" //
                            ,R"(h264)" //
                            ,R"(hevc)" //
                            ,R"(hls)" //
                            ,R"(ico)" //
                            ,R"(ilbc)" //
                            ,R"(image2)" //
                            ,R"(ipod)" //
                            ,R"(ircam)" //
                            ,R"(ismv)" //
                            ,R"(ivf)" //
                            ,R"(jacosub)" //
                            ,R"(kvag)" //
                            ,R"(latm)" //
                            ,R"(lrc)" //
                            ,R"(m4v)" //
                            ,R"(matroska)" //
                            ,R"(microdvd)" //
                            ,R"(mjpeg)" //
                            ,R"(mlp)" //
                            ,R"(mmf)" //
                            ,R"(mov)" //
                            ,R"(mp2)" //
                            ,R"(mp3)" //
                            ,R"(mp4)" //
                            ,R"(mpeg)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpegts)" //
                            ,R"(mpjpeg)" //
                            ,R"(mulaw)" //
                            ,R"(mxf)" //
                            ,R"(mxf_opatom)" //
                            ,R"(nut)" //
                            ,R"(obu)" //
                            ,R"(oga)" //
                            ,R"(ogg)" //
                            ,R"(ogv)" //
                            ,R"(oma)" //
                            ,R"(opus)" //
                            ,R"(psp)" //
                            ,R"(rawvideo)" //
                            ,R"(rm)" //
                            ,R"(roq)" //
                            ,R"(rso)" //
                            ,R"(s16le)" //
                            ,R"(s8)" //
                            ,R"(sbc)" //
                            ,R"(scc)" //
                            ,R"(sox)" //
                            ,R"(spdif)" //
                            ,R"(spx)" //
                            ,R"(srt)" //
                            ,R"(sup)" //
                            ,R"(svcd)" //
                            ,R"(swf)" //
                            ,R"(truehd)" //
                            ,R"(tta)" //
                            ,R"(ttml)" //
                            ,R"(u16le)" //
                            ,R"(u8)" //
                            ,R"(vc1)" //
                            ,R"(vc1test)" //
                            ,R"(vob)" //
                            ,R"(voc)" //
                            ,R"(w64)" //
                            ,R"(wav)" //
                            ,R"(webm)" //
                            ,R"(webm_chunk)" //
                            ,R"(webp)" //
                            ,R"(webvtt)" //
                            ,R"(wsaud)" //
                            ,R"(wtv)" //
                            ,R"(wv)" //
                            ,R"(yuv4mpegpipe)" //
                        } );
                    return defaultValue;
                }
            }

            NSABUtils::TFormatMap CPreferences::getVideoExtensionsMapDefault() const
            {
                static auto defaultValue = NSABUtils::TFormatMap(
                    {
                         {
                            NSABUtils::EFormatType::eVideo, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(mxf_d10)", QStringList( { } ) } //
                                ,{ R"(mov)", QStringList( { R"(*.mov)" } ) } //
                                ,{ R"(f4v)", QStringList( { R"(*.f4v)" } ) } //
                                ,{ R"(3g2)", QStringList( { R"(*.3g2)" } ) } //
                                ,{ R"(3gp)", QStringList( { R"(*.3gp)" } ) } //
                                ,{ R"(gxf)", QStringList( { R"(*.gxf)" } ) } //
                                ,{ R"(dvd)", QStringList( { R"(*.dvd)" } ) } //
                                ,{ R"(avm2)", QStringList( { } ) } //
                                ,{ R"(a64)", QStringList( { R"(*.a64)" } ) } //
                                ,{ R"(asf)", QStringList( { R"(*.asf)", R"(*.wmv)", R"(*.wma)" } ) } //
                                ,{ R"(mpeg)", QStringList( { R"(*.mpg)", R"(*.mpeg)" } ) } //
                                ,{ R"(amv)", QStringList( { R"(*.amv)" } ) } //
                                ,{ R"(framemd5)", QStringList( { } ) } //
                                ,{ R"(dash)", QStringList( { R"(*.mpd)" } ) } //
                                ,{ R"(asf_stream)", QStringList( { R"(*.asf)", R"(*.wmv)", R"(*.wma)" } ) } //
                                ,{ R"(ivf)", QStringList( { R"(*.ivf)" } ) } //
                                ,{ R"(psp)", QStringList( { R"(*.mp4)", R"(*.psp)" } ) } //
                                ,{ R"(avi)", QStringList( { R"(*.avi)" } ) } //
                                ,{ R"(h261)", QStringList( { R"(*.h261)" } ) } //
                                ,{ R"(yuv4mpegpipe)", QStringList( { R"(*.y4m)" } ) } //
                                ,{ R"(obu)", QStringList( { R"(*.obu)" } ) } //
                                ,{ R"(avs2)", QStringList( { R"(*.avs)", R"(*.avs2)" } ) } //
                                ,{ R"(mpjpeg)", QStringList( { R"(*.mjpg)" } ) } //
                                ,{ R"(avs3)", QStringList( { R"(*.avs3)" } ) } //
                                ,{ R"(mxf_opatom)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(cavsvideo)", QStringList( { R"(*.cavs)" } ) } //
                                ,{ R"(crc)", QStringList( { } ) } //
                                ,{ R"(dirac)", QStringList( { R"(*.drc)", R"(*.vc2)" } ) } //
                                ,{ R"(dnxhd)", QStringList( { R"(*.dnxhd)", R"(*.dnxhr)" } ) } //
                                ,{ R"(hash)", QStringList( { } ) } //
                                ,{ R"(dv)", QStringList( { R"(*.dv)" } ) } //
                                ,{ R"(svcd)", QStringList( { R"(*.vob)" } ) } //
                                ,{ R"(mkvtimestamp_v2)", QStringList( { } ) } //
                                ,{ R"(ogg)", QStringList( { R"(*.ogg)" } ) } //
                                ,{ R"(mpegts)", QStringList( { R"(*.ts)", R"(*.m2t)", R"(*.m2ts)", R"(*.mts)" } ) } //
                                ,{ R"(mxf)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(film_cpk)", QStringList( { R"(*.cpk)" } ) } //
                                ,{ R"(filmstrip)", QStringList( { R"(*.flm)" } ) } //
                                ,{ R"(hds)", QStringList( { } ) } //
                                ,{ R"(fits)", QStringList( { R"(*.fits)" } ) } //
                                ,{ R"(ipod)", QStringList( { R"(*.m4v)", R"(*.m4a)", R"(*.m4b)" } ) } //
                                ,{ R"(rtp_mpegts)", QStringList( { } ) } //
                                ,{ R"(rm)", QStringList( { R"(*.rm)", R"(*.ra)" } ) } //
                                ,{ R"(hls)", QStringList( { R"(*.m3u8)" } ) } //
                                ,{ R"(flv)", QStringList( { R"(*.flv)" } ) } //
                                ,{ R"(framecrc)", QStringList( { } ) } //
                                ,{ R"(framehash)", QStringList( { } ) } //
                                ,{ R"(h263)", QStringList( { R"(*.h263)" } ) } //
                                ,{ R"(h264)", QStringList( { R"(*.h264)", R"(*.264)" } ) } //
                                ,{ R"(rtp)", QStringList( { } ) } //
                                ,{ R"(hevc)", QStringList( { R"(*.hevc)", R"(*.h265)", R"(*.265)" } ) } //
                                ,{ R"(image2pipe)", QStringList( { } ) } //
                                ,{ R"(ismv)", QStringList( { R"(*.ismv)", R"(*.isma)" } ) } //
                                ,{ R"(m4v)", QStringList( { R"(*.m4v)" } ) } //
                                ,{ R"(matroska)", QStringList( { R"(*.mkv)" } ) } //
                                ,{ R"(md5)", QStringList( { } ) } //
                                ,{ R"(mjpeg)", QStringList( { R"(*.mjpg)", R"(*.mjpeg)" } ) } //
                                ,{ R"(mp4)", QStringList( { R"(*.mp4)" } ) } //
                                ,{ R"(mpeg1video)", QStringList( { R"(*.mpg)", R"(*.mpeg)", R"(*.m1v)" } ) } //
                                ,{ R"(mpeg2video)", QStringList( { R"(*.m2v)" } ) } //
                                ,{ R"(null)", QStringList( { } ) } //
                                ,{ R"(nut)", QStringList( { R"(*.nut)" } ) } //
                                ,{ R"(ogv)", QStringList( { R"(*.ogv)" } ) } //
                                ,{ R"(rawvideo)", QStringList( { R"(*.yuv)", R"(*.rgb)" } ) } //
                                ,{ R"(roq)", QStringList( { R"(*.roq)" } ) } //
                                ,{ R"(rtsp)", QStringList( { } ) } //
                                ,{ R"(sap)", QStringList( { } ) } //
                                ,{ R"(smjpeg)", QStringList( { } ) } //
                                ,{ R"(smoothstreaming)", QStringList( { } ) } //
                                ,{ R"(streamhash)", QStringList( { } ) } //
                                ,{ R"(swf)", QStringList( { R"(*.swf)" } ) } //
                                ,{ R"(uncodedframecrc)", QStringList( { } ) } //
                                ,{ R"(vc1)", QStringList( { R"(*.vc1)" } ) } //
                                ,{ R"(vc1test)", QStringList( { R"(*.rcv)" } ) } //
                                ,{ R"(vcd)", QStringList( { } ) } //
                                ,{ R"(vob)", QStringList( { R"(*.vob)" } ) } //
                                ,{ R"(webm)", QStringList( { R"(*.webm)" } ) } //
                                ,{ R"(webm_chunk)", QStringList( { R"(*.chk)" } ) } //
                                ,{ R"(wtv)", QStringList( { R"(*.wtv)" } ) } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eAudio, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(sbc)", QStringList( { R"(*.sbc)", R"(*.msbc)" } ) } //
                                ,{ R"(latm)", QStringList( { R"(*.latm)", R"(*.loas)" } ) } //
                                ,{ R"(chromaprint)", QStringList( { } ) } //
                                ,{ R"(ac3)", QStringList( { R"(*.ac3)" } ) } //
                                ,{ R"(alaw)", QStringList( { R"(*.al)" } ) } //
                                ,{ R"(adts)", QStringList( { R"(*.aac)", R"(*.adts)" } ) } //
                                ,{ R"(adx)", QStringList( { R"(*.adx)" } ) } //
                                ,{ R"(mulaw)", QStringList( { R"(*.ul)" } ) } //
                                ,{ R"(amr)", QStringList( { R"(*.amr)" } ) } //
                                ,{ R"(aiff)", QStringList( { R"(*.aif)", R"(*.aiff)", R"(*.afc)", R"(*.aifc)" } ) } //
                                ,{ R"(f32le)", QStringList( { } ) } //
                                ,{ R"(mmf)", QStringList( { R"(*.mmf)" } ) } //
                                ,{ R"(alp)", QStringList( { R"(*.tun)", R"(*.pcm)" } ) } //
                                ,{ R"(apm)", QStringList( { R"(*.apm)" } ) } //
                                ,{ R"(dts)", QStringList( { R"(*.dts)" } ) } //
                                ,{ R"(aptx)", QStringList( { R"(*.aptx)" } ) } //
                                ,{ R"(aptx_hd)", QStringList( { R"(*.aptxhd)" } ) } //
                                ,{ R"(ircam)", QStringList( { R"(*.sf)", R"(*.ircam)" } ) } //
                                ,{ R"(argo_asf)", QStringList( { } ) } //
                                ,{ R"(opus)", QStringList( { R"(*.opus)" } ) } //
                                ,{ R"(u16be)", QStringList( { } ) } //
                                ,{ R"(argo_cvg)", QStringList( { R"(*.cvg)" } ) } //
                                ,{ R"(ast)", QStringList( { R"(*.ast)" } ) } //
                                ,{ R"(bit)", QStringList( { R"(*.bit)" } ) } //
                                ,{ R"(au)", QStringList( { R"(*.au)" } ) } //
                                ,{ R"(f64le)", QStringList( { } ) } //
                                ,{ R"(mp3)", QStringList( { R"(*.mp3)" } ) } //
                                ,{ R"(caf)", QStringList( { R"(*.caf)" } ) } //
                                ,{ R"(s32le)", QStringList( { } ) } //
                                ,{ R"(codec2)", QStringList( { R"(*.c2)" } ) } //
                                ,{ R"(codec2raw)", QStringList( { } ) } //
                                ,{ R"(daud)", QStringList( { R"(*.302)" } ) } //
                                ,{ R"(eac3)", QStringList( { R"(*.eac3)" } ) } //
                                ,{ R"(f32be)", QStringList( { } ) } //
                                ,{ R"(gsm)", QStringList( { R"(*.gsm)" } ) } //
                                ,{ R"(rso)", QStringList( { R"(*.rso)" } ) } //
                                ,{ R"(s32be)", QStringList( { } ) } //
                                ,{ R"(s8)", QStringList( { R"(*.sb)" } ) } //
                                ,{ R"(f64be)", QStringList( { } ) } //
                                ,{ R"(flac)", QStringList( { R"(*.flac)" } ) } //
                                ,{ R"(mlp)", QStringList( { R"(*.mlp)" } ) } //
                                ,{ R"(g722)", QStringList( { R"(*.g722)" } ) } //
                                ,{ R"(s24le)", QStringList( { } ) } //
                                ,{ R"(g723_1)", QStringList( { R"(*.tco)", R"(*.rco)" } ) } //
                                ,{ R"(g726)", QStringList( { } ) } //
                                ,{ R"(g726le)", QStringList( { } ) } //
                                ,{ R"(ilbc)", QStringList( { R"(*.lbc)" } ) } //
                                ,{ R"(kvag)", QStringList( { R"(*.vag)" } ) } //
                                ,{ R"(mp2)", QStringList( { R"(*.mp2)", R"(*.m2a)", R"(*.mpa)" } ) } //
                                ,{ R"(oga)", QStringList( { R"(*.oga)" } ) } //
                                ,{ R"(oma)", QStringList( { R"(*.oma)" } ) } //
                                ,{ R"(s16be)", QStringList( { } ) } //
                                ,{ R"(s16le)", QStringList( { R"(*.sw)" } ) } //
                                ,{ R"(s24be)", QStringList( { } ) } //
                                ,{ R"(sox)", QStringList( { R"(*.sox)" } ) } //
                                ,{ R"(spdif)", QStringList( { R"(*.spdif)" } ) } //
                                ,{ R"(u32be)", QStringList( { } ) } //
                                ,{ R"(spx)", QStringList( { R"(*.spx)" } ) } //
                                ,{ R"(truehd)", QStringList( { R"(*.thd)" } ) } //
                                ,{ R"(tta)", QStringList( { R"(*.tta)" } ) } //
                                ,{ R"(u16le)", QStringList( { R"(*.uw)" } ) } //
                                ,{ R"(u24be)", QStringList( { } ) } //
                                ,{ R"(u24le)", QStringList( { } ) } //
                                ,{ R"(u32le)", QStringList( { } ) } //
                                ,{ R"(u8)", QStringList( { R"(*.ub)" } ) } //
                                ,{ R"(vidc)", QStringList( { } ) } //
                                ,{ R"(voc)", QStringList( { R"(*.voc)" } ) } //
                                ,{ R"(w64)", QStringList( { R"(*.w64)" } ) } //
                                ,{ R"(wav)", QStringList( { R"(*.wav)" } ) } //
                                ,{ R"(wsaud)", QStringList( { R"(*.aud)" } ) } //
                                ,{ R"(wv)", QStringList( { R"(*.wv)" } ) } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eImage, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(apng)", QStringList( { R"(*.apng)" } ) } //
                                ,{ R"(webp)", QStringList( { R"(*.webp)" } ) } //
                                ,{ R"(gif)", QStringList( { R"(*.gif)" } ) } //
                                ,{ R"(ico)", QStringList( { R"(*.ico)" } ) } //
                                ,{ R"(image2)", QStringList( { R"(*.bmp)", R"(*.dpx)", R"(*.exr)", R"(*.jls)", R"(*.jpeg)", R"(*.jpg)", R"(*.ljpg)", R"(*.pam)", R"(*.pbm)", R"(*.pcx)", R"(*.pfm)", R"(*.pgm)", R"(*.pgmyuv)", R"(*.png)", R"(*.ppm)", R"(*.sgi)", R"(*.tga)", R"(*.tif)", R"(*.tiff)", R"(*.jp2)", R"(*.j2c)", R"(*.j2k)", R"(*.xwd)", R"(*.sun)", R"(*.ras)", R"(*.rs)", R"(*.im1)", R"(*.im8)", R"(*.im24)", R"(*.sunras)", R"(*.xbm)", R"(*.xface)", R"(*.pix)", R"(*.y)" } ) } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eSubtitle, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(lrc)", QStringList( { R"(*.lrc)" } ) } //
                                ,{ R"(srt)", QStringList( { R"(*.srt)" } ) } //
                                ,{ R"(ass)", QStringList( { R"(*.ass)", R"(*.ssa)" } ) } //
                                ,{ R"(jacosub)", QStringList( { R"(*.jss)", R"(*.js)" } ) } //
                                ,{ R"(scc)", QStringList( { R"(*.scc)" } ) } //
                                ,{ R"(microdvd)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(sup)", QStringList( { R"(*.sup)" } ) } //
                                ,{ R"(ttml)", QStringList( { R"(*.ttml)" } ) } //
                                ,{ R"(webvtt)", QStringList( { R"(*.vtt)" } ) } //
                            } )
                        }
                    } );
                return defaultValue;
            }

            bool CPreferences::getForceMediaContainerDefault() const
            {
                return true;
            }

            QString CPreferences::getForceMediaContainerNameDefault() const
            {
                return R"(matroska)";
            }

            bool CPreferences::getTranscodeAudioDefault() const
            {
                return true;
            }

            bool CPreferences::getOnlyTranscodeAudioOnFormatChangeDefault() const
            {
                return true;
            }

            QString CPreferences::getTranscodeToAudioCodecDefault() const
            {
                return R"(eac3)";
            }

            QStringList CPreferences::getAllowedAudioCodecsDefault() const
            {
                static auto defaultValue =
                    QStringList(
                    {
                         R"(aac)" //
                    } );
                return defaultValue;
            }

            bool CPreferences::getTranscodeToH265Default() const
            {
                return true;
            }

            bool CPreferences::getOnlyTranscodeVideoOnFormatChangeDefault() const
            {
                return true;
            }

            bool CPreferences::getLosslessTranscodingDefault() const
            {
                return true;
            }

            bool CPreferences::getUseCRFDefault() const
            {
                return false;
            }

            bool CPreferences::getUseExplicitCRFDefault() const
            {
                return false;
            }

            int CPreferences::getExplicitCRFDefault() const
            {
                return 28;
            }

            bool CPreferences::getUsePresetDefault() const
            {
                return false;
            }

            EMakeMKVPreset CPreferences::getPresetDefault() const
            {
                return NMediaManager::NPreferences::NCore::EMakeMKVPreset::eMedium;
            }

            bool CPreferences::getUseTuneDefault() const
            {
                return false;
            }

            EMakeMKVTune CPreferences::getTuneDefault() const
            {
                return NMediaManager::NPreferences::NCore::EMakeMKVTune::eFilm;
            }

            bool CPreferences::getUseProfileDefault() const
            {
                return false;
            }

            EMakeMKVProfile CPreferences::getProfileDefault() const
            {
                return NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain422_12;
            }
        }
    }
}