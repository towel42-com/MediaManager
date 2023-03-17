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
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableAudioDecodersDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(8svx_exp - 8SVX exponential)" //
                            ,R"(8svx_fib - 8SVX fibonacci)" //
                            ,R"(aac - AAC (Advanced Audio Coding))" //
                            ,R"(aac_fixed - AAC (Advanced Audio Coding) (codec aac))" //
                            ,R"(aac_latm - AAC LATM (Advanced Audio Coding LATM syntax))" //
                            ,R"(ac3 - ATSC A/52A (AC-3))" //
                            ,R"(ac3_fixed - ATSC A/52A (AC-3) (codec ac3))" //
                            ,R"(ac4 - AC-4)" //
                            ,R"(acelp.kelvin - Sipro ACELP.KELVIN)" //
                            ,R"(adpcm_4xm - ADPCM 4X Movie)" //
                            ,R"(adpcm_adx - SEGA CRI ADX ADPCM)" //
                            ,R"(adpcm_afc - ADPCM Nintendo Gamecube AFC)" //
                            ,R"(adpcm_agm - ADPCM AmuseGraphics Movie)" //
                            ,R"(adpcm_aica - ADPCM Yamaha AICA)" //
                            ,R"(adpcm_argo - ADPCM Argonaut Games)" //
                            ,R"(adpcm_ct - ADPCM Creative Technology)" //
                            ,R"(adpcm_dtk - ADPCM Nintendo Gamecube DTK)" //
                            ,R"(adpcm_ea - ADPCM Electronic Arts)" //
                            ,R"(adpcm_ea_maxis_xa - ADPCM Electronic Arts Maxis CDROM XA)" //
                            ,R"(adpcm_ea_r1 - ADPCM Electronic Arts R1)" //
                            ,R"(adpcm_ea_r2 - ADPCM Electronic Arts R2)" //
                            ,R"(adpcm_ea_r3 - ADPCM Electronic Arts R3)" //
                            ,R"(adpcm_ea_xas - ADPCM Electronic Arts XAS)" //
                            ,R"(g722 - G.722 ADPCM (codec adpcm_g722))" //
                            ,R"(g726 - G.726 ADPCM (codec adpcm_g726))" //
                            ,R"(g726le - G.726 ADPCM little-endian (codec adpcm_g726le))" //
                            ,R"(adpcm_ima_acorn - ADPCM IMA Acorn Replay)" //
                            ,R"(adpcm_ima_alp - ADPCM IMA High Voltage Software ALP)" //
                            ,R"(adpcm_ima_amv - ADPCM IMA AMV)" //
                            ,R"(adpcm_ima_apc - ADPCM IMA CRYO APC)" //
                            ,R"(adpcm_ima_apm - ADPCM IMA Ubisoft APM)" //
                            ,R"(adpcm_ima_cunning - ADPCM IMA Cunning Developments)" //
                            ,R"(adpcm_ima_dat4 - ADPCM IMA Eurocom DAT4)" //
                            ,R"(adpcm_ima_dk3 - ADPCM IMA Duck DK3)" //
                            ,R"(adpcm_ima_dk4 - ADPCM IMA Duck DK4)" //
                            ,R"(adpcm_ima_ea_eacs - ADPCM IMA Electronic Arts EACS)" //
                            ,R"(adpcm_ima_ea_sead - ADPCM IMA Electronic Arts SEAD)" //
                            ,R"(adpcm_ima_iss - ADPCM IMA Funcom ISS)" //
                            ,R"(adpcm_ima_moflex - ADPCM IMA MobiClip MOFLEX)" //
                            ,R"(adpcm_ima_mtf - ADPCM IMA Capcom's MT Framework)" //
                            ,R"(adpcm_ima_oki - ADPCM IMA Dialogic OKI)" //
                            ,R"(adpcm_ima_qt - ADPCM IMA QuickTime)" //
                            ,R"(adpcm_ima_rad - ADPCM IMA Radical)" //
                            ,R"(adpcm_ima_smjpeg - ADPCM IMA Loki SDL MJPEG)" //
                            ,R"(adpcm_ima_ssi - ADPCM IMA Simon & Schuster Interactive)" //
                            ,R"(adpcm_ima_wav - ADPCM IMA WAV)" //
                            ,R"(adpcm_ima_ws - ADPCM IMA Westwood)" //
                            ,R"(adpcm_ms - ADPCM Microsoft)" //
                            ,R"(adpcm_mtaf - ADPCM MTAF)" //
                            ,R"(adpcm_psx - ADPCM Playstation)" //
                            ,R"(adpcm_sbpro_2 - ADPCM Sound Blaster Pro 2-bit)" //
                            ,R"(adpcm_sbpro_3 - ADPCM Sound Blaster Pro 2.6-bit)" //
                            ,R"(adpcm_sbpro_4 - ADPCM Sound Blaster Pro 4-bit)" //
                            ,R"(adpcm_swf - ADPCM Shockwave Flash)" //
                            ,R"(adpcm_thp - ADPCM Nintendo THP)" //
                            ,R"(adpcm_thp_le - ADPCM Nintendo THP (little-endian))" //
                            ,R"(adpcm_vima - LucasArts VIMA audio)" //
                            ,R"(adpcm_xa - ADPCM CDROM XA)" //
                            ,R"(adpcm_yamaha - ADPCM Yamaha)" //
                            ,R"(adpcm_zork - ADPCM Zork)" //
                            ,R"(alac - ALAC (Apple Lossless Audio Codec))" //
                            ,R"(amrnb - AMR-NB (Adaptive Multi-Rate NarrowBand) (codec amr_nb))" //
                            ,R"(libopencore_amrnb - OpenCORE AMR-NB (Adaptive Multi-Rate Narrow-Band) (codec amr_nb))" //
                            ,R"(amrwb - AMR-WB (Adaptive Multi-Rate WideBand) (codec amr_wb))" //
                            ,R"(libopencore_amrwb - OpenCORE AMR-WB (Adaptive Multi-Rate Wide-Band) (codec amr_wb))" //
                            ,R"(ape - Monkey's Audio)" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth))" //
                            ,R"(atrac1 - ATRAC1 (Adaptive TRansform Acoustic Coding))" //
                            ,R"(atrac3 - ATRAC3 (Adaptive TRansform Acoustic Coding 3))" //
                            ,R"(atrac3al - ATRAC3 AL (Adaptive TRansform Acoustic Coding 3 Advanced Lossless))" //
                            ,R"(atrac3plus - ATRAC3+ (Adaptive TRansform Acoustic Coding 3+) (codec atrac3p))" //
                            ,R"(atrac3plusal - ATRAC3+ AL (Adaptive TRansform Acoustic Coding 3+ Advanced Lossless) (codec atrac3pal))" //
                            ,R"(atrac9 - ATRAC9 (Adaptive TRansform Acoustic Coding 9))" //
                            ,R"(on2avc - On2 Audio for Video Codec (codec avc))" //
                            ,R"(binkaudio_dct - Bink Audio (DCT))" //
                            ,R"(binkaudio_rdft - Bink Audio (RDFT))" //
                            ,R"(bmv_audio - Discworld II BMV audio)" //
                            ,R"(comfortnoise - RFC 3389 comfort noise generator)" //
                            ,R"(cook - Cook / Cooker / Gecko (RealAudio G2))" //
                            ,R"(derf_dpcm - DPCM Xilam DERF)" //
                            ,R"(dolby_e - Dolby E)" //
                            ,R"(dsd_lsbf - DSD (Direct Stream Digital), least significant bit first)" //
                            ,R"(dsd_lsbf_planar - DSD (Direct Stream Digital), least significant bit first, planar)" //
                            ,R"(dsd_msbf - DSD (Direct Stream Digital), most significant bit first)" //
                            ,R"(dsd_msbf_planar - DSD (Direct Stream Digital), most significant bit first, planar)" //
                            ,R"(dsicinaudio - Delphine Software International CIN audio)" //
                            ,R"(dss_sp - Digital Speech Standard - Standard Play mode (DSS SP))" //
                            ,R"(dst - DST (Digital Stream Transfer))" //
                            ,R"(dca - DCA (DTS Coherent Acoustics) (codec dts))" //
                            ,R"(dvaudio - Ulead DV Audio)" //
                            ,R"(eac3 - ATSC A/52B (AC-3, E-AC-3))" //
                            ,R"(evrc - EVRC (Enhanced Variable Rate Codec))" //
                            ,R"(fastaudio - MobiClip FastAudio)" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec))" //
                            ,R"(g723_1 - G.723.1)" //
                            ,R"(g729 - G.729)" //
                            ,R"(gremlin_dpcm - DPCM Gremlin)" //
                            ,R"(gsm - GSM)" //
                            ,R"(gsm_ms - GSM Microsoft variant)" //
                            ,R"(hca - CRI HCA)" //
                            ,R"(hcom - HCOM Audio)" //
                            ,R"(iac - IAC (Indeo Audio Coder))" //
                            ,R"(ilbc - iLBC (Internet Low Bitrate Codec))" //
                            ,R"(imc - IMC (Intel Music Coder))" //
                            ,R"(interplay_dpcm - DPCM Interplay)" //
                            ,R"(interplayacm - Interplay ACM)" //
                            ,R"(mace3 - MACE (Macintosh Audio Compression/Expansion) 3:1)" //
                            ,R"(mace6 - MACE (Macintosh Audio Compression/Expansion) 6:1)" //
                            ,R"(metasound - Voxware MetaSound)" //
                            ,R"(mlp - MLP (Meridian Lossless Packing))" //
                            ,R"(mp1 - MP1 (MPEG audio layer 1))" //
                            ,R"(mp1float - MP1 (MPEG audio layer 1) (codec mp1))" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2))" //
                            ,R"(mp2float - MP2 (MPEG audio layer 2) (codec mp2))" //
                            ,R"(mp3float - MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(mp3 - MP3 (MPEG audio layer 3))" //
                            ,R"(mp3adufloat - ADU (Application Data Unit) MP3 (MPEG audio layer 3) (codec mp3adu))" //
                            ,R"(mp3adu - ADU (Application Data Unit) MP3 (MPEG audio layer 3))" //
                            ,R"(mp3on4float - MP3onMP4 (codec mp3on4))" //
                            ,R"(mp3on4 - MP3onMP4)" //
                            ,R"(als - MPEG-4 Audio Lossless Coding (ALS) (codec mp4als))" //
                            ,R"(msnsiren - MSN Siren)" //
                            ,R"(mpc7 - Musepack SV7 (codec musepack7))" //
                            ,R"(mpc8 - Musepack SV8 (codec musepack8))" //
                            ,R"(nellymoser - Nellymoser Asao)" //
                            ,R"(opus - Opus)" //
                            ,R"(libopus - libopus Opus (codec opus))" //
                            ,R"(paf_audio - Amazing Studio Packed Animation File Audio)" //
                            ,R"(pcm_alaw - PCM A-law / G.711 A-law)" //
                            ,R"(pcm_bluray - PCM signed 16|20|24-bit big-endian for Blu-ray media)" //
                            ,R"(pcm_dvd - PCM signed 16|20|24-bit big-endian for DVD media)" //
                            ,R"(pcm_f16le - PCM 16.8 floating point little-endian)" //
                            ,R"(pcm_f24le - PCM 24.0 floating point little-endian)" //
                            ,R"(pcm_f32be - PCM 32-bit floating point big-endian)" //
                            ,R"(pcm_f32le - PCM 32-bit floating point little-endian)" //
                            ,R"(pcm_f64be - PCM 64-bit floating point big-endian)" //
                            ,R"(pcm_f64le - PCM 64-bit floating point little-endian)" //
                            ,R"(pcm_lxf - PCM signed 20-bit little-endian planar)" //
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
                            ,R"(pcm_sga - PCM SGA)" //
                            ,R"(pcm_u16be - PCM unsigned 16-bit big-endian)" //
                            ,R"(pcm_u16le - PCM unsigned 16-bit little-endian)" //
                            ,R"(pcm_u24be - PCM unsigned 24-bit big-endian)" //
                            ,R"(pcm_u24le - PCM unsigned 24-bit little-endian)" //
                            ,R"(pcm_u32be - PCM unsigned 32-bit big-endian)" //
                            ,R"(pcm_u32le - PCM unsigned 32-bit little-endian)" //
                            ,R"(pcm_u8 - PCM unsigned 8-bit)" //
                            ,R"(pcm_vidc - PCM Archimedes VIDC)" //
                            ,R"(qcelp - QCELP / PureVoice)" //
                            ,R"(qdm2 - QDesign Music Codec 2)" //
                            ,R"(qdmc - QDesign Music Codec 1)" //
                            ,R"(real_144 - RealAudio 1.0 (14.4K) (codec ra_144))" //
                            ,R"(real_288 - RealAudio 2.0 (28.8K) (codec ra_288))" //
                            ,R"(ralf - RealAudio Lossless)" //
                            ,R"(roq_dpcm - DPCM id RoQ)" //
                            ,R"(s302m - SMPTE 302M)" //
                            ,R"(sbc - SBC (low-complexity subband codec))" //
                            ,R"(sdx2_dpcm - DPCM Squareroot-Delta-Exact)" //
                            ,R"(shorten - Shorten)" //
                            ,R"(sipr - RealAudio SIPR / ACELP.NET)" //
                            ,R"(siren - Siren)" //
                            ,R"(smackaud - Smacker audio (codec smackaudio))" //
                            ,R"(sol_dpcm - DPCM Sol)" //
                            ,R"(speex - Speex)" //
                            ,R"(libspeex - libspeex Speex (codec speex))" //
                            ,R"(tak - TAK (Tom's lossless Audio Kompressor))" //
                            ,R"(truehd - TrueHD)" //
                            ,R"(truespeech - DSP Group TrueSpeech)" //
                            ,R"(tta - TTA (True Audio))" //
                            ,R"(twinvq - VQF TwinVQ)" //
                            ,R"(vmdaudio - Sierra VMD audio)" //
                            ,R"(vorbis - Vorbis)" //
                            ,R"(libvorbis - libvorbis (codec vorbis))" //
                            ,R"(wavesynth - Wave synthesis pseudo-codec)" //
                            ,R"(wavpack - WavPack)" //
                            ,R"(ws_snd1 - Westwood Audio (SND1) (codec westwood_snd1))" //
                            ,R"(wmalossless - Windows Media Audio Lossless)" //
                            ,R"(wmapro - Windows Media Audio 9 Professional)" //
                            ,R"(wmav1 - Windows Media Audio 1)" //
                            ,R"(wmav2 - Windows Media Audio 2)" //
                            ,R"(wmavoice - Windows Media Audio Voice)" //
                            ,R"(xan_dpcm - DPCM Xan)" //
                            ,R"(xma1 - Xbox Media Audio 1)" //
                            ,R"(xma2 - Xbox Media Audio 2)" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(8svx_exp)" //
                            ,R"(8svx_fib)" //
                            ,R"(aac)" //
                            ,R"(aac_fixed)" //
                            ,R"(aac_latm)" //
                            ,R"(ac3)" //
                            ,R"(ac3_fixed)" //
                            ,R"(ac4)" //
                            ,R"(acelp.kelvin)" //
                            ,R"(adpcm_4xm)" //
                            ,R"(adpcm_adx)" //
                            ,R"(adpcm_afc)" //
                            ,R"(adpcm_agm)" //
                            ,R"(adpcm_aica)" //
                            ,R"(adpcm_argo)" //
                            ,R"(adpcm_ct)" //
                            ,R"(adpcm_dtk)" //
                            ,R"(adpcm_ea)" //
                            ,R"(adpcm_ea_maxis_xa)" //
                            ,R"(adpcm_ea_r1)" //
                            ,R"(adpcm_ea_r2)" //
                            ,R"(adpcm_ea_r3)" //
                            ,R"(adpcm_ea_xas)" //
                            ,R"(g722)" //
                            ,R"(g726)" //
                            ,R"(g726le)" //
                            ,R"(adpcm_ima_acorn)" //
                            ,R"(adpcm_ima_alp)" //
                            ,R"(adpcm_ima_amv)" //
                            ,R"(adpcm_ima_apc)" //
                            ,R"(adpcm_ima_apm)" //
                            ,R"(adpcm_ima_cunning)" //
                            ,R"(adpcm_ima_dat4)" //
                            ,R"(adpcm_ima_dk3)" //
                            ,R"(adpcm_ima_dk4)" //
                            ,R"(adpcm_ima_ea_eacs)" //
                            ,R"(adpcm_ima_ea_sead)" //
                            ,R"(adpcm_ima_iss)" //
                            ,R"(adpcm_ima_moflex)" //
                            ,R"(adpcm_ima_mtf)" //
                            ,R"(adpcm_ima_oki)" //
                            ,R"(adpcm_ima_qt)" //
                            ,R"(adpcm_ima_rad)" //
                            ,R"(adpcm_ima_smjpeg)" //
                            ,R"(adpcm_ima_ssi)" //
                            ,R"(adpcm_ima_wav)" //
                            ,R"(adpcm_ima_ws)" //
                            ,R"(adpcm_ms)" //
                            ,R"(adpcm_mtaf)" //
                            ,R"(adpcm_psx)" //
                            ,R"(adpcm_sbpro_2)" //
                            ,R"(adpcm_sbpro_3)" //
                            ,R"(adpcm_sbpro_4)" //
                            ,R"(adpcm_swf)" //
                            ,R"(adpcm_thp)" //
                            ,R"(adpcm_thp_le)" //
                            ,R"(adpcm_vima)" //
                            ,R"(adpcm_xa)" //
                            ,R"(adpcm_yamaha)" //
                            ,R"(adpcm_zork)" //
                            ,R"(alac)" //
                            ,R"(amrnb)" //
                            ,R"(libopencore_amrnb)" //
                            ,R"(amrwb)" //
                            ,R"(libopencore_amrwb)" //
                            ,R"(ape)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(atrac1)" //
                            ,R"(atrac3)" //
                            ,R"(atrac3al)" //
                            ,R"(atrac3plus)" //
                            ,R"(atrac3plusal)" //
                            ,R"(atrac9)" //
                            ,R"(on2avc)" //
                            ,R"(binkaudio_dct)" //
                            ,R"(binkaudio_rdft)" //
                            ,R"(bmv_audio)" //
                            ,R"(comfortnoise)" //
                            ,R"(cook)" //
                            ,R"(derf_dpcm)" //
                            ,R"(dolby_e)" //
                            ,R"(dsd_lsbf)" //
                            ,R"(dsd_lsbf_planar)" //
                            ,R"(dsd_msbf)" //
                            ,R"(dsd_msbf_planar)" //
                            ,R"(dsicinaudio)" //
                            ,R"(dss_sp)" //
                            ,R"(dst)" //
                            ,R"(dca)" //
                            ,R"(dvaudio)" //
                            ,R"(eac3)" //
                            ,R"(evrc)" //
                            ,R"(fastaudio)" //
                            ,R"(flac)" //
                            ,R"(g723_1)" //
                            ,R"(g729)" //
                            ,R"(gremlin_dpcm)" //
                            ,R"(gsm)" //
                            ,R"(gsm_ms)" //
                            ,R"(hca)" //
                            ,R"(hcom)" //
                            ,R"(iac)" //
                            ,R"(ilbc)" //
                            ,R"(imc)" //
                            ,R"(interplay_dpcm)" //
                            ,R"(interplayacm)" //
                            ,R"(mace3)" //
                            ,R"(mace6)" //
                            ,R"(metasound)" //
                            ,R"(mlp)" //
                            ,R"(mp1)" //
                            ,R"(mp1float)" //
                            ,R"(mp2)" //
                            ,R"(mp2float)" //
                            ,R"(mp3float)" //
                            ,R"(mp3)" //
                            ,R"(mp3adufloat)" //
                            ,R"(mp3adu)" //
                            ,R"(mp3on4float)" //
                            ,R"(mp3on4)" //
                            ,R"(als)" //
                            ,R"(msnsiren)" //
                            ,R"(mpc7)" //
                            ,R"(mpc8)" //
                            ,R"(nellymoser)" //
                            ,R"(opus)" //
                            ,R"(libopus)" //
                            ,R"(paf_audio)" //
                            ,R"(pcm_alaw)" //
                            ,R"(pcm_bluray)" //
                            ,R"(pcm_dvd)" //
                            ,R"(pcm_f16le)" //
                            ,R"(pcm_f24le)" //
                            ,R"(pcm_f32be)" //
                            ,R"(pcm_f32le)" //
                            ,R"(pcm_f64be)" //
                            ,R"(pcm_f64le)" //
                            ,R"(pcm_lxf)" //
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
                            ,R"(pcm_sga)" //
                            ,R"(pcm_u16be)" //
                            ,R"(pcm_u16le)" //
                            ,R"(pcm_u24be)" //
                            ,R"(pcm_u24le)" //
                            ,R"(pcm_u32be)" //
                            ,R"(pcm_u32le)" //
                            ,R"(pcm_u8)" //
                            ,R"(pcm_vidc)" //
                            ,R"(qcelp)" //
                            ,R"(qdm2)" //
                            ,R"(qdmc)" //
                            ,R"(real_144)" //
                            ,R"(real_288)" //
                            ,R"(ralf)" //
                            ,R"(roq_dpcm)" //
                            ,R"(s302m)" //
                            ,R"(sbc)" //
                            ,R"(sdx2_dpcm)" //
                            ,R"(shorten)" //
                            ,R"(sipr)" //
                            ,R"(siren)" //
                            ,R"(smackaud)" //
                            ,R"(sol_dpcm)" //
                            ,R"(speex)" //
                            ,R"(libspeex)" //
                            ,R"(tak)" //
                            ,R"(truehd)" //
                            ,R"(truespeech)" //
                            ,R"(tta)" //
                            ,R"(twinvq)" //
                            ,R"(vmdaudio)" //
                            ,R"(vorbis)" //
                            ,R"(libvorbis)" //
                            ,R"(wavesynth)" //
                            ,R"(wavpack)" //
                            ,R"(ws_snd1)" //
                            ,R"(wmalossless)" //
                            ,R"(wmapro)" //
                            ,R"(wmav1)" //
                            ,R"(wmav2)" //
                            ,R"(wmavoice)" //
                            ,R"(xan_dpcm)" //
                            ,R"(xma1)" //
                            ,R"(xma2)" //
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
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableVideoDecodersDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(012v - Uncompressed 4:2:2 10-bit)" //
                            ,R"(4xm - 4X Movie)" //
                            ,R"(8bps - QuickTime 8BPS video)" //
                            ,R"(aasc - Autodesk RLE)" //
                            ,R"(agm - Amuse Graphics Movie)" //
                            ,R"(aic - Apple Intermediate Codec)" //
                            ,R"(alias_pix - Alias/Wavefront PIX image)" //
                            ,R"(amv - AMV Video)" //
                            ,R"(anm - Deluxe Paint Animation)" //
                            ,R"(ansi - ASCII/ANSI art)" //
                            ,R"(apng - APNG (Animated Portable Network Graphics) image)" //
                            ,R"(arbc - Gryphon's Anim Compressor)" //
                            ,R"(argo - Argonaut Games Video)" //
                            ,R"(asv1 - ASUS V1)" //
                            ,R"(asv2 - ASUS V2)" //
                            ,R"(aura - Auravision AURA)" //
                            ,R"(aura2 - Auravision Aura 2)" //
                            ,R"(libdav1d - dav1d AV1 decoder by VideoLAN (codec av1))" //
                            ,R"(libaom-av1 - libaom AV1 (codec av1))" //
                            ,R"(av1 - Alliance for Open Media AV1)" //
                            ,R"(av1_qsv - AV1 video (Intel Quick Sync Video acceleration) (codec av1))" //
                            ,R"(avrn - Avid AVI Codec)" //
                            ,R"(avrp - Avid 1:1 10-bit RGB Packer)" //
                            ,R"(avs - AVS (Audio Video Standard) video)" //
                            ,R"(avui - Avid Meridien Uncompressed)" //
                            ,R"(ayuv - Uncompressed packed MS 4:4:4:4)" //
                            ,R"(bethsoftvid - Bethesda VID video)" //
                            ,R"(bfi - Brute Force & Ignorance)" //
                            ,R"(binkvideo - Bink video)" //
                            ,R"(bintext - Binary text)" //
                            ,R"(bitpacked - Bitpacked)" //
                            ,R"(bmp - BMP (Windows and OS/2 bitmap))" //
                            ,R"(bmv_video - Discworld II BMV video)" //
                            ,R"(brender_pix - BRender PIX image)" //
                            ,R"(c93 - Interplay C93)" //
                            ,R"(cavs - Chinese AVS (Audio Video Standard) (AVS1-P2, JiZhun profile))" //
                            ,R"(cdgraphics - CD Graphics video)" //
                            ,R"(cdtoons - CDToons video)" //
                            ,R"(cdxl - Commodore CDXL video)" //
                            ,R"(cfhd - GoPro CineForm HD)" //
                            ,R"(cinepak - Cinepak)" //
                            ,R"(clearvideo - Iterated Systems ClearVideo)" //
                            ,R"(cljr - Cirrus Logic AccuPak)" //
                            ,R"(cllc - Canopus Lossless Codec)" //
                            ,R"(eacmv - Electronic Arts CMV video (codec cmv))" //
                            ,R"(cpia - CPiA video format)" //
                            ,R"(cri - Cintel RAW)" //
                            ,R"(camstudio - CamStudio (codec cscd))" //
                            ,R"(cyuv - Creative YUV (CYUV))" //
                            ,R"(dds - DirectDraw Surface image decoder)" //
                            ,R"(dfa - Chronomaster DFA)" //
                            ,R"(dirac - BBC Dirac VC-2)" //
                            ,R"(dnxhd - VC3/DNxHD)" //
                            ,R"(dpx - DPX (Digital Picture Exchange) image)" //
                            ,R"(dsicinvideo - Delphine Software International CIN video)" //
                            ,R"(dvvideo - DV (Digital Video))" //
                            ,R"(dxa - Feeble Files/ScummVM DXA)" //
                            ,R"(dxtory - Dxtory)" //
                            ,R"(dxv - Resolume DXV)" //
                            ,R"(escape124 - Escape 124)" //
                            ,R"(escape130 - Escape 130)" //
                            ,R"(exr - OpenEXR image)" //
                            ,R"(ffv1 - FFmpeg video codec #1)" //
                            ,R"(ffvhuff - Huffyuv FFmpeg variant)" //
                            ,R"(fic - Mirillis FIC)" //
                            ,R"(fits - Flexible Image Transport System)" //
                            ,R"(flashsv - Flash Screen Video v1)" //
                            ,R"(flashsv2 - Flash Screen Video v2)" //
                            ,R"(flic - Autodesk Animator Flic video)" //
                            ,R"(flv - FLV / Sorenson Spark / Sorenson H.263 (Flash Video) (codec flv1))" //
                            ,R"(fmvc - FM Screen Capture Codec)" //
                            ,R"(fraps - Fraps)" //
                            ,R"(frwu - Forward Uncompressed)" //
                            ,R"(g2m - Go2Meeting)" //
                            ,R"(gdv - Gremlin Digital Video)" //
                            ,R"(gem - GEM Raster image)" //
                            ,R"(gif - GIF (Graphics Interchange Format))" //
                            ,R"(h261 - H.261)" //
                            ,R"(h263 - H.263 / H.263-1996, H.263+ / H.263-1998 / H.263 version 2)" //
                            ,R"(h263i - Intel H.263)" //
                            ,R"(h263p - H.263 / H.263-1996, H.263+ / H.263-1998 / H.263 version 2)" //
                            ,R"(h264 - H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10)" //
                            ,R"(h264_qsv - H264 video (Intel Quick Sync Video acceleration) (codec h264))" //
                            ,R"(h264_cuvid - Nvidia CUVID H264 decoder (codec h264))" //
                            ,R"(hap - Vidvox Hap)" //
                            ,R"(hevc - HEVC (High Efficiency Video Coding))" //
                            ,R"(hevc_qsv - HEVC video (Intel Quick Sync Video acceleration) (codec hevc))" //
                            ,R"(hevc_cuvid - Nvidia CUVID HEVC decoder (codec hevc))" //
                            ,R"(hnm4video - HNM 4 video)" //
                            ,R"(hq_hqa - Canopus HQ/HQA)" //
                            ,R"(hqx - Canopus HQX)" //
                            ,R"(huffyuv - Huffyuv / HuffYUV)" //
                            ,R"(hymt - HuffYUV MT)" //
                            ,R"(idcinvideo - id Quake II CIN video (codec idcin))" //
                            ,R"(idf - iCEDraw text)" //
                            ,R"(iff - IFF ACBM/ANIM/DEEP/ILBM/PBM/RGB8/RGBN (codec iff_ilbm))" //
                            ,R"(imm4 - Infinity IMM4)" //
                            ,R"(imm5 - Infinity IMM5)" //
                            ,R"(indeo2 - Intel Indeo 2)" //
                            ,R"(indeo3 - Intel Indeo 3)" //
                            ,R"(indeo4 - Intel Indeo Video Interactive 4)" //
                            ,R"(indeo5 - Intel Indeo Video Interactive 5)" //
                            ,R"(interplayvideo - Interplay MVE video)" //
                            ,R"(ipu - IPU Video)" //
                            ,R"(jpeg2000 - JPEG 2000)" //
                            ,R"(libopenjpeg - OpenJPEG JPEG 2000 (codec jpeg2000))" //
                            ,R"(jpegls - JPEG-LS)" //
                            ,R"(jv - Bitmap Brothers JV video)" //
                            ,R"(kgv1 - Kega Game Video)" //
                            ,R"(kmvc - Karl Morton's video codec)" //
                            ,R"(lagarith - Lagarith lossless)" //
                            ,R"(loco - LOCO)" //
                            ,R"(lscr - LEAD Screen Capture)" //
                            ,R"(m101 - Matrox Uncompressed SD)" //
                            ,R"(eamad - Electronic Arts Madcow Video (codec mad))" //
                            ,R"(magicyuv - MagicYUV video)" //
                            ,R"(mdec - Sony PlayStation MDEC (Motion DECoder))" //
                            ,R"(mimic - Mimic)" //
                            ,R"(mjpeg - MJPEG (Motion JPEG))" //
                            ,R"(mjpeg_cuvid - Nvidia CUVID MJPEG decoder (codec mjpeg))" //
                            ,R"(mjpeg_qsv - MJPEG video (Intel Quick Sync Video acceleration) (codec mjpeg))" //
                            ,R"(mjpegb - Apple MJPEG-B)" //
                            ,R"(mmvideo - American Laser Games MM Video)" //
                            ,R"(mobiclip - MobiClip Video)" //
                            ,R"(motionpixels - Motion Pixels video)" //
                            ,R"(mpeg1video - MPEG-1 video)" //
                            ,R"(mpeg1_cuvid - Nvidia CUVID MPEG1VIDEO decoder (codec mpeg1video))" //
                            ,R"(mpeg2video - MPEG-2 video)" //
                            ,R"(mpegvideo - MPEG-1 video (codec mpeg2video))" //
                            ,R"(mpeg2_qsv - MPEG2VIDEO video (Intel Quick Sync Video acceleration) (codec mpeg2video))" //
                            ,R"(mpeg2_cuvid - Nvidia CUVID MPEG2VIDEO decoder (codec mpeg2video))" //
                            ,R"(mpeg4 - MPEG-4 part 2)" //
                            ,R"(mpeg4_cuvid - Nvidia CUVID MPEG4 decoder (codec mpeg4))" //
                            ,R"(msa1 - MS ATC Screen)" //
                            ,R"(mscc - Mandsoft Screen Capture Codec)" //
                            ,R"(msmpeg4v1 - MPEG-4 part 2 Microsoft variant version 1)" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2)" //
                            ,R"(msmpeg4 - MPEG-4 part 2 Microsoft variant version 3 (codec msmpeg4v3))" //
                            ,R"(msp2 - Microsoft Paint (MSP) version 2)" //
                            ,R"(msrle - Microsoft RLE)" //
                            ,R"(mss1 - MS Screen 1)" //
                            ,R"(mss2 - MS Windows Media Video V9 Screen)" //
                            ,R"(msvideo1 - Microsoft Video 1)" //
                            ,R"(mszh - LCL (LossLess Codec Library) MSZH)" //
                            ,R"(mts2 - MS Expression Encoder Screen)" //
                            ,R"(mv30 - MidiVid 3.0)" //
                            ,R"(mvc1 - Silicon Graphics Motion Video Compressor 1)" //
                            ,R"(mvc2 - Silicon Graphics Motion Video Compressor 2)" //
                            ,R"(mvdv - MidiVid VQ)" //
                            ,R"(mvha - MidiVid Archive Codec)" //
                            ,R"(mwsc - MatchWare Screen Capture Codec)" //
                            ,R"(mxpeg - Mobotix MxPEG video)" //
                            ,R"(notchlc - NotchLC)" //
                            ,R"(nuv - NuppelVideo/RTJPEG)" //
                            ,R"(paf_video - Amazing Studio Packed Animation File Video)" //
                            ,R"(pam - PAM (Portable AnyMap) image)" //
                            ,R"(pbm - PBM (Portable BitMap) image)" //
                            ,R"(pcx - PC Paintbrush PCX image)" //
                            ,R"(pfm - PFM (Portable FloatMap) image)" //
                            ,R"(pgm - PGM (Portable GrayMap) image)" //
                            ,R"(pgmyuv - PGMYUV (Portable GrayMap YUV) image)" //
                            ,R"(pgx - PGX (JPEG2000 Test Format))" //
                            ,R"(photocd - Kodak Photo CD)" //
                            ,R"(pictor - Pictor/PC Paint)" //
                            ,R"(pixlet - Apple Pixlet)" //
                            ,R"(png - PNG (Portable Network Graphics) image)" //
                            ,R"(ppm - PPM (Portable PixelMap) image)" //
                            ,R"(prores - Apple ProRes (iCodec Pro))" //
                            ,R"(prosumer - Brooktree ProSumer Video)" //
                            ,R"(psd - Photoshop PSD file)" //
                            ,R"(ptx - V.Flash PTX image)" //
                            ,R"(qdraw - Apple QuickDraw)" //
                            ,R"(qpeg - Q-team QPEG)" //
                            ,R"(qtrle - QuickTime Animation (RLE) video)" //
                            ,R"(r10k - AJA Kona 10-bit RGB Codec)" //
                            ,R"(r210 - Uncompressed RGB 10-bit)" //
                            ,R"(rasc - RemotelyAnywhere Screen Capture)" //
                            ,R"(rawvideo - raw video)" //
                            ,R"(rl2 - RL2 video)" //
                            ,R"(roqvideo - id RoQ video (codec roq))" //
                            ,R"(rpza - QuickTime video (RPZA))" //
                            ,R"(rscc - innoHeim/Rsupport Screen Capture Codec)" //
                            ,R"(rv10 - RealVideo 1.0)" //
                            ,R"(rv20 - RealVideo 2.0)" //
                            ,R"(rv30 - RealVideo 3.0)" //
                            ,R"(rv40 - RealVideo 4.0)" //
                            ,R"(sanm - LucasArts SANM/Smush video)" //
                            ,R"(scpr - ScreenPressor)" //
                            ,R"(screenpresso - Screenpresso)" //
                            ,R"(sga - Digital Pictures SGA Video)" //
                            ,R"(sgi - SGI image)" //
                            ,R"(sgirle - Silicon Graphics RLE 8-bit video)" //
                            ,R"(sheervideo - BitJazz SheerVideo)" //
                            ,R"(simbiosis_imx - Simbiosis Interactive IMX Video)" //
                            ,R"(smackvid - Smacker video (codec smackvideo))" //
                            ,R"(smc - QuickTime Graphics (SMC))" //
                            ,R"(smvjpeg - SMV JPEG)" //
                            ,R"(snow - Snow)" //
                            ,R"(sp5x - Sunplus JPEG (SP5X))" //
                            ,R"(speedhq - NewTek SpeedHQ)" //
                            ,R"(srgc - Screen Recorder Gold Codec)" //
                            ,R"(sunrast - Sun Rasterfile image)" //
                            ,R"(svq1 - Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1)" //
                            ,R"(svq3 - Sorenson Vector Quantizer 3 / Sorenson Video 3 / SVQ3)" //
                            ,R"(targa - Truevision Targa image)" //
                            ,R"(targa_y216 - Pinnacle TARGA CineWave YUV16)" //
                            ,R"(tdsc - TDSC)" //
                            ,R"(eatgq - Electronic Arts TGQ video (codec tgq))" //
                            ,R"(eatgv - Electronic Arts TGV video (codec tgv))" //
                            ,R"(theora - Theora)" //
                            ,R"(thp - Nintendo Gamecube THP video)" //
                            ,R"(tiertexseqvideo - Tiertex Limited SEQ video)" //
                            ,R"(tiff - TIFF image)" //
                            ,R"(tmv - 8088flex TMV)" //
                            ,R"(eatqi - Electronic Arts TQI Video (codec tqi))" //
                            ,R"(truemotion1 - Duck TrueMotion 1.0)" //
                            ,R"(truemotion2 - Duck TrueMotion 2.0)" //
                            ,R"(truemotion2rt - Duck TrueMotion 2.0 Real Time)" //
                            ,R"(camtasia - TechSmith Screen Capture Codec (codec tscc))" //
                            ,R"(tscc2 - TechSmith Screen Codec 2)" //
                            ,R"(txd - Renderware TXD (TeXture Dictionary) image)" //
                            ,R"(ultimotion - IBM UltiMotion (codec ulti))" //
                            ,R"(utvideo - Ut Video)" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit)" //
                            ,R"(v210x - Uncompressed 4:2:2 10-bit)" //
                            ,R"(v308 - Uncompressed packed 4:4:4)" //
                            ,R"(v408 - Uncompressed packed QT 4:4:4:4)" //
                            ,R"(v410 - Uncompressed 4:4:4 10-bit)" //
                            ,R"(vb - Beam Software VB)" //
                            ,R"(vble - VBLE Lossless Codec)" //
                            ,R"(vc1 - SMPTE VC-1)" //
                            ,R"(vc1_qsv - VC1 video (Intel Quick Sync Video acceleration) (codec vc1))" //
                            ,R"(vc1_cuvid - Nvidia CUVID VC1 decoder (codec vc1))" //
                            ,R"(vc1image - Windows Media Video 9 Image v2)" //
                            ,R"(vcr1 - ATI VCR1)" //
                            ,R"(xl - Miro VideoXL (codec vixl))" //
                            ,R"(vmdvideo - Sierra VMD video)" //
                            ,R"(vmnc - VMware Screen Codec / VMware Video)" //
                            ,R"(vp3 - On2 VP3)" //
                            ,R"(vp4 - On2 VP4)" //
                            ,R"(vp5 - On2 VP5)" //
                            ,R"(vp6 - On2 VP6)" //
                            ,R"(vp6a - On2 VP6 (Flash version, with alpha channel))" //
                            ,R"(vp6f - On2 VP6 (Flash version))" //
                            ,R"(vp7 - On2 VP7)" //
                            ,R"(vp8 - On2 VP8)" //
                            ,R"(libvpx - libvpx VP8 (codec vp8))" //
                            ,R"(vp8_cuvid - Nvidia CUVID VP8 decoder (codec vp8))" //
                            ,R"(vp8_qsv - VP8 video (Intel Quick Sync Video acceleration) (codec vp8))" //
                            ,R"(vp9 - Google VP9)" //
                            ,R"(libvpx-vp9 - libvpx VP9 (codec vp9))" //
                            ,R"(vp9_cuvid - Nvidia CUVID VP9 decoder (codec vp9))" //
                            ,R"(vp9_qsv - VP9 video (Intel Quick Sync Video acceleration) (codec vp9))" //
                            ,R"(wcmv - WinCAM Motion Video)" //
                            ,R"(webp - WebP image)" //
                            ,R"(wmv1 - Windows Media Video 7)" //
                            ,R"(wmv2 - Windows Media Video 8)" //
                            ,R"(wmv3 - Windows Media Video 9)" //
                            ,R"(wmv3image - Windows Media Video 9 Image)" //
                            ,R"(wnv1 - Winnov WNV1)" //
                            ,R"(wrapped_avframe - AVPacket to AVFrame passthrough)" //
                            ,R"(vqavideo - Westwood Studios VQA (Vector Quantized Animation) video (codec ws_vqa))" //
                            ,R"(xan_wc3 - Wing Commander III / Xan)" //
                            ,R"(xan_wc4 - Wing Commander IV / Xxan)" //
                            ,R"(xbin - eXtended BINary text)" //
                            ,R"(xbm - XBM (X BitMap) image)" //
                            ,R"(xface - X-face image)" //
                            ,R"(xpm - XPM (X PixMap) image)" //
                            ,R"(xwd - XWD (X Window Dump) image)" //
                            ,R"(y41p - Uncompressed YUV 4:1:1 12-bit)" //
                            ,R"(ylc - YUY2 Lossless Codec)" //
                            ,R"(yop - Psygnosis YOP Video)" //
                            ,R"(yuv4 - Uncompressed packed 4:2:0)" //
                            ,R"(zerocodec - ZeroCodec Lossless Video)" //
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
                             R"(012v)" //
                            ,R"(4xm)" //
                            ,R"(8bps)" //
                            ,R"(aasc)" //
                            ,R"(agm)" //
                            ,R"(aic)" //
                            ,R"(alias_pix)" //
                            ,R"(amv)" //
                            ,R"(anm)" //
                            ,R"(ansi)" //
                            ,R"(apng)" //
                            ,R"(arbc)" //
                            ,R"(argo)" //
                            ,R"(asv1)" //
                            ,R"(asv2)" //
                            ,R"(aura)" //
                            ,R"(aura2)" //
                            ,R"(libdav1d)" //
                            ,R"(libaom-av1)" //
                            ,R"(av1)" //
                            ,R"(av1_qsv)" //
                            ,R"(avrn)" //
                            ,R"(avrp)" //
                            ,R"(avs)" //
                            ,R"(avui)" //
                            ,R"(ayuv)" //
                            ,R"(bethsoftvid)" //
                            ,R"(bfi)" //
                            ,R"(binkvideo)" //
                            ,R"(bintext)" //
                            ,R"(bitpacked)" //
                            ,R"(bmp)" //
                            ,R"(bmv_video)" //
                            ,R"(brender_pix)" //
                            ,R"(c93)" //
                            ,R"(cavs)" //
                            ,R"(cdgraphics)" //
                            ,R"(cdtoons)" //
                            ,R"(cdxl)" //
                            ,R"(cfhd)" //
                            ,R"(cinepak)" //
                            ,R"(clearvideo)" //
                            ,R"(cljr)" //
                            ,R"(cllc)" //
                            ,R"(eacmv)" //
                            ,R"(cpia)" //
                            ,R"(cri)" //
                            ,R"(camstudio)" //
                            ,R"(cyuv)" //
                            ,R"(dds)" //
                            ,R"(dfa)" //
                            ,R"(dirac)" //
                            ,R"(dnxhd)" //
                            ,R"(dpx)" //
                            ,R"(dsicinvideo)" //
                            ,R"(dvvideo)" //
                            ,R"(dxa)" //
                            ,R"(dxtory)" //
                            ,R"(dxv)" //
                            ,R"(escape124)" //
                            ,R"(escape130)" //
                            ,R"(exr)" //
                            ,R"(ffv1)" //
                            ,R"(ffvhuff)" //
                            ,R"(fic)" //
                            ,R"(fits)" //
                            ,R"(flashsv)" //
                            ,R"(flashsv2)" //
                            ,R"(flic)" //
                            ,R"(flv)" //
                            ,R"(fmvc)" //
                            ,R"(fraps)" //
                            ,R"(frwu)" //
                            ,R"(g2m)" //
                            ,R"(gdv)" //
                            ,R"(gem)" //
                            ,R"(gif)" //
                            ,R"(h261)" //
                            ,R"(h263)" //
                            ,R"(h263i)" //
                            ,R"(h263p)" //
                            ,R"(h264)" //
                            ,R"(h264_qsv)" //
                            ,R"(h264_cuvid)" //
                            ,R"(hap)" //
                            ,R"(hevc)" //
                            ,R"(hevc_qsv)" //
                            ,R"(hevc_cuvid)" //
                            ,R"(hnm4video)" //
                            ,R"(hq_hqa)" //
                            ,R"(hqx)" //
                            ,R"(huffyuv)" //
                            ,R"(hymt)" //
                            ,R"(idcinvideo)" //
                            ,R"(idf)" //
                            ,R"(iff)" //
                            ,R"(imm4)" //
                            ,R"(imm5)" //
                            ,R"(indeo2)" //
                            ,R"(indeo3)" //
                            ,R"(indeo4)" //
                            ,R"(indeo5)" //
                            ,R"(interplayvideo)" //
                            ,R"(ipu)" //
                            ,R"(jpeg2000)" //
                            ,R"(libopenjpeg)" //
                            ,R"(jpegls)" //
                            ,R"(jv)" //
                            ,R"(kgv1)" //
                            ,R"(kmvc)" //
                            ,R"(lagarith)" //
                            ,R"(loco)" //
                            ,R"(lscr)" //
                            ,R"(m101)" //
                            ,R"(eamad)" //
                            ,R"(magicyuv)" //
                            ,R"(mdec)" //
                            ,R"(mimic)" //
                            ,R"(mjpeg)" //
                            ,R"(mjpeg_cuvid)" //
                            ,R"(mjpeg_qsv)" //
                            ,R"(mjpegb)" //
                            ,R"(mmvideo)" //
                            ,R"(mobiclip)" //
                            ,R"(motionpixels)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg1_cuvid)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpegvideo)" //
                            ,R"(mpeg2_qsv)" //
                            ,R"(mpeg2_cuvid)" //
                            ,R"(mpeg4)" //
                            ,R"(mpeg4_cuvid)" //
                            ,R"(msa1)" //
                            ,R"(mscc)" //
                            ,R"(msmpeg4v1)" //
                            ,R"(msmpeg4v2)" //
                            ,R"(msmpeg4)" //
                            ,R"(msp2)" //
                            ,R"(msrle)" //
                            ,R"(mss1)" //
                            ,R"(mss2)" //
                            ,R"(msvideo1)" //
                            ,R"(mszh)" //
                            ,R"(mts2)" //
                            ,R"(mv30)" //
                            ,R"(mvc1)" //
                            ,R"(mvc2)" //
                            ,R"(mvdv)" //
                            ,R"(mvha)" //
                            ,R"(mwsc)" //
                            ,R"(mxpeg)" //
                            ,R"(notchlc)" //
                            ,R"(nuv)" //
                            ,R"(paf_video)" //
                            ,R"(pam)" //
                            ,R"(pbm)" //
                            ,R"(pcx)" //
                            ,R"(pfm)" //
                            ,R"(pgm)" //
                            ,R"(pgmyuv)" //
                            ,R"(pgx)" //
                            ,R"(photocd)" //
                            ,R"(pictor)" //
                            ,R"(pixlet)" //
                            ,R"(png)" //
                            ,R"(ppm)" //
                            ,R"(prores)" //
                            ,R"(prosumer)" //
                            ,R"(psd)" //
                            ,R"(ptx)" //
                            ,R"(qdraw)" //
                            ,R"(qpeg)" //
                            ,R"(qtrle)" //
                            ,R"(r10k)" //
                            ,R"(r210)" //
                            ,R"(rasc)" //
                            ,R"(rawvideo)" //
                            ,R"(rl2)" //
                            ,R"(roqvideo)" //
                            ,R"(rpza)" //
                            ,R"(rscc)" //
                            ,R"(rv10)" //
                            ,R"(rv20)" //
                            ,R"(rv30)" //
                            ,R"(rv40)" //
                            ,R"(sanm)" //
                            ,R"(scpr)" //
                            ,R"(screenpresso)" //
                            ,R"(sga)" //
                            ,R"(sgi)" //
                            ,R"(sgirle)" //
                            ,R"(sheervideo)" //
                            ,R"(simbiosis_imx)" //
                            ,R"(smackvid)" //
                            ,R"(smc)" //
                            ,R"(smvjpeg)" //
                            ,R"(snow)" //
                            ,R"(sp5x)" //
                            ,R"(speedhq)" //
                            ,R"(srgc)" //
                            ,R"(sunrast)" //
                            ,R"(svq1)" //
                            ,R"(svq3)" //
                            ,R"(targa)" //
                            ,R"(targa_y216)" //
                            ,R"(tdsc)" //
                            ,R"(eatgq)" //
                            ,R"(eatgv)" //
                            ,R"(theora)" //
                            ,R"(thp)" //
                            ,R"(tiertexseqvideo)" //
                            ,R"(tiff)" //
                            ,R"(tmv)" //
                            ,R"(eatqi)" //
                            ,R"(truemotion1)" //
                            ,R"(truemotion2)" //
                            ,R"(truemotion2rt)" //
                            ,R"(camtasia)" //
                            ,R"(tscc2)" //
                            ,R"(txd)" //
                            ,R"(ultimotion)" //
                            ,R"(utvideo)" //
                            ,R"(v210)" //
                            ,R"(v210x)" //
                            ,R"(v308)" //
                            ,R"(v408)" //
                            ,R"(v410)" //
                            ,R"(vb)" //
                            ,R"(vble)" //
                            ,R"(vc1)" //
                            ,R"(vc1_qsv)" //
                            ,R"(vc1_cuvid)" //
                            ,R"(vc1image)" //
                            ,R"(vcr1)" //
                            ,R"(xl)" //
                            ,R"(vmdvideo)" //
                            ,R"(vmnc)" //
                            ,R"(vp3)" //
                            ,R"(vp4)" //
                            ,R"(vp5)" //
                            ,R"(vp6)" //
                            ,R"(vp6a)" //
                            ,R"(vp6f)" //
                            ,R"(vp7)" //
                            ,R"(vp8)" //
                            ,R"(libvpx)" //
                            ,R"(vp8_cuvid)" //
                            ,R"(vp8_qsv)" //
                            ,R"(vp9)" //
                            ,R"(libvpx-vp9)" //
                            ,R"(vp9_cuvid)" //
                            ,R"(vp9_qsv)" //
                            ,R"(wcmv)" //
                            ,R"(webp)" //
                            ,R"(wmv1)" //
                            ,R"(wmv2)" //
                            ,R"(wmv3)" //
                            ,R"(wmv3image)" //
                            ,R"(wnv1)" //
                            ,R"(wrapped_avframe)" //
                            ,R"(vqavideo)" //
                            ,R"(xan_wc3)" //
                            ,R"(xan_wc4)" //
                            ,R"(xbin)" //
                            ,R"(xbm)" //
                            ,R"(xface)" //
                            ,R"(xpm)" //
                            ,R"(xwd)" //
                            ,R"(y41p)" //
                            ,R"(ylc)" //
                            ,R"(yop)" //
                            ,R"(yuv4)" //
                            ,R"(zerocodec)" //
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
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableSubtitleDecodersDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(libaribb24 - libaribb24 ARIB STD-B24 caption decoder (codec arib_caption))" //
                            ,R"(ssa - ASS (Advanced SubStation Alpha) subtitle (codec ass))" //
                            ,R"(ass - ASS (Advanced SubStation Alpha) subtitle)" //
                            ,R"(dvbsub - DVB subtitles (codec dvb_subtitle))" //
                            ,R"(libzvbi_teletextdec - Libzvbi DVB teletext decoder (codec dvb_teletext))" //
                            ,R"(dvdsub - DVD subtitles (codec dvd_subtitle))" //
                            ,R"(cc_dec - Closed Caption (EIA-608 / CEA-708) (codec eia_608))" //
                            ,R"(pgssub - HDMV Presentation Graphic Stream subtitles (codec hdmv_pgs_subtitle))" //
                            ,R"(jacosub - JACOsub subtitle)" //
                            ,R"(microdvd - MicroDVD subtitle)" //
                            ,R"(mov_text - 3GPP Timed Text subtitle)" //
                            ,R"(mpl2 - MPL2 subtitle)" //
                            ,R"(pjs - PJS subtitle)" //
                            ,R"(realtext - RealText subtitle)" //
                            ,R"(sami - SAMI subtitle)" //
                            ,R"(stl - Spruce subtitle format)" //
                            ,R"(srt - SubRip subtitle (codec subrip))" //
                            ,R"(subrip - SubRip subtitle)" //
                            ,R"(subviewer - SubViewer subtitle)" //
                            ,R"(subviewer1 - SubViewer1 subtitle)" //
                            ,R"(text - Raw text subtitle)" //
                            ,R"(vplayer - VPlayer subtitle)" //
                            ,R"(webvtt - WebVTT subtitle)" //
                            ,R"(xsub - XSUB)" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(libaribb24)" //
                            ,R"(ssa)" //
                            ,R"(ass)" //
                            ,R"(dvbsub)" //
                            ,R"(libzvbi_teletextdec)" //
                            ,R"(dvdsub)" //
                            ,R"(cc_dec)" //
                            ,R"(pgssub)" //
                            ,R"(jacosub)" //
                            ,R"(microdvd)" //
                            ,R"(mov_text)" //
                            ,R"(mpl2)" //
                            ,R"(pjs)" //
                            ,R"(realtext)" //
                            ,R"(sami)" //
                            ,R"(stl)" //
                            ,R"(srt)" //
                            ,R"(subrip)" //
                            ,R"(subviewer)" //
                            ,R"(subviewer1)" //
                            ,R"(text)" //
                            ,R"(vplayer)" //
                            ,R"(webvtt)" //
                            ,R"(xsub)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableHWAccelsDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(cuda - Use the nVidia Cuda Video acceleration for video transcoding.)" //
                            ,R"(dxva2 - Use DXVA2 (DirectX Video Acceleration) API hardware acceleration.)" //
                            ,R"(qsv - Use the Intel QuickSync Video acceleration for video transcoding.)" //
                            ,R"(d3d11va - Use D3D11VA (DirectX Video Acceleration) API hardware acceleration.)" //
                            ,R"(opencl - Use OpenCL hardware acceleration for API video transcoding.))" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(cuda)" //
                            ,R"(dxva2)" //
                            ,R"(qsv)" //
                            ,R"(d3d11va)" //
                            ,R"(opencl)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableMediaEncoderFormatsDefault( bool verbose ) const
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
                            ,R"(ffmetadata - FFmpeg metadata in text (*.ffmeta))" //
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
                            ,R"(webm_dash_manifest - WebM DASH Manifest (*.xml))" //
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
                            ,R"(ffmetadata)" //
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
                            ,R"(webm_dash_manifest)" //
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

            QStringList CPreferences::availableMediaDecoderFormatsDefault( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(3dostr - 3DO STR (*.str))" //
                            ,R"(aa - Audible AA format files (*.aa))" //
                            ,R"(aac - raw ADTS AAC (Advanced Audio Coding) (*.aac))" //
                            ,R"(aax - CRI AAX (*.aax))" //
                            ,R"(ac4 - raw AC-4 (*.ac4))" //
                            ,R"(acm - Interplay ACM (*.acm))" //
                            ,R"(adf - Artworx Data Format (*.adf))" //
                            ,R"(adp - ADP (*.adp;*.dtk))" //
                            ,R"(ads - Sony PS2 ADS (*.ads;*.ss2))" //
                            ,R"(aea - MD STUDIO audio (*.aea))" //
                            ,R"(afc - AFC (*.afc))" //
                            ,R"(aix - CRI AIX (*.aix))" //
                            ,R"(ape - Monkey's Audio (*.ape;*.apl;*.mac))" //
                            ,R"(aqtitle - AQTitle subtitles (*.aqt))" //
                            ,R"(av1 - AV1 Annex B (*.obu))" //
                            ,R"(avr - AVR (Audio Visual Research) (*.avr))" //
                            ,R"(bfstm - BFSTM (Binary Cafe Stream) (*.bfstm;*.bcstm))" //
                            ,R"(binka - Bink Audio (*.binka))" //
                            ,R"(bitpacked - Bitpacked (*.bitpacked))" //
                            ,R"(bmv - Discworld II BMV (*.bmv))" //
                            ,R"(brstm - BRSTM (Binary Revolution Stream) (*.brstm))" //
                            ,R"(cdg - CD Graphics (*.cdg))" //
                            ,R"(cdxl - Commodore CDXL video (*.cdxl;*.xl))" //
                            ,R"(dcstr - Sega DC STR (*.str))" //
                            ,R"(derf - Xilam DERF (*.adp))" //
                            ,R"(dhav - Video DAV (*.dav))" //
                            ,R"(dss - Digital Speech Standard (DSS) (*.dss))" //
                            ,R"(dtshd - raw DTS-HD (*.dtshd))" //
                            ,R"(ea_cdata - Electronic Arts cdata (*.cdata))" //
                            ,R"(epaf - Ensoniq Paris Audio File (*.paf;*.fap))" //
                            ,R"(fsb - FMOD Sample Bank (*.fsb))" //
                            ,R"(fwse - Capcom's MT Framework sound (*.fwse))" //
                            ,R"(g729 - G.729 raw format demuxer (*.g729))" //
                            ,R"(genh - GENeric Header (*.genh))" //
                            ,R"(hca - CRI HCA (*.hca))" //
                            ,R"(idf - iCE Draw File (*.idf))" //
                            ,R"(ifv - IFV CCTV DVR (*.ifv))" //
                            ,R"(ingenient - raw Ingenient MJPEG (*.cgi))" //
                            ,R"(ipu - raw IPU Video (*.ipu))" //
                            ,R"(ivr - IVR (Internet Video Recording) (*.ivr))" //
                            ,R"(kux - KUX (YouKu) (*.kux))" //
                            ,R"(live_flv - live RTMP FLV (Flash Video) (*.flv))" //
                            ,R"(luodat - Video CCTV DAT (*.dat))" //
                            ,R"(lvf - LVF (*.lvf))" //
                            ,R"(matroska - Matroska / WebM (*.mkv;*.mk3d;*.mka;*.mks;*.webm))" //
                            ,R"(webm - Matroska / WebM (*.mkv;*.mk3d;*.mka;*.mks;*.webm))" //
                            ,R"(mca - MCA Audio Format (*.mca))" //
                            ,R"(mcc - MacCaption (*.mcc))" //
                            ,R"(mjpeg_2000 - raw MJPEG 2000 video (*.j2k))" //
                            ,R"(mods - MobiClip MODS (*.mods))" //
                            ,R"(moflex - MobiClip MOFLEX (*.moflex))" //
                            ,R"(mov - QuickTime / MOV (*.mov;*.mp4;*.m4a;*.3gp;*.3g2;*.mj2;*.psp;*.m4b;*.ism;*.ismv;*.isma;*.f4v))" //
                            ,R"(mp4 - QuickTime / MOV (*.mov;*.mp4;*.m4a;*.3gp;*.3g2;*.mj2;*.psp;*.m4b;*.ism;*.ismv;*.isma;*.f4v))" //
                            ,R"(m4a - QuickTime / MOV (*.mov;*.mp4;*.m4a;*.3gp;*.3g2;*.mj2;*.psp;*.m4b;*.ism;*.ismv;*.isma;*.f4v))" //
                            ,R"(3gp - QuickTime / MOV (*.mov;*.mp4;*.m4a;*.3gp;*.3g2;*.mj2;*.psp;*.m4b;*.ism;*.ismv;*.isma;*.f4v))" //
                            ,R"(3g2 - QuickTime / MOV (*.mov;*.mp4;*.m4a;*.3gp;*.3g2;*.mj2;*.psp;*.m4b;*.ism;*.ismv;*.isma;*.f4v))" //
                            ,R"(mj2 - QuickTime / MOV (*.mov;*.mp4;*.m4a;*.3gp;*.3g2;*.mj2;*.psp;*.m4b;*.ism;*.ismv;*.isma;*.f4v))" //
                            ,R"(mpc - Musepack (*.mpc))" //
                            ,R"(mpl2 - MPL2 subtitles (*.txt;*.mpl2))" //
                            ,R"(mpsub - MPlayer subtitles (*.sub))" //
                            ,R"(msf - Sony PS3 MSF (*.msf))" //
                            ,R"(mtaf - Konami PS2 MTAF (*.mtaf))" //
                            ,R"(musx - Eurocom MUSX (*.musx))" //
                            ,R"(mvi - Motion Pixels MVI (*.mvi))" //
                            ,R"(mxg - MxPEG clip (*.mxg))" //
                            ,R"(nc - NC camera feed (*.v))" //
                            ,R"(nistsphere - NIST SPeech HEader REsources (*.nist;*.sph))" //
                            ,R"(nsp - Computerized Speech Lab NSP (*.nsp))" //
                            ,R"(pjs - PJS (Phoenix Japanimation Society) subtitles (*.pjs))" //
                            ,R"(pvf - PVF (Portable Voice Format) (*.pvf))" //
                            ,R"(realtext - RealText subtitle format (*.rt))" //
                            ,R"(redspark - RedSpark (*.rsd))" //
                            ,R"(rsd - GameCube RSD (*.rsd))" //
                            ,R"(sami - SAMI subtitle format (*.smi;*.sami))" //
                            ,R"(sbg - SBaGen binaural beats script (*.sbg))" //
                            ,R"(sdr2 - SDR2 (*.sdr2))" //
                            ,R"(sds - MIDI Sample Dump Standard (*.sds))" //
                            ,R"(sdx - Sample Dump eXchange (*.sdx))" //
                            ,R"(ser - SER (Simple uncompressed video format for astronomical capturing) (*.ser))" //
                            ,R"(sga - Digital Pictures SGA (*.sga))" //
                            ,R"(shn - raw Shorten (*.shn))" //
                            ,R"(siff - Beam Software SIFF (*.vb;*.son))" //
                            ,R"(simbiosis_imx - Simbiosis Interactive IMX (*.imx))" //
                            ,R"(sln - Asterisk raw pcm (*.sln))" //
                            ,R"(stl - Spruce subtitle format (*.stl))" //
                            ,R"(subviewer - SubViewer subtitle format (*.sub))" //
                            ,R"(subviewer1 - SubViewer v1 subtitle format (*.sub))" //
                            ,R"(svag - Konami PS2 SVAG (*.svag))" //
                            ,R"(svs - Square SVS (*.svs))" //
                            ,R"(tak - raw TAK (*.tak))" //
                            ,R"(tty - Tele-typewriter (*.ans;*.art;*.asc;*.diz;*.ice;*.nfo;*.txt;*.vt))" //
                            ,R"(ty - TiVo TY Stream (*.ty;*.ty+))" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit (*.v210))" //
                            ,R"(v210x - Uncompressed 4:2:2 10-bit (*.yuv10))" //
                            ,R"(vag - Sony PS2 VAG (*.vag))" //
                            ,R"(vivo - Vivo (*.viv))" //
                            ,R"(vobsub - VobSub subtitle format (*.idx))" //
                            ,R"(vpk - Sony PS2 VPK (*.vpk))" //
                            ,R"(vplayer - VPlayer subtitles (*.txt))" //
                            ,R"(vqf - Nippon Telegraph and Telephone Corporation (NTT) TwinVQ (*.vqf;*.vql;*.vqe))" //
                            ,R"(wsd - Wideband Single-bit Data (WSD) (*.wsd))" //
                            ,R"(xmv - Microsoft XMV (*.xmv))" //
                            ,R"(xvag - Sony PS3 XVAG (*.xvag))" //
                            ,R"(yop - Psygnosis YOP (*.yop))" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(3dostr)" //
                            ,R"(aa)" //
                            ,R"(aac)" //
                            ,R"(aax)" //
                            ,R"(ac4)" //
                            ,R"(acm)" //
                            ,R"(adf)" //
                            ,R"(adp)" //
                            ,R"(ads)" //
                            ,R"(aea)" //
                            ,R"(afc)" //
                            ,R"(aix)" //
                            ,R"(ape)" //
                            ,R"(aqtitle)" //
                            ,R"(av1)" //
                            ,R"(avr)" //
                            ,R"(bfstm)" //
                            ,R"(binka)" //
                            ,R"(bitpacked)" //
                            ,R"(bmv)" //
                            ,R"(brstm)" //
                            ,R"(cdg)" //
                            ,R"(cdxl)" //
                            ,R"(dcstr)" //
                            ,R"(derf)" //
                            ,R"(dhav)" //
                            ,R"(dss)" //
                            ,R"(dtshd)" //
                            ,R"(ea_cdata)" //
                            ,R"(epaf)" //
                            ,R"(fsb)" //
                            ,R"(fwse)" //
                            ,R"(g729)" //
                            ,R"(genh)" //
                            ,R"(hca)" //
                            ,R"(idf)" //
                            ,R"(ifv)" //
                            ,R"(ingenient)" //
                            ,R"(ipu)" //
                            ,R"(ivr)" //
                            ,R"(kux)" //
                            ,R"(live_flv)" //
                            ,R"(luodat)" //
                            ,R"(lvf)" //
                            ,R"(matroska)" //
                            ,R"(webm)" //
                            ,R"(mca)" //
                            ,R"(mcc)" //
                            ,R"(mjpeg_2000)" //
                            ,R"(mods)" //
                            ,R"(moflex)" //
                            ,R"(mov)" //
                            ,R"(mp4)" //
                            ,R"(m4a)" //
                            ,R"(3gp)" //
                            ,R"(3g2)" //
                            ,R"(mj2)" //
                            ,R"(mpc)" //
                            ,R"(mpl2)" //
                            ,R"(mpsub)" //
                            ,R"(msf)" //
                            ,R"(mtaf)" //
                            ,R"(musx)" //
                            ,R"(mvi)" //
                            ,R"(mxg)" //
                            ,R"(nc)" //
                            ,R"(nistsphere)" //
                            ,R"(nsp)" //
                            ,R"(pjs)" //
                            ,R"(pvf)" //
                            ,R"(realtext)" //
                            ,R"(redspark)" //
                            ,R"(rsd)" //
                            ,R"(sami)" //
                            ,R"(sbg)" //
                            ,R"(sdr2)" //
                            ,R"(sds)" //
                            ,R"(sdx)" //
                            ,R"(ser)" //
                            ,R"(sga)" //
                            ,R"(shn)" //
                            ,R"(siff)" //
                            ,R"(simbiosis_imx)" //
                            ,R"(sln)" //
                            ,R"(stl)" //
                            ,R"(subviewer)" //
                            ,R"(subviewer1)" //
                            ,R"(svag)" //
                            ,R"(svs)" //
                            ,R"(tak)" //
                            ,R"(tty)" //
                            ,R"(ty)" //
                            ,R"(v210)" //
                            ,R"(v210x)" //
                            ,R"(vag)" //
                            ,R"(vivo)" //
                            ,R"(vobsub)" //
                            ,R"(vpk)" //
                            ,R"(vplayer)" //
                            ,R"(vqf)" //
                            ,R"(wsd)" //
                            ,R"(xmv)" //
                            ,R"(xvag)" //
                            ,R"(yop)" //
                        } );
                    return defaultValue;
                }
            }

            NSABUtils::TFormatMap CPreferences::getEncoderFormatExtensionsMapDefault() const
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
                                ,{ R"(dvd)", QStringList( { R"(*.dvd)" } ) } //
                                ,{ R"(avm2)", QStringList( { } ) } //
                                ,{ R"(gxf)", QStringList( { R"(*.gxf)" } ) } //
                                ,{ R"(3gp)", QStringList( { R"(*.3gp)" } ) } //
                                ,{ R"(a64)", QStringList( { R"(*.a64)" } ) } //
                                ,{ R"(mpeg)", QStringList( { R"(*.mpg)", R"(*.mpeg)" } ) } //
                                ,{ R"(asf)", QStringList( { R"(*.asf)", R"(*.wmv)", R"(*.wma)" } ) } //
                                ,{ R"(framemd5)", QStringList( { } ) } //
                                ,{ R"(dash)", QStringList( { R"(*.mpd)" } ) } //
                                ,{ R"(amv)", QStringList( { R"(*.amv)" } ) } //
                                ,{ R"(psp)", QStringList( { R"(*.mp4)", R"(*.psp)" } ) } //
                                ,{ R"(ivf)", QStringList( { R"(*.ivf)" } ) } //
                                ,{ R"(asf_stream)", QStringList( { R"(*.asf)", R"(*.wmv)", R"(*.wma)" } ) } //
                                ,{ R"(obu)", QStringList( { R"(*.obu)" } ) } //
                                ,{ R"(h261)", QStringList( { R"(*.h261)" } ) } //
                                ,{ R"(yuv4mpegpipe)", QStringList( { R"(*.y4m)" } ) } //
                                ,{ R"(avi)", QStringList( { R"(*.avi)" } ) } //
                                ,{ R"(mpjpeg)", QStringList( { R"(*.mjpg)" } ) } //
                                ,{ R"(avs2)", QStringList( { R"(*.avs)", R"(*.avs2)" } ) } //
                                ,{ R"(avs3)", QStringList( { R"(*.avs3)" } ) } //
                                ,{ R"(mxf_opatom)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(cavsvideo)", QStringList( { R"(*.cavs)" } ) } //
                                ,{ R"(crc)", QStringList( { } ) } //
                                ,{ R"(dirac)", QStringList( { R"(*.drc)", R"(*.vc2)" } ) } //
                                ,{ R"(hash)", QStringList( { } ) } //
                                ,{ R"(dnxhd)", QStringList( { R"(*.dnxhd)", R"(*.dnxhr)" } ) } //
                                ,{ R"(ogg)", QStringList( { R"(*.ogg)" } ) } //
                                ,{ R"(mkvtimestamp_v2)", QStringList( { } ) } //
                                ,{ R"(dv)", QStringList( { R"(*.dv)" } ) } //
                                ,{ R"(svcd)", QStringList( { R"(*.vob)" } ) } //
                                ,{ R"(mxf)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(film_cpk)", QStringList( { R"(*.cpk)" } ) } //
                                ,{ R"(mpegts)", QStringList( { R"(*.ts)", R"(*.m2t)", R"(*.m2ts)", R"(*.mts)" } ) } //
                                ,{ R"(hds)", QStringList( { } ) } //
                                ,{ R"(filmstrip)", QStringList( { R"(*.flm)" } ) } //
                                ,{ R"(rtp_mpegts)", QStringList( { } ) } //
                                ,{ R"(ipod)", QStringList( { R"(*.m4v)", R"(*.m4a)", R"(*.m4b)" } ) } //
                                ,{ R"(fits)", QStringList( { R"(*.fits)" } ) } //
                                ,{ R"(rm)", QStringList( { R"(*.rm)", R"(*.ra)" } ) } //
                                ,{ R"(hls)", QStringList( { R"(*.m3u8)" } ) } //
                                ,{ R"(flv)", QStringList( { R"(*.flv)" } ) } //
                                ,{ R"(framecrc)", QStringList( { } ) } //
                                ,{ R"(framehash)", QStringList( { } ) } //
                                ,{ R"(h263)", QStringList( { R"(*.h263)" } ) } //
                                ,{ R"(rtp)", QStringList( { } ) } //
                                ,{ R"(h264)", QStringList( { R"(*.h264)", R"(*.264)" } ) } //
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
                                 { R"(mxf_d10)", QStringList( { } ) } //
                                ,{ R"(mov)", QStringList( { R"(*.mov)" } ) } //
                                ,{ R"(mmf)", QStringList( { R"(*.mmf)" } ) } //
                                ,{ R"(f4v)", QStringList( { R"(*.f4v)" } ) } //
                                ,{ R"(aiff)", QStringList( { R"(*.aif)", R"(*.aiff)", R"(*.afc)", R"(*.aifc)" } ) } //
                                ,{ R"(f32le)", QStringList( { } ) } //
                                ,{ R"(wtv)", QStringList( { R"(*.wtv)" } ) } //
                                ,{ R"(3g2)", QStringList( { R"(*.3g2)" } ) } //
                                ,{ R"(latm)", QStringList( { R"(*.latm)", R"(*.loas)" } ) } //
                                ,{ R"(chromaprint)", QStringList( { } ) } //
                                ,{ R"(ac3)", QStringList( { R"(*.ac3)" } ) } //
                                ,{ R"(alaw)", QStringList( { R"(*.al)" } ) } //
                                ,{ R"(gxf)", QStringList( { R"(*.gxf)" } ) } //
                                ,{ R"(3gp)", QStringList( { R"(*.3gp)" } ) } //
                                ,{ R"(s32be)", QStringList( { } ) } //
                                ,{ R"(f64be)", QStringList( { } ) } //
                                ,{ R"(s8)", QStringList( { R"(*.sb)" } ) } //
                                ,{ R"(adts)", QStringList( { R"(*.aac)", R"(*.adts)" } ) } //
                                ,{ R"(mulaw)", QStringList( { R"(*.ul)" } ) } //
                                ,{ R"(adx)", QStringList( { R"(*.adx)" } ) } //
                                ,{ R"(alp)", QStringList( { R"(*.tun)", R"(*.pcm)" } ) } //
                                ,{ R"(amr)", QStringList( { R"(*.amr)" } ) } //
                                ,{ R"(apm)", QStringList( { R"(*.apm)" } ) } //
                                ,{ R"(dts)", QStringList( { R"(*.dts)" } ) } //
                                ,{ R"(aptx)", QStringList( { R"(*.aptx)" } ) } //
                                ,{ R"(ircam)", QStringList( { R"(*.sf)", R"(*.ircam)" } ) } //
                                ,{ R"(aptx_hd)", QStringList( { R"(*.aptxhd)" } ) } //
                                ,{ R"(argo_asf)", QStringList( { } ) } //
                                ,{ R"(argo_cvg)", QStringList( { R"(*.cvg)" } ) } //
                                ,{ R"(u16be)", QStringList( { } ) } //
                                ,{ R"(bit)", QStringList( { R"(*.bit)" } ) } //
                                ,{ R"(ast)", QStringList( { R"(*.ast)" } ) } //
                                ,{ R"(au)", QStringList( { R"(*.au)" } ) } //
                                ,{ R"(g726le)", QStringList( { } ) } //
                                ,{ R"(avm2)", QStringList( { } ) } //
                                ,{ R"(mp3)", QStringList( { R"(*.mp3)" } ) } //
                                ,{ R"(caf)", QStringList( { R"(*.caf)" } ) } //
                                ,{ R"(f64le)", QStringList( { } ) } //
                                ,{ R"(codec2)", QStringList( { R"(*.c2)" } ) } //
                                ,{ R"(mxf)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(film_cpk)", QStringList( { R"(*.cpk)" } ) } //
                                ,{ R"(codec2raw)", QStringList( { } ) } //
                                ,{ R"(crc)", QStringList( { } ) } //
                                ,{ R"(framemd5)", QStringList( { } ) } //
                                ,{ R"(dash)", QStringList( { R"(*.mpd)" } ) } //
                                ,{ R"(daud)", QStringList( { R"(*.302)" } ) } //
                                ,{ R"(ogg)", QStringList( { R"(*.ogg)" } ) } //
                                ,{ R"(dv)", QStringList( { R"(*.dv)" } ) } //
                                ,{ R"(eac3)", QStringList( { R"(*.eac3)" } ) } //
                                ,{ R"(gsm)", QStringList( { R"(*.gsm)" } ) } //
                                ,{ R"(f32be)", QStringList( { } ) } //
                                ,{ R"(mlp)", QStringList( { R"(*.mlp)" } ) } //
                                ,{ R"(flac)", QStringList( { R"(*.flac)" } ) } //
                                ,{ R"(framecrc)", QStringList( { } ) } //
                                ,{ R"(framehash)", QStringList( { } ) } //
                                ,{ R"(g722)", QStringList( { R"(*.g722)" } ) } //
                                ,{ R"(g723_1)", QStringList( { R"(*.tco)", R"(*.rco)" } ) } //
                                ,{ R"(g726)", QStringList( { } ) } //
                                ,{ R"(hash)", QStringList( { } ) } //
                                ,{ R"(hds)", QStringList( { } ) } //
                                ,{ R"(hls)", QStringList( { R"(*.m3u8)" } ) } //
                                ,{ R"(rm)", QStringList( { R"(*.rm)", R"(*.ra)" } ) } //
                                ,{ R"(ilbc)", QStringList( { R"(*.lbc)" } ) } //
                                ,{ R"(kvag)", QStringList( { R"(*.vag)" } ) } //
                                ,{ R"(md5)", QStringList( { } ) } //
                                ,{ R"(mp2)", QStringList( { R"(*.mp2)", R"(*.m2a)", R"(*.mpa)" } ) } //
                                ,{ R"(roq)", QStringList( { R"(*.roq)" } ) } //
                                ,{ R"(mxf_opatom)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(null)", QStringList( { } ) } //
                                ,{ R"(oga)", QStringList( { R"(*.oga)" } ) } //
                                ,{ R"(oma)", QStringList( { R"(*.oma)" } ) } //
                                ,{ R"(opus)", QStringList( { R"(*.opus)" } ) } //
                                ,{ R"(psp)", QStringList( { R"(*.mp4)", R"(*.psp)" } ) } //
                                ,{ R"(rso)", QStringList( { R"(*.rso)" } ) } //
                                ,{ R"(rtp)", QStringList( { } ) } //
                                ,{ R"(rtp_mpegts)", QStringList( { } ) } //
                                ,{ R"(rtsp)", QStringList( { } ) } //
                                ,{ R"(s16be)", QStringList( { } ) } //
                                ,{ R"(s16le)", QStringList( { R"(*.sw)" } ) } //
                                ,{ R"(s24be)", QStringList( { } ) } //
                                ,{ R"(s24le)", QStringList( { } ) } //
                                ,{ R"(smoothstreaming)", QStringList( { } ) } //
                                ,{ R"(s32le)", QStringList( { } ) } //
                                ,{ R"(sap)", QStringList( { } ) } //
                                ,{ R"(sbc)", QStringList( { R"(*.sbc)", R"(*.msbc)" } ) } //
                                ,{ R"(smjpeg)", QStringList( { } ) } //
                                ,{ R"(sox)", QStringList( { R"(*.sox)" } ) } //
                                ,{ R"(spdif)", QStringList( { R"(*.spdif)" } ) } //
                                ,{ R"(spx)", QStringList( { R"(*.spx)" } ) } //
                                ,{ R"(streamhash)", QStringList( { } ) } //
                                ,{ R"(swf)", QStringList( { R"(*.swf)" } ) } //
                                ,{ R"(truehd)", QStringList( { R"(*.thd)" } ) } //
                                ,{ R"(tta)", QStringList( { R"(*.tta)" } ) } //
                                ,{ R"(u16le)", QStringList( { R"(*.uw)" } ) } //
                                ,{ R"(u24be)", QStringList( { } ) } //
                                ,{ R"(u24le)", QStringList( { } ) } //
                                ,{ R"(u32be)", QStringList( { } ) } //
                                ,{ R"(u32le)", QStringList( { } ) } //
                                ,{ R"(u8)", QStringList( { R"(*.ub)" } ) } //
                                ,{ R"(uncodedframecrc)", QStringList( { } ) } //
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
                                ,{ R"(gif)", QStringList( { R"(*.gif)" } ) } //
                                ,{ R"(webp)", QStringList( { R"(*.webp)" } ) } //
                                ,{ R"(ico)", QStringList( { R"(*.ico)" } ) } //
                                ,{ R"(image2)", QStringList( { R"(*.bmp)", R"(*.dpx)", R"(*.exr)", R"(*.jls)", R"(*.jpeg)", R"(*.jpg)", R"(*.ljpg)", R"(*.pam)", R"(*.pbm)", R"(*.pcx)", R"(*.pfm)", R"(*.pgm)", R"(*.pgmyuv)", R"(*.png)", R"(*.ppm)", R"(*.sgi)", R"(*.tga)", R"(*.tif)", R"(*.tiff)", R"(*.jp2)", R"(*.j2c)", R"(*.j2k)", R"(*.xwd)", R"(*.sun)", R"(*.ras)", R"(*.rs)", R"(*.im1)", R"(*.im8)", R"(*.im24)", R"(*.sunras)", R"(*.xbm)", R"(*.xface)", R"(*.pix)", R"(*.y)" } ) } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eSubtitle, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(srt)", QStringList( { R"(*.srt)" } ) } //
                                ,{ R"(ass)", QStringList( { R"(*.ass)", R"(*.ssa)" } ) } //
                                ,{ R"(lrc)", QStringList( { R"(*.lrc)" } ) } //
                                ,{ R"(hls)", QStringList( { R"(*.m3u8)" } ) } //
                                ,{ R"(scc)", QStringList( { R"(*.scc)" } ) } //
                                ,{ R"(jacosub)", QStringList( { R"(*.jss)", R"(*.js)" } ) } //
                                ,{ R"(microdvd)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(sup)", QStringList( { R"(*.sup)" } ) } //
                                ,{ R"(ttml)", QStringList( { R"(*.ttml)" } ) } //
                                ,{ R"(webvtt)", QStringList( { R"(*.vtt)" } ) } //
                            } )
                        }
                    } );
                return defaultValue;
            }

            NSABUtils::TFormatMap CPreferences::getDecoderFormatExtensionsMapDefault() const
            {
                static auto defaultValue = NSABUtils::TFormatMap(
                    {
                         {
                            NSABUtils::EFormatType::eVideo, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(amrnb)", QStringList( { } ) } //
                                ,{ R"(3dostr)", QStringList( { R"(*.str)" } ) } //
                                ,{ R"(dfa)", QStringList( { } ) } //
                                ,{ R"(aa)", QStringList( { R"(*.aa)" } ) } //
                                ,{ R"(bink)", QStringList( { } ) } //
                                ,{ R"(musx)", QStringList( { R"(*.musx)" } ) } //
                                ,{ R"(afc)", QStringList( { R"(*.afc)" } ) } //
                                ,{ R"(ads)", QStringList( { R"(*.ads)", R"(*.ss2)" } ) } //
                                ,{ R"(bitpacked)", QStringList( { R"(*.bitpacked)" } ) } //
                                ,{ R"(loas)", QStringList( { } ) } //
                                ,{ R"(4xm)", QStringList( { } ) } //
                                ,{ R"(boa)", QStringList( { } ) } //
                                ,{ R"(aax)", QStringList( { R"(*.aax)" } ) } //
                                ,{ R"(hnm)", QStringList( { } ) } //
                                ,{ R"(adf)", QStringList( { R"(*.adf)" } ) } //
                                ,{ R"(dshow)", QStringList( { } ) } //
                                ,{ R"(aac)", QStringList( { R"(*.aac)" } ) } //
                                ,{ R"(dpx_pipe)", QStringList( { } ) } //
                                ,{ R"(avr)", QStringList( { R"(*.avr)" } ) } //
                                ,{ R"(pgm_pipe)", QStringList( { } ) } //
                                ,{ R"(ac4)", QStringList( { R"(*.ac4)" } ) } //
                                ,{ R"(acm)", QStringList( { R"(*.acm)" } ) } //
                                ,{ R"(pcx_pipe)", QStringList( { } ) } //
                                ,{ R"(ace)", QStringList( { } ) } //
                                ,{ R"(tedcaptions)", QStringList( { } ) } //
                                ,{ R"(apc)", QStringList( { } ) } //
                                ,{ R"(act)", QStringList( { } ) } //
                                ,{ R"(dss)", QStringList( { R"(*.dss)" } ) } //
                                ,{ R"(adp)", QStringList( { R"(*.adp)", R"(*.dtk)" } ) } //
                                ,{ R"(dds_pipe)", QStringList( { } ) } //
                                ,{ R"(aea)", QStringList( { R"(*.aea)" } ) } //
                                ,{ R"(dcstr)", QStringList( { R"(*.str)" } ) } //
                                ,{ R"(asf_o)", QStringList( { } ) } //
                                ,{ R"(bfi)", QStringList( { } ) } //
                                ,{ R"(aix)", QStringList( { R"(*.aix)" } ) } //
                                ,{ R"(alias_pix)", QStringList( { } ) } //
                                ,{ R"(amrwb)", QStringList( { } ) } //
                                ,{ R"(anm)", QStringList( { } ) } //
                                ,{ R"(cri_pipe)", QStringList( { } ) } //
                                ,{ R"(ppm_pipe)", QStringList( { } ) } //
                                ,{ R"(ape)", QStringList( { R"(*.ape)", R"(*.apl)", R"(*.mac)" } ) } //
                                ,{ R"(mgsts)", QStringList( { } ) } //
                                ,{ R"(aqtitle)", QStringList( { R"(*.aqt)" } ) } //
                                ,{ R"(argo_brp)", QStringList( { } ) } //
                                ,{ R"(av1)", QStringList( { R"(*.obu)" } ) } //
                                ,{ R"(rpl)", QStringList( { } ) } //
                                ,{ R"(avs)", QStringList( { } ) } //
                                ,{ R"(stl)", QStringList( { R"(*.stl)" } ) } //
                                ,{ R"(bethsoftvid)", QStringList( { } ) } //
                                ,{ R"(bfstm)", QStringList( { R"(*.bfstm)", R"(*.bcstm)" } ) } //
                                ,{ R"(bin)", QStringList( { } ) } //
                                ,{ R"(mpegvideo)", QStringList( { } ) } //
                                ,{ R"(binka)", QStringList( { R"(*.binka)" } ) } //
                                ,{ R"(matroska)", QStringList( { R"(*.mkv)", R"(*.mk3d)", R"(*.mka)", R"(*.mks)", R"(*.webm)" } ) } //
                                ,{ R"(tak)", QStringList( { R"(*.tak)" } ) } //
                                ,{ R"(brender_pix)", QStringList( { } ) } //
                                ,{ R"(msf)", QStringList( { R"(*.msf)" } ) } //
                                ,{ R"(bmp_pipe)", QStringList( { } ) } //
                                ,{ R"(bmv)", QStringList( { R"(*.bmv)" } ) } //
                                ,{ R"(brstm)", QStringList( { R"(*.brstm)" } ) } //
                                ,{ R"(derf)", QStringList( { R"(*.adp)" } ) } //
                                ,{ R"(r3d)", QStringList( { } ) } //
                                ,{ R"(c93)", QStringList( { } ) } //
                                ,{ R"(cdg)", QStringList( { R"(*.cdg)" } ) } //
                                ,{ R"(cdxl)", QStringList( { R"(*.cdxl)", R"(*.xl)" } ) } //
                                ,{ R"(mov)", QStringList( { R"(*.mov)", R"(*.mp4)", R"(*.m4a)", R"(*.3gp)", R"(*.3g2)", R"(*.mj2)", R"(*.psp)", R"(*.m4b)", R"(*.ism)", R"(*.ismv)", R"(*.isma)", R"(*.f4v)" } ) } //
                                ,{ R"(cine)", QStringList( { } ) } //
                                ,{ R"(concat)", QStringList( { } ) } //
                                ,{ R"(dhav)", QStringList( { R"(*.dav)" } ) } //
                                ,{ R"(xbin)", QStringList( { } ) } //
                                ,{ R"(ea)", QStringList( { } ) } //
                                ,{ R"(lxf)", QStringList( { } ) } //
                                ,{ R"(dsf)", QStringList( { } ) } //
                                ,{ R"(exr_pipe)", QStringList( { } ) } //
                                ,{ R"(dsicin)", QStringList( { } ) } //
                                ,{ R"(dvbtxt)", QStringList( { } ) } //
                                ,{ R"(dtshd)", QStringList( { R"(*.dtshd)" } ) } //
                                ,{ R"(dvbsub)", QStringList( { } ) } //
                                ,{ R"(dxa)", QStringList( { } ) } //
                                ,{ R"(ea_cdata)", QStringList( { R"(*.cdata)" } ) } //
                                ,{ R"(epaf)", QStringList( { R"(*.paf)", R"(*.fap)" } ) } //
                                ,{ R"(flic)", QStringList( { } ) } //
                                ,{ R"(frm)", QStringList( { } ) } //
                                ,{ R"(fsb)", QStringList( { R"(*.fsb)" } ) } //
                                ,{ R"(fwse)", QStringList( { R"(*.fwse)" } ) } //
                                ,{ R"(g729)", QStringList( { R"(*.g729)" } ) } //
                                ,{ R"(vobsub)", QStringList( { R"(*.idx)" } ) } //
                                ,{ R"(gdigrab)", QStringList( { } ) } //
                                ,{ R"(gdv)", QStringList( { } ) } //
                                ,{ R"(gem_pipe)", QStringList( { } ) } //
                                ,{ R"(pmp)", QStringList( { } ) } //
                                ,{ R"(genh)", QStringList( { R"(*.genh)" } ) } //
                                ,{ R"(gif_pipe)", QStringList( { } ) } //
                                ,{ R"(hca)", QStringList( { R"(*.hca)" } ) } //
                                ,{ R"(hcom)", QStringList( { } ) } //
                                ,{ R"(idcin)", QStringList( { } ) } //
                                ,{ R"(ingenient)", QStringList( { R"(*.cgi)" } ) } //
                                ,{ R"(idf)", QStringList( { R"(*.idf)" } ) } //
                                ,{ R"(ifv)", QStringList( { R"(*.ifv)" } ) } //
                                ,{ R"(iff)", QStringList( { } ) } //
                                ,{ R"(ipmovie)", QStringList( { } ) } //
                                ,{ R"(ipu)", QStringList( { R"(*.ipu)" } ) } //
                                ,{ R"(iss)", QStringList( { } ) } //
                                ,{ R"(iv8)", QStringList( { } ) } //
                                ,{ R"(s337m)", QStringList( { } ) } //
                                ,{ R"(ivr)", QStringList( { R"(*.ivr)" } ) } //
                                ,{ R"(j2k_pipe)", QStringList( { } ) } //
                                ,{ R"(jpeg_pipe)", QStringList( { } ) } //
                                ,{ R"(png_pipe)", QStringList( { } ) } //
                                ,{ R"(jpegls_pipe)", QStringList( { } ) } //
                                ,{ R"(sol)", QStringList( { } ) } //
                                ,{ R"(jv)", QStringList( { } ) } //
                                ,{ R"(kux)", QStringList( { R"(*.kux)" } ) } //
                                ,{ R"(lavfi)", QStringList( { } ) } //
                                ,{ R"(mtv)", QStringList( { } ) } //
                                ,{ R"(live_flv)", QStringList( { R"(*.flv)" } ) } //
                                ,{ R"(lmlm4)", QStringList( { } ) } //
                                ,{ R"(luodat)", QStringList( { R"(*.dat)" } ) } //
                                ,{ R"(lvf)", QStringList( { R"(*.lvf)" } ) } //
                                ,{ R"(mp4)", QStringList( { R"(*.mov)", R"(*.mp4)", R"(*.m4a)", R"(*.3gp)", R"(*.3g2)", R"(*.mj2)", R"(*.psp)", R"(*.m4b)", R"(*.ism)", R"(*.ismv)", R"(*.isma)", R"(*.f4v)" } ) } //
                                ,{ R"(webm)", QStringList( { R"(*.mkv)", R"(*.mk3d)", R"(*.mka)", R"(*.mks)", R"(*.webm)" } ) } //
                                ,{ R"(mca)", QStringList( { R"(*.mca)" } ) } //
                                ,{ R"(mcc)", QStringList( { R"(*.mcc)" } ) } //
                                ,{ R"(mjpeg_2000)", QStringList( { R"(*.j2k)" } ) } //
                                ,{ R"(mlv)", QStringList( { } ) } //
                                ,{ R"(realtext)", QStringList( { R"(*.rt)" } ) } //
                                ,{ R"(mm)", QStringList( { } ) } //
                                ,{ R"(mods)", QStringList( { R"(*.mods)" } ) } //
                                ,{ R"(moflex)", QStringList( { R"(*.moflex)" } ) } //
                                ,{ R"(m4a)", QStringList( { R"(*.mov)", R"(*.mp4)", R"(*.m4a)", R"(*.3gp)", R"(*.3g2)", R"(*.mj2)", R"(*.psp)", R"(*.m4b)", R"(*.ism)", R"(*.ismv)", R"(*.isma)", R"(*.f4v)" } ) } //
                                ,{ R"(3gp)", QStringList( { R"(*.mov)", R"(*.mp4)", R"(*.m4a)", R"(*.3gp)", R"(*.3g2)", R"(*.mj2)", R"(*.psp)", R"(*.m4b)", R"(*.ism)", R"(*.ismv)", R"(*.isma)", R"(*.f4v)" } ) } //
                                ,{ R"(3g2)", QStringList( { R"(*.mov)", R"(*.mp4)", R"(*.m4a)", R"(*.3gp)", R"(*.3g2)", R"(*.mj2)", R"(*.psp)", R"(*.m4b)", R"(*.ism)", R"(*.ismv)", R"(*.isma)", R"(*.f4v)" } ) } //
                                ,{ R"(mj2)", QStringList( { R"(*.mov)", R"(*.mp4)", R"(*.m4a)", R"(*.3gp)", R"(*.3g2)", R"(*.mj2)", R"(*.psp)", R"(*.m4b)", R"(*.ism)", R"(*.ismv)", R"(*.isma)", R"(*.f4v)" } ) } //
                                ,{ R"(photocd_pipe)", QStringList( { } ) } //
                                ,{ R"(mpc)", QStringList( { R"(*.mpc)" } ) } //
                                ,{ R"(mpc8)", QStringList( { } ) } //
                                ,{ R"(svag)", QStringList( { R"(*.svag)" } ) } //
                                ,{ R"(mpegtsraw)", QStringList( { } ) } //
                                ,{ R"(mpl2)", QStringList( { R"(*.txt)", R"(*.mpl2)" } ) } //
                                ,{ R"(qcp)", QStringList( { } ) } //
                                ,{ R"(mpsub)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(msnwctcp)", QStringList( { } ) } //
                                ,{ R"(msp)", QStringList( { } ) } //
                                ,{ R"(mtaf)", QStringList( { R"(*.mtaf)" } ) } //
                                ,{ R"(mv)", QStringList( { } ) } //
                                ,{ R"(mvi)", QStringList( { R"(*.mvi)" } ) } //
                                ,{ R"(mxg)", QStringList( { R"(*.mxg)" } ) } //
                                ,{ R"(nc)", QStringList( { R"(*.v)" } ) } //
                                ,{ R"(nistsphere)", QStringList( { R"(*.nist)", R"(*.sph)" } ) } //
                                ,{ R"(nsp)", QStringList( { R"(*.nsp)" } ) } //
                                ,{ R"(sdx)", QStringList( { R"(*.sdx)" } ) } //
                                ,{ R"(nsv)", QStringList( { } ) } //
                                ,{ R"(nuv)", QStringList( { } ) } //
                                ,{ R"(paf)", QStringList( { } ) } //
                                ,{ R"(pam_pipe)", QStringList( { } ) } //
                                ,{ R"(pbm_pipe)", QStringList( { } ) } //
                                ,{ R"(sdp)", QStringList( { } ) } //
                                ,{ R"(vpk)", QStringList( { R"(*.vpk)" } ) } //
                                ,{ R"(pgmyuv_pipe)", QStringList( { } ) } //
                                ,{ R"(pgx_pipe)", QStringList( { } ) } //
                                ,{ R"(pictor_pipe)", QStringList( { } ) } //
                                ,{ R"(pjs)", QStringList( { R"(*.pjs)" } ) } //
                                ,{ R"(pp_bnk)", QStringList( { } ) } //
                                ,{ R"(psd_pipe)", QStringList( { } ) } //
                                ,{ R"(psxstr)", QStringList( { } ) } //
                                ,{ R"(pva)", QStringList( { } ) } //
                                ,{ R"(pvf)", QStringList( { R"(*.pvf)" } ) } //
                                ,{ R"(qdraw_pipe)", QStringList( { } ) } //
                                ,{ R"(redspark)", QStringList( { R"(*.rsd)" } ) } //
                                ,{ R"(wc3movie)", QStringList( { } ) } //
                                ,{ R"(rl2)", QStringList( { } ) } //
                                ,{ R"(rsd)", QStringList( { R"(*.rsd)" } ) } //
                                ,{ R"(sami)", QStringList( { R"(*.smi)", R"(*.sami)" } ) } //
                                ,{ R"(sbg)", QStringList( { R"(*.sbg)" } ) } //
                                ,{ R"(scd)", QStringList( { } ) } //
                                ,{ R"(sdr2)", QStringList( { R"(*.sdr2)" } ) } //
                                ,{ R"(sds)", QStringList( { R"(*.sds)" } ) } //
                                ,{ R"(ser)", QStringList( { R"(*.ser)" } ) } //
                                ,{ R"(sga)", QStringList( { R"(*.sga)" } ) } //
                                ,{ R"(sgi_pipe)", QStringList( { } ) } //
                                ,{ R"(shn)", QStringList( { R"(*.shn)" } ) } //
                                ,{ R"(siff)", QStringList( { R"(*.vb)", R"(*.son)" } ) } //
                                ,{ R"(simbiosis_imx)", QStringList( { R"(*.imx)" } ) } //
                                ,{ R"(sln)", QStringList( { R"(*.sln)" } ) } //
                                ,{ R"(smk)", QStringList( { } ) } //
                                ,{ R"(xwma)", QStringList( { } ) } //
                                ,{ R"(smush)", QStringList( { } ) } //
                                ,{ R"(subviewer)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(subviewer1)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(sunrast_pipe)", QStringList( { } ) } //
                                ,{ R"(svg_pipe)", QStringList( { } ) } //
                                ,{ R"(svs)", QStringList( { R"(*.svs)" } ) } //
                                ,{ R"(thp)", QStringList( { } ) } //
                                ,{ R"(tiertexseq)", QStringList( { } ) } //
                                ,{ R"(xwd_pipe)", QStringList( { } ) } //
                                ,{ R"(tiff_pipe)", QStringList( { } ) } //
                                ,{ R"(tmv)", QStringList( { } ) } //
                                ,{ R"(tty)", QStringList( { R"(*.ans)", R"(*.art)", R"(*.asc)", R"(*.diz)", R"(*.ice)", R"(*.nfo)", R"(*.txt)", R"(*.vt)" } ) } //
                                ,{ R"(txd)", QStringList( { } ) } //
                                ,{ R"(ty)", QStringList( { R"(*.ty)", R"(*.ty+)" } ) } //
                                ,{ R"(vqf)", QStringList( { R"(*.vqf)", R"(*.vql)", R"(*.vqe)" } ) } //
                                ,{ R"(v210)", QStringList( { R"(*.v210)" } ) } //
                                ,{ R"(v210x)", QStringList( { R"(*.yuv10)" } ) } //
                                ,{ R"(vag)", QStringList( { R"(*.vag)" } ) } //
                                ,{ R"(wve)", QStringList( { } ) } //
                                ,{ R"(vfwcap)", QStringList( { } ) } //
                                ,{ R"(vividas)", QStringList( { } ) } //
                                ,{ R"(vivo)", QStringList( { R"(*.viv)" } ) } //
                                ,{ R"(vmd)", QStringList( { } ) } //
                                ,{ R"(vplayer)", QStringList( { R"(*.txt)" } ) } //
                                ,{ R"(webp_pipe)", QStringList( { } ) } //
                                ,{ R"(wsd)", QStringList( { R"(*.wsd)" } ) } //
                                ,{ R"(wsvqa)", QStringList( { } ) } //
                                ,{ R"(xa)", QStringList( { } ) } //
                                ,{ R"(xbm_pipe)", QStringList( { } ) } //
                                ,{ R"(xmv)", QStringList( { R"(*.xmv)" } ) } //
                                ,{ R"(xpm_pipe)", QStringList( { } ) } //
                                ,{ R"(xvag)", QStringList( { R"(*.xvag)" } ) } //
                                ,{ R"(yop)", QStringList( { R"(*.yop)" } ) } //
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

            bool CPreferences::getTranscodeVideoDefault() const
            {
                return true;
            }

            bool CPreferences::getOnlyTranscodeVideoOnFormatChangeDefault() const
            {
                return true;
            }

            QString CPreferences::getTranscodeToVideoCodecDefault() const
            {
                return R"(libx265)";
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