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
                        ,R"(complete)" //
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
                        ,R"(MP4)" //
                        ,R"(MT)" //
                        ,R"(MX)" //
                        ,R"(NAISU)" //
                        ,R"(NF)" //
                        ,R"(nogrp)" //
                        ,R"(NTb)" //
                        ,R"(PCOK)" //
                        ,R"(PECULATE)" //
                        ,R"(PEGASUS)" //
                        ,R"(PEPE)" //
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
                        ,R"(TiMELORDS)" //
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

            bool CPreferences::getConvertMediaContainerDefault() const
            {
                return true;
            }

            QString CPreferences::getConvertMediaToContainerDefault() const
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

            bool CPreferences::getAddAACAudioCodecDefault() const
            {
                return true;
            }

            QString CPreferences::getTranscodeToAudioCodecDefault() const
            {
                return R"(aac)";
            }

            bool CPreferences::getTranscodeVideoDefault() const
            {
                return true;
            }

            bool CPreferences::getOnlyTranscodeVideoOnFormatChangeDefault() const
            {
                return false;
            }

            QString CPreferences::getTranscodeToVideoCodecDefault() const
            {
                return R"(hevc_nvenc)";
            }

            bool CPreferences::getLosslessEncodingDefault() const
            {
                return false;
            }

            bool CPreferences::getUseCRFDefault() const
            {
                return true;
            }

            bool CPreferences::getUseExplicitCRFDefault() const
            {
                return true;
            }

            int CPreferences::getExplicitCRFDefault() const
            {
                return 18;
            }

            bool CPreferences::getUsePresetDefault() const
            {
                return false;
            }

            ETranscodePreset CPreferences::getPresetDefault() const
            {
                return NMediaManager::NPreferences::NCore::ETranscodePreset::eMedium;
            }

            bool CPreferences::getUseTuneDefault() const
            {
                return false;
            }

            ETranscodeTune CPreferences::getTuneDefault() const
            {
                return NMediaManager::NPreferences::NCore::ETranscodeTune::eFilm;
            }

            bool CPreferences::getUseProfileDefault() const
            {
                return false;
            }

            ETranscodeProfile CPreferences::getProfileDefault() const
            {
                return NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain422_12;
            }

            QStringList CPreferences::availableMediaEncoderFormatsStatic( bool verbose ) const
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
                            ,R"(avif - AVIF (*.avif))" //
                            ,R"(avs2 - raw AVS2-P2/IEEE1857.4 video (*.avs;*.avs2))" //
                            ,R"(avs3 - AVS3-P2/IEEE1857.10 (*.avs3))" //
                            ,R"(bit - G.729 BIT file format (*.bit))" //
                            ,R"(caf - Apple CAF (Core Audio Format) (*.caf))" //
                            ,R"(cavsvideo - raw Chinese AVS (Audio Video Standard) video (*.cavs))" //
                            ,R"(codec2 - codec2 .c2 muxer (*.c2))" //
                            ,R"(dash - DASH Muxer (*.mpd))" //
                            ,R"(daud - D-Cinema audio (*.302))" //
                            ,R"(dfpwm - raw DFPWM1a (*.dfpwm))" //
                            ,R"(dirac - raw Dirac (*.drc;*.vc2))" //
                            ,R"(dnxhd - raw DNxHD (SMPTE VC-3) (*.dnxhd;*.dnxhr))" //
                            ,R"(dts - raw DTS (*.dts))" //
                            ,R"(dv - DV (Digital Video) (*.dv))" //
                            ,R"(dvd - MPEG-2 PS (DVD VOB) (*.dvd))" //
                            ,R"(eac3 - raw E-AC-3 (*.eac3;*.ec3))" //
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
                            ,R"(image2 - image2 sequence (*.bmp;*.dpx;*.exr;*.jls;*.jpeg;*.jpg;*.jxl;*.ljpg;*.pam;*.pbm;*.pcx;*.pfm;*.pgm;*.pgmyuv;*.phm;*.png;*.ppm;*.sgi;*.tga;*.tif;*.tiff;*.jp2;*.j2c;*.j2k;*.xwd;*.sun;*.ras;*.rs;*.im1;*.im8;*.im24;*.sunras;*.vbn;*.xbm;*.xface;*.pix;*.y;*.avif;*.qoi;*.hdr;*.wbmp))" //
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
                            ,R"(avif)" //
                            ,R"(avs2)" //
                            ,R"(avs3)" //
                            ,R"(bit)" //
                            ,R"(caf)" //
                            ,R"(cavsvideo)" //
                            ,R"(codec2)" //
                            ,R"(dash)" //
                            ,R"(daud)" //
                            ,R"(dfpwm)" //
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

            NSABUtils::TFormatMap CPreferences::getEncoderFormatExtensionsMapStatic() const
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
                                ,{ R"(sdl,sdl2)", QStringList( { } ) } //
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
                                 { R"(mxf_d10)", QStringList( { } ) } //
                                ,{ R"(mmf)", QStringList( { R"(*.mmf)" } ) } //
                                ,{ R"(mov)", QStringList( { R"(*.mov)" } ) } //
                                ,{ R"(aiff)", QStringList( { R"(*.aif)", R"(*.aiff)", R"(*.afc)", R"(*.aifc)" } ) } //
                                ,{ R"(f4v)", QStringList( { R"(*.f4v)" } ) } //
                                ,{ R"(wtv)", QStringList( { R"(*.wtv)" } ) } //
                                ,{ R"(f32le)", QStringList( { } ) } //
                                ,{ R"(3g2)", QStringList( { R"(*.3g2)" } ) } //
                                ,{ R"(alaw)", QStringList( { R"(*.al)" } ) } //
                                ,{ R"(ac3)", QStringList( { R"(*.ac3)" } ) } //
                                ,{ R"(chromaprint)", QStringList( { } ) } //
                                ,{ R"(latm)", QStringList( { R"(*.latm)", R"(*.loas)" } ) } //
                                ,{ R"(f64be)", QStringList( { } ) } //
                                ,{ R"(s8)", QStringList( { R"(*.sb)" } ) } //
                                ,{ R"(gxf)", QStringList( { R"(*.gxf)" } ) } //
                                ,{ R"(3gp)", QStringList( { R"(*.3gp)" } ) } //
                                ,{ R"(s32be)", QStringList( { } ) } //
                                ,{ R"(adts)", QStringList( { R"(*.aac)", R"(*.adts)" } ) } //
                                ,{ R"(adx)", QStringList( { R"(*.adx)" } ) } //
                                ,{ R"(mulaw)", QStringList( { R"(*.ul)" } ) } //
                                ,{ R"(alp)", QStringList( { R"(*.tun)", R"(*.pcm)" } ) } //
                                ,{ R"(amr)", QStringList( { R"(*.amr)" } ) } //
                                ,{ R"(apm)", QStringList( { R"(*.apm)" } ) } //
                                ,{ R"(dts)", QStringList( { R"(*.dts)" } ) } //
                                ,{ R"(aptx)", QStringList( { R"(*.aptx)" } ) } //
                                ,{ R"(aptx_hd)", QStringList( { R"(*.aptxhd)" } ) } //
                                ,{ R"(ircam)", QStringList( { R"(*.sf)", R"(*.ircam)" } ) } //
                                ,{ R"(argo_asf)", QStringList( { } ) } //
                                ,{ R"(argo_cvg)", QStringList( { R"(*.cvg)" } ) } //
                                ,{ R"(u16be)", QStringList( { } ) } //
                                ,{ R"(ast)", QStringList( { R"(*.ast)" } ) } //
                                ,{ R"(bit)", QStringList( { R"(*.bit)" } ) } //
                                ,{ R"(au)", QStringList( { R"(*.au)" } ) } //
                                ,{ R"(avm2)", QStringList( { } ) } //
                                ,{ R"(g726le)", QStringList( { } ) } //
                                ,{ R"(f64le)", QStringList( { } ) } //
                                ,{ R"(mp3)", QStringList( { R"(*.mp3)" } ) } //
                                ,{ R"(caf)", QStringList( { R"(*.caf)" } ) } //
                                ,{ R"(codec2)", QStringList( { R"(*.c2)" } ) } //
                                ,{ R"(codec2raw)", QStringList( { } ) } //
                                ,{ R"(mxf)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(film_cpk)", QStringList( { R"(*.cpk)" } ) } //
                                ,{ R"(crc)", QStringList( { } ) } //
                                ,{ R"(framemd5)", QStringList( { } ) } //
                                ,{ R"(dash)", QStringList( { R"(*.mpd)" } ) } //
                                ,{ R"(daud)", QStringList( { R"(*.302)" } ) } //
                                ,{ R"(dfpwm)", QStringList( { R"(*.dfpwm)" } ) } //
                                ,{ R"(u24be)", QStringList( { } ) } //
                                ,{ R"(dv)", QStringList( { R"(*.dv)" } ) } //
                                ,{ R"(eac3)", QStringList( { R"(*.eac3)", R"(*.ec3)" } ) } //
                                ,{ R"(f32be)", QStringList( { } ) } //
                                ,{ R"(gsm)", QStringList( { R"(*.gsm)" } ) } //
                                ,{ R"(flac)", QStringList( { R"(*.flac)" } ) } //
                                ,{ R"(mlp)", QStringList( { R"(*.mlp)" } ) } //
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
                                ,{ R"(roq)", QStringList( { R"(*.roq)" } ) } //
                                ,{ R"(mp2)", QStringList( { R"(*.mp2)", R"(*.m2a)", R"(*.mpa)" } ) } //
                                ,{ R"(mxf_opatom)", QStringList( { R"(*.mxf)" } ) } //
                                ,{ R"(null)", QStringList( { } ) } //
                                ,{ R"(oga)", QStringList( { R"(*.oga)" } ) } //
                                ,{ R"(ogg)", QStringList( { R"(*.ogg)" } ) } //
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
                                ,{ R"(smoothstreaming)", QStringList( { } ) } //
                                ,{ R"(s24le)", QStringList( { } ) } //
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
                                ,{ R"(avif)", QStringList( { R"(*.avif)" } ) } //
                                ,{ R"(webp)", QStringList( { R"(*.webp)" } ) } //
                                ,{ R"(gif)", QStringList( { R"(*.gif)" } ) } //
                                ,{ R"(ico)", QStringList( { R"(*.ico)" } ) } //
                                ,{ R"(image2)", QStringList( { R"(*.bmp)", R"(*.dpx)", R"(*.exr)", R"(*.jls)", R"(*.jpeg)", R"(*.jpg)", R"(*.jxl)", R"(*.ljpg)", R"(*.pam)", R"(*.pbm)", R"(*.pcx)", R"(*.pfm)", R"(*.pgm)", R"(*.pgmyuv)", R"(*.phm)", R"(*.png)", R"(*.ppm)", R"(*.sgi)", R"(*.tga)", R"(*.tif)", R"(*.tiff)", R"(*.jp2)", R"(*.j2c)", R"(*.j2k)", R"(*.xwd)", R"(*.sun)", R"(*.ras)", R"(*.rs)", R"(*.im1)", R"(*.im8)", R"(*.im24)", R"(*.sunras)", R"(*.vbn)", R"(*.xbm)", R"(*.xface)", R"(*.pix)", R"(*.y)", R"(*.avif)", R"(*.qoi)", R"(*.hdr)", R"(*.wbmp)" } ) } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eSubtitle, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(lrc)", QStringList( { R"(*.lrc)" } ) } //
                                ,{ R"(srt)", QStringList( { R"(*.srt)" } ) } //
                                ,{ R"(ass)", QStringList( { R"(*.ass)", R"(*.ssa)" } ) } //
                                ,{ R"(hls)", QStringList( { R"(*.m3u8)" } ) } //
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

            QStringList CPreferences::availableMediaDecoderFormatsStatic( bool verbose ) const
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
                            ,R"(avif - AVIF (*.avif))" //
                            ,R"(avs2 - raw AVS2-P2/IEEE1857.4 video (*.avs;*.avs2))" //
                            ,R"(avs3 - AVS3-P2/IEEE1857.10 (*.avs3))" //
                            ,R"(bit - G.729 BIT file format (*.bit))" //
                            ,R"(caf - Apple CAF (Core Audio Format) (*.caf))" //
                            ,R"(cavsvideo - raw Chinese AVS (Audio Video Standard) video (*.cavs))" //
                            ,R"(codec2 - codec2 .c2 muxer (*.c2))" //
                            ,R"(dash - DASH Muxer (*.mpd))" //
                            ,R"(daud - D-Cinema audio (*.302))" //
                            ,R"(dfpwm - raw DFPWM1a (*.dfpwm))" //
                            ,R"(dirac - raw Dirac (*.drc;*.vc2))" //
                            ,R"(dnxhd - raw DNxHD (SMPTE VC-3) (*.dnxhd;*.dnxhr))" //
                            ,R"(dts - raw DTS (*.dts))" //
                            ,R"(dv - DV (Digital Video) (*.dv))" //
                            ,R"(dvd - MPEG-2 PS (DVD VOB) (*.dvd))" //
                            ,R"(eac3 - raw E-AC-3 (*.eac3;*.ec3))" //
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
                            ,R"(image2 - image2 sequence (*.bmp;*.dpx;*.exr;*.jls;*.jpeg;*.jpg;*.jxl;*.ljpg;*.pam;*.pbm;*.pcx;*.pfm;*.pgm;*.pgmyuv;*.phm;*.png;*.ppm;*.sgi;*.tga;*.tif;*.tiff;*.jp2;*.j2c;*.j2k;*.xwd;*.sun;*.ras;*.rs;*.im1;*.im8;*.im24;*.sunras;*.vbn;*.xbm;*.xface;*.pix;*.y;*.avif;*.qoi;*.hdr;*.wbmp))" //
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
                            ,R"(avif)" //
                            ,R"(avs2)" //
                            ,R"(avs3)" //
                            ,R"(bit)" //
                            ,R"(caf)" //
                            ,R"(cavsvideo)" //
                            ,R"(codec2)" //
                            ,R"(dash)" //
                            ,R"(daud)" //
                            ,R"(dfpwm)" //
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

            NSABUtils::TFormatMap CPreferences::getDecoderFormatExtensionsMapStatic() const
            {
                static auto defaultValue = NSABUtils::TFormatMap(
                    {
                         {
                            NSABUtils::EFormatType::eVideo, std::unordered_map< QString, QStringList >
                             ( {
                                 { R"(amrnb)", QStringList( { } ) } //
                                ,{ R"(3dostr)", QStringList( { R"(*.str)" } ) } //
                                ,{ R"(aa)", QStringList( { R"(*.aa)" } ) } //
                                ,{ R"(dfa)", QStringList( { } ) } //
                                ,{ R"(musx)", QStringList( { R"(*.musx)" } ) } //
                                ,{ R"(bink)", QStringList( { } ) } //
                                ,{ R"(ads)", QStringList( { R"(*.ads)", R"(*.ss2)" } ) } //
                                ,{ R"(afc)", QStringList( { R"(*.afc)" } ) } //
                                ,{ R"(4xm)", QStringList( { } ) } //
                                ,{ R"(bitpacked)", QStringList( { R"(*.bitpacked)" } ) } //
                                ,{ R"(loas)", QStringList( { } ) } //
                                ,{ R"(adf)", QStringList( { R"(*.adf)" } ) } //
                                ,{ R"(boa)", QStringList( { } ) } //
                                ,{ R"(aax)", QStringList( { R"(*.aax)" } ) } //
                                ,{ R"(hnm)", QStringList( { } ) } //
                                ,{ R"(dpx_pipe)", QStringList( { } ) } //
                                ,{ R"(dshow)", QStringList( { } ) } //
                                ,{ R"(aac)", QStringList( { R"(*.aac)" } ) } //
                                ,{ R"(acm)", QStringList( { R"(*.acm)" } ) } //
                                ,{ R"(pcx_pipe)", QStringList( { } ) } //
                                ,{ R"(ace)", QStringList( { } ) } //
                                ,{ R"(tedcaptions)", QStringList( { } ) } //
                                ,{ R"(act)", QStringList( { } ) } //
                                ,{ R"(apc)", QStringList( { } ) } //
                                ,{ R"(adp)", QStringList( { R"(*.adp)", R"(*.dtk)" } ) } //
                                ,{ R"(dss)", QStringList( { R"(*.dss)" } ) } //
                                ,{ R"(aea)", QStringList( { R"(*.aea)" } ) } //
                                ,{ R"(dds_pipe)", QStringList( { } ) } //
                                ,{ R"(aix)", QStringList( { R"(*.aix)" } ) } //
                                ,{ R"(bfi)", QStringList( { } ) } //
                                ,{ R"(dcstr)", QStringList( { R"(*.str)" } ) } //
                                ,{ R"(asf_o)", QStringList( { } ) } //
                                ,{ R"(alias_pix)", QStringList( { } ) } //
                                ,{ R"(amrwb)", QStringList( { } ) } //
                                ,{ R"(anm)", QStringList( { } ) } //
                                ,{ R"(apac)", QStringList( { R"(*.apc)" } ) } //
                                ,{ R"(vag)", QStringList( { R"(*.vag)" } ) } //
                                ,{ R"(wve)", QStringList( { } ) } //
                                ,{ R"(ape)", QStringList( { R"(*.ape)", R"(*.apl)", R"(*.mac)" } ) } //
                                ,{ R"(mgsts)", QStringList( { } ) } //
                                ,{ R"(cri_pipe)", QStringList( { } ) } //
                                ,{ R"(ppm_pipe)", QStringList( { } ) } //
                                ,{ R"(aqtitle)", QStringList( { R"(*.aqt)" } ) } //
                                ,{ R"(argo_brp)", QStringList( { } ) } //
                                ,{ R"(av1)", QStringList( { R"(*.obu)" } ) } //
                                ,{ R"(rpl)", QStringList( { } ) } //
                                ,{ R"(avisynth)", QStringList( { R"(*.avs)" } ) } //
                                ,{ R"(dxa)", QStringList( { } ) } //
                                ,{ R"(avr)", QStringList( { R"(*.avr)" } ) } //
                                ,{ R"(pgm_pipe)", QStringList( { } ) } //
                                ,{ R"(avs)", QStringList( { } ) } //
                                ,{ R"(stl)", QStringList( { R"(*.stl)" } ) } //
                                ,{ R"(bethsoftvid)", QStringList( { } ) } //
                                ,{ R"(bfstm)", QStringList( { R"(*.bfstm)", R"(*.bcstm)" } ) } //
                                ,{ R"(bin)", QStringList( { } ) } //
                                ,{ R"(mpegvideo)", QStringList( { } ) } //
                                ,{ R"(binka)", QStringList( { R"(*.binka)" } ) } //
                                ,{ R"(tak)", QStringList( { R"(*.tak)" } ) } //
                                ,{ R"(bmp_pipe)", QStringList( { } ) } //
                                ,{ R"(brender_pix)", QStringList( { } ) } //
                                ,{ R"(msf)", QStringList( { R"(*.msf)" } ) } //
                                ,{ R"(bmv)", QStringList( { R"(*.bmv)" } ) } //
                                ,{ R"(bonk)", QStringList( { R"(*.bonk)" } ) } //
                                ,{ R"(pva)", QStringList( { } ) } //
                                ,{ R"(brstm)", QStringList( { R"(*.brstm)" } ) } //
                                ,{ R"(r3d)", QStringList( { } ) } //
                                ,{ R"(derf)", QStringList( { R"(*.adp)" } ) } //
                                ,{ R"(c93)", QStringList( { } ) } //
                                ,{ R"(cdg)", QStringList( { R"(*.cdg)" } ) } //
                                ,{ R"(cdxl)", QStringList( { R"(*.cdxl)", R"(*.xl)" } ) } //
                                ,{ R"(cine)", QStringList( { } ) } //
                                ,{ R"(concat)", QStringList( { } ) } //
                                ,{ R"(xbin)", QStringList( { } ) } //
                                ,{ R"(dhav)", QStringList( { R"(*.dav)" } ) } //
                                ,{ R"(lxf)", QStringList( { } ) } //
                                ,{ R"(ea)", QStringList( { } ) } //
                                ,{ R"(dsf)", QStringList( { } ) } //
                                ,{ R"(dsicin)", QStringList( { } ) } //
                                ,{ R"(rka)", QStringList( { R"(*.rka)" } ) } //
                                ,{ R"(dvbtxt)", QStringList( { } ) } //
                                ,{ R"(dtshd)", QStringList( { R"(*.dtshd)" } ) } //
                                ,{ R"(dvbsub)", QStringList( { } ) } //
                                ,{ R"(ea_cdata)", QStringList( { R"(*.cdata)" } ) } //
                                ,{ R"(epaf)", QStringList( { R"(*.paf)", R"(*.fap)" } ) } //
                                ,{ R"(exr_pipe)", QStringList( { } ) } //
                                ,{ R"(flic)", QStringList( { } ) } //
                                ,{ R"(frm)", QStringList( { } ) } //
                                ,{ R"(fsb)", QStringList( { R"(*.fsb)" } ) } //
                                ,{ R"(fwse)", QStringList( { R"(*.fwse)" } ) } //
                                ,{ R"(vobsub)", QStringList( { R"(*.idx)" } ) } //
                                ,{ R"(g729)", QStringList( { R"(*.g729)" } ) } //
                                ,{ R"(gdigrab)", QStringList( { } ) } //
                                ,{ R"(gdv)", QStringList( { } ) } //
                                ,{ R"(pmp)", QStringList( { } ) } //
                                ,{ R"(gem_pipe)", QStringList( { } ) } //
                                ,{ R"(genh)", QStringList( { R"(*.genh)" } ) } //
                                ,{ R"(gif_pipe)", QStringList( { } ) } //
                                ,{ R"(hca)", QStringList( { R"(*.hca)" } ) } //
                                ,{ R"(hcom)", QStringList( { } ) } //
                                ,{ R"(imf)", QStringList( { } ) } //
                                ,{ R"(hdr_pipe)", QStringList( { } ) } //
                                ,{ R"(ingenient)", QStringList( { R"(*.cgi)" } ) } //
                                ,{ R"(idcin)", QStringList( { } ) } //
                                ,{ R"(ifv)", QStringList( { R"(*.ifv)" } ) } //
                                ,{ R"(idf)", QStringList( { R"(*.idf)" } ) } //
                                ,{ R"(iff)", QStringList( { } ) } //
                                ,{ R"(ipmovie)", QStringList( { } ) } //
                                ,{ R"(ipu)", QStringList( { R"(*.ipu)" } ) } //
                                ,{ R"(iss)", QStringList( { } ) } //
                                ,{ R"(s337m)", QStringList( { } ) } //
                                ,{ R"(iv8)", QStringList( { } ) } //
                                ,{ R"(ivr)", QStringList( { R"(*.ivr)" } ) } //
                                ,{ R"(j2k_pipe)", QStringList( { } ) } //
                                ,{ R"(png_pipe)", QStringList( { } ) } //
                                ,{ R"(jpeg_pipe)", QStringList( { } ) } //
                                ,{ R"(sol)", QStringList( { } ) } //
                                ,{ R"(jpegls_pipe)", QStringList( { } ) } //
                                ,{ R"(jpegxl_pipe)", QStringList( { } ) } //
                                ,{ R"(jv)", QStringList( { } ) } //
                                ,{ R"(kux)", QStringList( { R"(*.kux)" } ) } //
                                ,{ R"(laf)", QStringList( { R"(*.laf)" } ) } //
                                ,{ R"(mtv)", QStringList( { } ) } //
                                ,{ R"(lavfi)", QStringList( { } ) } //
                                ,{ R"(libgme)", QStringList( { } ) } //
                                ,{ R"(libopenmpt)", QStringList( { R"(*.669)", R"(*.amf)", R"(*.ams)", R"(*.dbm)", R"(*.digi)", R"(*.dmf)", R"(*.dsm)", R"(*.dtm)", R"(*.far)", R"(*.gdm)", R"(*.ice)", R"(*.imf)", R"(*.it)", R"(*.j2b)", R"(*.m15)", R"(*.mdl)", R"(*.med)", R"(*.mmcmp)", R"(*.mms)", R"(*.mo3)", R"(*.mod)", R"(*.mptm)", R"(*.mt2)", R"(*.mtm)", R"(*.nst)", R"(*.okt)", R"(*.plm)", R"(*.ppm)", R"(*.psm)", R"(*.pt36)", R"(*.ptm)", R"(*.s3m)", R"(*.sfx)", R"(*.sfx2)", R"(*.st26)", R"(*.stk)", R"(*.stm)", R"(*.stp)", R"(*.ult)", R"(*.umx)", R"(*.wow)", R"(*.xm)", R"(*.xpk)" } ) } //
                                ,{ R"(live_flv)", QStringList( { R"(*.flv)" } ) } //
                                ,{ R"(lmlm4)", QStringList( { } ) } //
                                ,{ R"(luodat)", QStringList( { R"(*.dat)" } ) } //
                                ,{ R"(lvf)", QStringList( { R"(*.lvf)" } ) } //
                                ,{ R"(matroska,webm)", QStringList( { R"(*.mkv)", R"(*.mk3d)", R"(*.mka)", R"(*.mks)", R"(*.webm)" } ) } //
                                ,{ R"(mca)", QStringList( { R"(*.mca)" } ) } //
                                ,{ R"(mcc)", QStringList( { R"(*.mcc)" } ) } //
                                ,{ R"(mjpeg_2000)", QStringList( { R"(*.j2k)" } ) } //
                                ,{ R"(realtext)", QStringList( { R"(*.rt)" } ) } //
                                ,{ R"(mlv)", QStringList( { } ) } //
                                ,{ R"(mm)", QStringList( { } ) } //
                                ,{ R"(mods)", QStringList( { R"(*.mods)" } ) } //
                                ,{ R"(moflex)", QStringList( { R"(*.moflex)" } ) } //
                                ,{ R"(mov,mp4,m4a,3gp,3g2,mj2)", QStringList( { R"(*.mov)", R"(*.mp4)", R"(*.m4a)", R"(*.3gp)", R"(*.3g2)", R"(*.mj2)", R"(*.psp)", R"(*.m4b)", R"(*.ism)", R"(*.ismv)", R"(*.isma)", R"(*.f4v)", R"(*.avif)" } ) } //
                                ,{ R"(qoi_pipe)", QStringList( { } ) } //
                                ,{ R"(mpc)", QStringList( { R"(*.mpc)" } ) } //
                                ,{ R"(svag)", QStringList( { R"(*.svag)" } ) } //
                                ,{ R"(mpc8)", QStringList( { } ) } //
                                ,{ R"(mpegtsraw)", QStringList( { } ) } //
                                ,{ R"(qcp)", QStringList( { } ) } //
                                ,{ R"(mpl2)", QStringList( { R"(*.txt)", R"(*.mpl2)" } ) } //
                                ,{ R"(mpsub)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(msnwctcp)", QStringList( { } ) } //
                                ,{ R"(msp)", QStringList( { } ) } //
                                ,{ R"(mtaf)", QStringList( { R"(*.mtaf)" } ) } //
                                ,{ R"(mv)", QStringList( { } ) } //
                                ,{ R"(mvi)", QStringList( { R"(*.mvi)" } ) } //
                                ,{ R"(mxg)", QStringList( { R"(*.mxg)" } ) } //
                                ,{ R"(nc)", QStringList( { R"(*.v)" } ) } //
                                ,{ R"(nistsphere)", QStringList( { R"(*.nist)", R"(*.sph)" } ) } //
                                ,{ R"(sdx)", QStringList( { R"(*.sdx)" } ) } //
                                ,{ R"(nsp)", QStringList( { R"(*.nsp)" } ) } //
                                ,{ R"(nsv)", QStringList( { } ) } //
                                ,{ R"(nuv)", QStringList( { } ) } //
                                ,{ R"(openal)", QStringList( { } ) } //
                                ,{ R"(paf)", QStringList( { } ) } //
                                ,{ R"(pam_pipe)", QStringList( { } ) } //
                                ,{ R"(vpk)", QStringList( { R"(*.vpk)" } ) } //
                                ,{ R"(sdp)", QStringList( { } ) } //
                                ,{ R"(pbm_pipe)", QStringList( { } ) } //
                                ,{ R"(pfm_pipe)", QStringList( { } ) } //
                                ,{ R"(pgmyuv_pipe)", QStringList( { } ) } //
                                ,{ R"(pgx_pipe)", QStringList( { } ) } //
                                ,{ R"(svg_pipe)", QStringList( { } ) } //
                                ,{ R"(phm_pipe)", QStringList( { } ) } //
                                ,{ R"(photocd_pipe)", QStringList( { } ) } //
                                ,{ R"(pictor_pipe)", QStringList( { } ) } //
                                ,{ R"(pjs)", QStringList( { R"(*.pjs)" } ) } //
                                ,{ R"(pp_bnk)", QStringList( { } ) } //
                                ,{ R"(psd_pipe)", QStringList( { } ) } //
                                ,{ R"(psxstr)", QStringList( { } ) } //
                                ,{ R"(pvf)", QStringList( { R"(*.pvf)" } ) } //
                                ,{ R"(qdraw_pipe)", QStringList( { } ) } //
                                ,{ R"(wc3movie)", QStringList( { } ) } //
                                ,{ R"(redspark)", QStringList( { R"(*.rsd)" } ) } //
                                ,{ R"(rl2)", QStringList( { } ) } //
                                ,{ R"(rsd)", QStringList( { R"(*.rsd)" } ) } //
                                ,{ R"(sami)", QStringList( { R"(*.smi)", R"(*.sami)" } ) } //
                                ,{ R"(sbg)", QStringList( { R"(*.sbg)" } ) } //
                                ,{ R"(scd)", QStringList( { } ) } //
                                ,{ R"(sdns)", QStringList( { R"(*.sdns)" } ) } //
                                ,{ R"(sdr2)", QStringList( { R"(*.sdr2)" } ) } //
                                ,{ R"(sds)", QStringList( { R"(*.sds)" } ) } //
                                ,{ R"(ser)", QStringList( { R"(*.ser)" } ) } //
                                ,{ R"(sga)", QStringList( { R"(*.sga)" } ) } //
                                ,{ R"(sgi_pipe)", QStringList( { } ) } //
                                ,{ R"(shn)", QStringList( { R"(*.shn)" } ) } //
                                ,{ R"(siff)", QStringList( { R"(*.vb)", R"(*.son)" } ) } //
                                ,{ R"(simbiosis_imx)", QStringList( { R"(*.imx)" } ) } //
                                ,{ R"(sln)", QStringList( { R"(*.sln)" } ) } //
                                ,{ R"(xwma)", QStringList( { } ) } //
                                ,{ R"(smk)", QStringList( { } ) } //
                                ,{ R"(smush)", QStringList( { } ) } //
                                ,{ R"(subviewer)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(wavarc)", QStringList( { R"(*.wa)" } ) } //
                                ,{ R"(subviewer1)", QStringList( { R"(*.sub)" } ) } //
                                ,{ R"(sunrast_pipe)", QStringList( { } ) } //
                                ,{ R"(svs)", QStringList( { R"(*.svs)" } ) } //
                                ,{ R"(thp)", QStringList( { } ) } //
                                ,{ R"(xwd_pipe)", QStringList( { } ) } //
                                ,{ R"(tiertexseq)", QStringList( { } ) } //
                                ,{ R"(tiff_pipe)", QStringList( { } ) } //
                                ,{ R"(tmv)", QStringList( { } ) } //
                                ,{ R"(tty)", QStringList( { R"(*.ans)", R"(*.art)", R"(*.asc)", R"(*.diz)", R"(*.ice)", R"(*.nfo)", R"(*.txt)", R"(*.vt)" } ) } //
                                ,{ R"(txd)", QStringList( { } ) } //
                                ,{ R"(vqf)", QStringList( { R"(*.vqf)", R"(*.vql)", R"(*.vqe)" } ) } //
                                ,{ R"(ty)", QStringList( { R"(*.ty)", R"(*.ty+)" } ) } //
                                ,{ R"(v210)", QStringList( { R"(*.v210)" } ) } //
                                ,{ R"(v210x)", QStringList( { R"(*.yuv10)" } ) } //
                                ,{ R"(vbn_pipe)", QStringList( { } ) } //
                                ,{ R"(vfwcap)", QStringList( { } ) } //
                                ,{ R"(vividas)", QStringList( { } ) } //
                                ,{ R"(vivo)", QStringList( { R"(*.viv)" } ) } //
                                ,{ R"(vmd)", QStringList( { } ) } //
                                ,{ R"(vplayer)", QStringList( { R"(*.txt)" } ) } //
                                ,{ R"(wady)", QStringList( { R"(*.way)" } ) } //
                                ,{ R"(webp_pipe)", QStringList( { } ) } //
                                ,{ R"(wsd)", QStringList( { R"(*.wsd)" } ) } //
                                ,{ R"(wsvqa)", QStringList( { } ) } //
                                ,{ R"(xa)", QStringList( { } ) } //
                                ,{ R"(xbm_pipe)", QStringList( { } ) } //
                                ,{ R"(xmd)", QStringList( { R"(*.xmd)" } ) } //
                                ,{ R"(xmv)", QStringList( { R"(*.xmv)" } ) } //
                                ,{ R"(xpm_pipe)", QStringList( { } ) } //
                                ,{ R"(xvag)", QStringList( { R"(*.xvag)" } ) } //
                                ,{ R"(yop)", QStringList( { R"(*.yop)" } ) } //
                            } )
                        }
                    } );
                return defaultValue;
            }

            QStringList CPreferences::availableVideoEncodingCodecsStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(a64_multi - Multicolor charset for Commodore 64 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(a64_multi5 - Multicolor charset for Commodore 64, extended with 5th color (colram) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(alias_pix - Alias/Wavefront PIX image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(amv - AMV Video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(apng - APNG (Animated Portable Network Graphics) image - (Lossless compression))" //
                            ,R"(asv1 - ASUS V1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(asv2 - ASUS V2 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(av1 - Alliance for Open Media AV1 - (Lossy compression))" //
                            ,R"(avrp - Avid 1:1 10-bit RGB Packer - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(avs2 - AVS2-P2/IEEE1857.4 - (Lossy compression))" //
                            ,R"(avui - Avid Meridien Uncompressed - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(ayuv - Uncompressed packed MS 4:4:4:4 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(bitpacked - Bitpacked - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(bmp - BMP (Windows and OS/2 bitmap) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(cfhd - GoPro CineForm HD - (Lossy compression))" //
                            ,R"(cinepak - Cinepak - (Lossy compression))" //
                            ,R"(cljr - Cirrus Logic AccuPak - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dirac - Dirac - (Lossy compression) (Lossless compression))" //
                            ,R"(dnxhd - VC3/DNxHD - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dpx - DPX (Digital Picture Exchange) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(dvvideo - DV (Digital Video) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(exr - OpenEXR image - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(ffv1 - FFmpeg video codec #1 - (Lossless compression))" //
                            ,R"(ffvhuff - Huffyuv FFmpeg variant - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(fits - FITS (Flexible Image Transport System) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(flashsv - Flash Screen Video v1 - (Lossless compression))" //
                            ,R"(flashsv2 - Flash Screen Video v2 - (Lossy compression))" //
                            ,R"(flv1 - FLV / Sorenson Spark / Sorenson H.263 (Flash Video) - (Lossy compression))" //
                            ,R"(gif - CompuServe GIF (Graphics Interchange Format) - (Lossless compression))" //
                            ,R"(h261 - H.261 - (Lossy compression))" //
                            ,R"(h263 - H.263 / H.263-1996, H.263+ / H.263-1998 / H.263 version 2 - (Lossy compression))" //
                            ,R"(h263p - H.263+ / H.263-1998 / H.263 version 2 - (Lossy compression))" //
                            ,R"(h264 - H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 - (Lossy compression) (Lossless compression))" //
                            ,R"(hdr - HDR (Radiance RGBE format) image - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(hevc - H.265 / HEVC (High Efficiency Video Coding) - (Lossy compression))" //
                            ,R"(huffyuv - HuffYUV - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(jpeg2000 - JPEG 2000 - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(jpegls - JPEG-LS - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(jpegxl - JPEG XL - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(ljpeg - Lossless JPEG - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(magicyuv - MagicYUV video - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(mjpeg - Motion JPEG - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mpeg1video - MPEG-1 video - (Lossy compression))" //
                            ,R"(mpeg2video - MPEG-2 video - (Lossy compression))" //
                            ,R"(mpeg4 - MPEG-4 part 2 - (Lossy compression))" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2 - (Lossy compression))" //
                            ,R"(msmpeg4v3 - MPEG-4 part 2 Microsoft variant version 3 - (Lossy compression))" //
                            ,R"(msvideo1 - Microsoft Video 1 - (Lossy compression))" //
                            ,R"(pam - PAM (Portable AnyMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pbm - PBM (Portable BitMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcx - PC Paintbrush PCX image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pfm - PFM (Portable FloatMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pgm - PGM (Portable GrayMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pgmyuv - PGMYUV (Portable GrayMap YUV) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(phm - PHM (Portable HalfFloatMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(png - PNG (Portable Network Graphics) image - (Lossless compression))" //
                            ,R"(ppm - PPM (Portable PixelMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(prores - Apple ProRes (iCodec Pro) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(qoi - QOI (Quite OK Image) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(qtrle - QuickTime Animation (RLE) video - (Lossless compression))" //
                            ,R"(r10k - AJA Kona 10-bit RGB Codec - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(r210 - Uncompressed RGB 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(rawvideo - raw video - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(roq - id RoQ video - (Lossy compression))" //
                            ,R"(rpza - QuickTime video (RPZA) - (Lossy compression))" //
                            ,R"(rv10 - RealVideo 1.0 - (Lossy compression))" //
                            ,R"(rv20 - RealVideo 2.0 - (Lossy compression))" //
                            ,R"(sgi - SGI image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(smc - QuickTime Graphics (SMC) - (Lossy compression))" //
                            ,R"(snow - Snow - (Lossy compression) (Lossless compression))" //
                            ,R"(speedhq - NewTek SpeedHQ - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(sunrast - Sun Rasterfile image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(svq1 - Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1 - (Lossy compression))" //
                            ,R"(targa - Truevision Targa image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(theora - Theora - (Lossy compression))" //
                            ,R"(tiff - TIFF image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(utvideo - Ut Video - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v308 - Uncompressed packed 4:4:4 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v408 - Uncompressed packed QT 4:4:4:4 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v410 - Uncompressed 4:4:4 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(vbn - Vizrt Binary Image - (Lossy compression))" //
                            ,R"(vnull - Null video codec - )" //
                            ,R"(vp8 - On2 VP8 - (Lossy compression))" //
                            ,R"(vp9 - Google VP9 - (Lossy compression))" //
                            ,R"(wbmp - WBMP (Wireless Application Protocol Bitmap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(webp - WebP - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(wmv1 - Windows Media Video 7 - (Lossy compression))" //
                            ,R"(wmv2 - Windows Media Video 8 - (Lossy compression))" //
                            ,R"(wrapped_avframe - AVFrame to AVPacket passthrough - (Lossless compression))" //
                            ,R"(xbm - XBM (X BitMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(xface - X-face image - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(xwd - XWD (X Window Dump) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(y41p - Uncompressed YUV 4:1:1 12-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(yuv4 - Uncompressed packed 4:2:0 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(zlib - LCL (LossLess Codec Library) ZLIB - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(zmbv - Zip Motion Blocks Video - (Lossless compression))" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(a64_multi)" //
                            ,R"(a64_multi5)" //
                            ,R"(alias_pix)" //
                            ,R"(amv)" //
                            ,R"(apng)" //
                            ,R"(asv1)" //
                            ,R"(asv2)" //
                            ,R"(av1)" //
                            ,R"(avrp)" //
                            ,R"(avs2)" //
                            ,R"(avui)" //
                            ,R"(ayuv)" //
                            ,R"(bitpacked)" //
                            ,R"(bmp)" //
                            ,R"(cfhd)" //
                            ,R"(cinepak)" //
                            ,R"(cljr)" //
                            ,R"(dirac)" //
                            ,R"(dnxhd)" //
                            ,R"(dpx)" //
                            ,R"(dvvideo)" //
                            ,R"(exr)" //
                            ,R"(ffv1)" //
                            ,R"(ffvhuff)" //
                            ,R"(fits)" //
                            ,R"(flashsv)" //
                            ,R"(flashsv2)" //
                            ,R"(flv1)" //
                            ,R"(gif)" //
                            ,R"(h261)" //
                            ,R"(h263)" //
                            ,R"(h263p)" //
                            ,R"(h264)" //
                            ,R"(hdr)" //
                            ,R"(hevc)" //
                            ,R"(huffyuv)" //
                            ,R"(jpeg2000)" //
                            ,R"(jpegls)" //
                            ,R"(jpegxl)" //
                            ,R"(ljpeg)" //
                            ,R"(magicyuv)" //
                            ,R"(mjpeg)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpeg4)" //
                            ,R"(msmpeg4v2)" //
                            ,R"(msmpeg4v3)" //
                            ,R"(msvideo1)" //
                            ,R"(pam)" //
                            ,R"(pbm)" //
                            ,R"(pcx)" //
                            ,R"(pfm)" //
                            ,R"(pgm)" //
                            ,R"(pgmyuv)" //
                            ,R"(phm)" //
                            ,R"(png)" //
                            ,R"(ppm)" //
                            ,R"(prores)" //
                            ,R"(qoi)" //
                            ,R"(qtrle)" //
                            ,R"(r10k)" //
                            ,R"(r210)" //
                            ,R"(rawvideo)" //
                            ,R"(roq)" //
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
                            ,R"(theora)" //
                            ,R"(tiff)" //
                            ,R"(utvideo)" //
                            ,R"(v210)" //
                            ,R"(v308)" //
                            ,R"(v408)" //
                            ,R"(v410)" //
                            ,R"(vbn)" //
                            ,R"(vnull)" //
                            ,R"(vp8)" //
                            ,R"(vp9)" //
                            ,R"(wbmp)" //
                            ,R"(webp)" //
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

            QStringList CPreferences::availableVideoDecodingCodecsStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(012v - Uncompressed 4:2:2 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(4xm - 4X Movie - (Lossy compression))" //
                            ,R"(8bps - QuickTime 8BPS video - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(aasc - Autodesk RLE - (Lossless compression))" //
                            ,R"(agm - Amuse Graphics Movie - (Lossy compression))" //
                            ,R"(aic - Apple Intermediate Codec - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(alias_pix - Alias/Wavefront PIX image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(amv - AMV Video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(anm - Deluxe Paint Animation - (Lossy compression))" //
                            ,R"(ansi - ASCII/ANSI art - (Lossy compression))" //
                            ,R"(apng - APNG (Animated Portable Network Graphics) image - (Lossless compression))" //
                            ,R"(arbc - Gryphon's Anim Compressor - (Lossy compression))" //
                            ,R"(argo - Argonaut Games Video - (Lossy compression))" //
                            ,R"(asv1 - ASUS V1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(asv2 - ASUS V2 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(aura - Auravision AURA - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(aura2 - Auravision Aura 2 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(av1 - Alliance for Open Media AV1 - (Lossy compression))" //
                            ,R"(avrn - Avid AVI Codec - )" //
                            ,R"(avrp - Avid 1:1 10-bit RGB Packer - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(avs - AVS (Audio Video Standard) video - (Lossy compression))" //
                            ,R"(avs2 - AVS2-P2/IEEE1857.4 - (Lossy compression))" //
                            ,R"(avs3 - AVS3-P2/IEEE1857.10 - (Lossy compression))" //
                            ,R"(avui - Avid Meridien Uncompressed - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(ayuv - Uncompressed packed MS 4:4:4:4 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(bethsoftvid - Bethesda VID video - (Lossy compression))" //
                            ,R"(bfi - Brute Force & Ignorance - (Lossy compression))" //
                            ,R"(binkvideo - Bink video - (Lossy compression))" //
                            ,R"(bintext - Binary text - (Intra frame-only codec))" //
                            ,R"(bitpacked - Bitpacked - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(bmp - BMP (Windows and OS/2 bitmap) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(bmv_video - Discworld II BMV video - (Lossless compression))" //
                            ,R"(brender_pix - BRender PIX image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(c93 - Interplay C93 - (Lossy compression))" //
                            ,R"(cavs - Chinese AVS (Audio Video Standard) (AVS1-P2, JiZhun profile) - (Lossy compression))" //
                            ,R"(cdgraphics - CD Graphics video - (Lossy compression))" //
                            ,R"(cdtoons - CDToons video - (Lossless compression))" //
                            ,R"(cdxl - Commodore CDXL video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(cfhd - GoPro CineForm HD - (Lossy compression))" //
                            ,R"(cinepak - Cinepak - (Lossy compression))" //
                            ,R"(clearvideo - Iterated Systems ClearVideo - (Lossy compression))" //
                            ,R"(cljr - Cirrus Logic AccuPak - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(cllc - Canopus Lossless Codec - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(cmv - Electronic Arts CMV video - (Lossy compression))" //
                            ,R"(cpia - CPiA video format - )" //
                            ,R"(cri - Cintel RAW - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(cscd - CamStudio - (Lossless compression))" //
                            ,R"(cyuv - Creative YUV (CYUV) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dds - DirectDraw Surface image decoder - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(dfa - Chronomaster DFA - (Lossy compression))" //
                            ,R"(dirac - Dirac - (Lossy compression) (Lossless compression))" //
                            ,R"(dnxhd - VC3/DNxHD - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dpx - DPX (Digital Picture Exchange) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(dsicinvideo - Delphine Software International CIN video - (Lossy compression))" //
                            ,R"(dvvideo - DV (Digital Video) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dxa - Feeble Files/ScummVM DXA - (Lossless compression))" //
                            ,R"(dxtory - Dxtory - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(dxv - Resolume DXV - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(escape124 - Escape 124 - (Lossy compression))" //
                            ,R"(escape130 - Escape 130 - (Lossy compression))" //
                            ,R"(exr - OpenEXR image - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(ffv1 - FFmpeg video codec #1 - (Lossless compression))" //
                            ,R"(ffvhuff - Huffyuv FFmpeg variant - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(fic - Mirillis FIC - (Lossy compression))" //
                            ,R"(fits - FITS (Flexible Image Transport System) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(flashsv - Flash Screen Video v1 - (Lossless compression))" //
                            ,R"(flashsv2 - Flash Screen Video v2 - (Lossy compression))" //
                            ,R"(flic - Autodesk Animator Flic video - (Lossless compression))" //
                            ,R"(flv1 - FLV / Sorenson Spark / Sorenson H.263 (Flash Video) - (Lossy compression))" //
                            ,R"(fmvc - FM Screen Capture Codec - (Lossless compression))" //
                            ,R"(fraps - Fraps - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(frwu - Forward Uncompressed - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(g2m - Go2Meeting - (Lossy compression))" //
                            ,R"(gdv - Gremlin Digital Video - (Lossy compression))" //
                            ,R"(gem - GEM Raster image - (Lossy compression))" //
                            ,R"(gif - CompuServe GIF (Graphics Interchange Format) - (Lossless compression))" //
                            ,R"(h261 - H.261 - (Lossy compression))" //
                            ,R"(h263 - H.263 / H.263-1996, H.263+ / H.263-1998 / H.263 version 2 - (Lossy compression))" //
                            ,R"(h263i - Intel H.263 - (Lossy compression))" //
                            ,R"(h263p - H.263+ / H.263-1998 / H.263 version 2 - (Lossy compression))" //
                            ,R"(h264 - H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 - (Lossy compression) (Lossless compression))" //
                            ,R"(hap - Vidvox Hap - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(hdr - HDR (Radiance RGBE format) image - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(hevc - H.265 / HEVC (High Efficiency Video Coding) - (Lossy compression))" //
                            ,R"(hnm4video - HNM 4 video - (Lossy compression))" //
                            ,R"(hq_hqa - Canopus HQ/HQA - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(hqx - Canopus HQX - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(huffyuv - HuffYUV - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(hymt - HuffYUV MT - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(idcin - id Quake II CIN video - (Lossy compression))" //
                            ,R"(idf - iCEDraw text - (Intra frame-only codec))" //
                            ,R"(iff_ilbm - IFF ACBM/ANIM/DEEP/ILBM/PBM/RGB8/RGBN - (Lossy compression))" //
                            ,R"(imm4 - Infinity IMM4 - (Lossy compression))" //
                            ,R"(imm5 - Infinity IMM5 - (Lossy compression))" //
                            ,R"(indeo2 - Intel Indeo 2 - (Lossy compression))" //
                            ,R"(indeo3 - Intel Indeo 3 - (Lossy compression))" //
                            ,R"(indeo4 - Intel Indeo Video Interactive 4 - (Lossy compression))" //
                            ,R"(indeo5 - Intel Indeo Video Interactive 5 - (Lossy compression))" //
                            ,R"(interplayvideo - Interplay MVE video - (Lossy compression))" //
                            ,R"(ipu - IPU Video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(jpeg2000 - JPEG 2000 - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(jpegls - JPEG-LS - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(jpegxl - JPEG XL - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(jv - Bitmap Brothers JV video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(kgv1 - Kega Game Video - (Lossy compression))" //
                            ,R"(kmvc - Karl Morton's video codec - (Lossy compression))" //
                            ,R"(lagarith - Lagarith lossless - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(loco - LOCO - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(lscr - LEAD Screen Capture - (Lossy compression))" //
                            ,R"(m101 - Matrox Uncompressed SD - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(mad - Electronic Arts Madcow Video - (Lossy compression))" //
                            ,R"(magicyuv - MagicYUV video - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(mdec - Sony PlayStation MDEC (Motion DECoder) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(media100 - Media 100i - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mimic - Mimic - (Lossy compression))" //
                            ,R"(mjpeg - Motion JPEG - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mjpegb - Apple MJPEG-B - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mmvideo - American Laser Games MM Video - (Lossy compression))" //
                            ,R"(mobiclip - MobiClip Video - (Lossy compression))" //
                            ,R"(motionpixels - Motion Pixels video - (Lossy compression))" //
                            ,R"(mpeg1video - MPEG-1 video - (Lossy compression))" //
                            ,R"(mpeg2video - MPEG-2 video - (Lossy compression))" //
                            ,R"(mpeg4 - MPEG-4 part 2 - (Lossy compression))" //
                            ,R"(msa1 - MS ATC Screen - (Lossy compression))" //
                            ,R"(mscc - Mandsoft Screen Capture Codec - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(msmpeg4v1 - MPEG-4 part 2 Microsoft variant version 1 - (Lossy compression))" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2 - (Lossy compression))" //
                            ,R"(msmpeg4v3 - MPEG-4 part 2 Microsoft variant version 3 - (Lossy compression))" //
                            ,R"(msp2 - Microsoft Paint (MSP) version 2 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(msrle - Microsoft RLE - (Lossless compression))" //
                            ,R"(mss1 - MS Screen 1 - (Lossy compression))" //
                            ,R"(mss2 - MS Windows Media Video V9 Screen - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(msvideo1 - Microsoft Video 1 - (Lossy compression))" //
                            ,R"(mszh - LCL (LossLess Codec Library) MSZH - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(mts2 - MS Expression Encoder Screen - (Lossy compression))" //
                            ,R"(mv30 - MidiVid 3.0 - (Lossy compression))" //
                            ,R"(mvc1 - Silicon Graphics Motion Video Compressor 1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mvc2 - Silicon Graphics Motion Video Compressor 2 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mvdv - MidiVid VQ - (Lossy compression))" //
                            ,R"(mvha - MidiVid Archive Codec - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mwsc - MatchWare Screen Capture Codec - (Lossless compression))" //
                            ,R"(mxpeg - Mobotix MxPEG video - (Lossy compression))" //
                            ,R"(notchlc - NotchLC - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(nuv - NuppelVideo/RTJPEG - (Lossy compression))" //
                            ,R"(paf_video - Amazing Studio Packed Animation File Video - (Lossy compression))" //
                            ,R"(pam - PAM (Portable AnyMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pbm - PBM (Portable BitMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcx - PC Paintbrush PCX image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pfm - PFM (Portable FloatMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pgm - PGM (Portable GrayMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pgmyuv - PGMYUV (Portable GrayMap YUV) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pgx - PGX (JPEG2000 Test Format) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(phm - PHM (Portable HalfFloatMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(photocd - Kodak Photo CD - (Lossy compression))" //
                            ,R"(pictor - Pictor/PC Paint - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(pixlet - Apple Pixlet - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(png - PNG (Portable Network Graphics) image - (Lossless compression))" //
                            ,R"(ppm - PPM (Portable PixelMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(prores - Apple ProRes (iCodec Pro) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(prosumer - Brooktree ProSumer Video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(psd - Photoshop PSD file - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(ptx - V.Flash PTX image - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(qdraw - Apple QuickDraw - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(qoi - QOI (Quite OK Image) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(qpeg - Q-team QPEG - (Lossy compression))" //
                            ,R"(qtrle - QuickTime Animation (RLE) video - (Lossless compression))" //
                            ,R"(r10k - AJA Kona 10-bit RGB Codec - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(r210 - Uncompressed RGB 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(rasc - RemotelyAnywhere Screen Capture - (Lossy compression))" //
                            ,R"(rawvideo - raw video - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(rl2 - RL2 video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(roq - id RoQ video - (Lossy compression))" //
                            ,R"(rpza - QuickTime video (RPZA) - (Lossy compression))" //
                            ,R"(rscc - innoHeim/Rsupport Screen Capture Codec - (Lossless compression))" //
                            ,R"(rv10 - RealVideo 1.0 - (Lossy compression))" //
                            ,R"(rv20 - RealVideo 2.0 - (Lossy compression))" //
                            ,R"(rv30 - RealVideo 3.0 - (Lossy compression))" //
                            ,R"(rv40 - RealVideo 4.0 - (Lossy compression))" //
                            ,R"(sanm - LucasArts SANM/SMUSH video - (Lossy compression))" //
                            ,R"(scpr - ScreenPressor - (Lossy compression) (Lossless compression))" //
                            ,R"(screenpresso - Screenpresso - (Lossless compression))" //
                            ,R"(sga - Digital Pictures SGA Video - (Lossy compression))" //
                            ,R"(sgi - SGI image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(sgirle - SGI RLE 8-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(sheervideo - BitJazz SheerVideo - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(simbiosis_imx - Simbiosis Interactive IMX Video - (Lossy compression))" //
                            ,R"(smackvideo - Smacker video - (Lossy compression))" //
                            ,R"(smc - QuickTime Graphics (SMC) - (Lossy compression))" //
                            ,R"(smvjpeg - Sigmatel Motion Video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(snow - Snow - (Lossy compression) (Lossless compression))" //
                            ,R"(sp5x - Sunplus JPEG (SP5X) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(speedhq - NewTek SpeedHQ - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(srgc - Screen Recorder Gold Codec - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(sunrast - Sun Rasterfile image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(svq1 - Sorenson Vector Quantizer 1 / Sorenson Video 1 / SVQ1 - (Lossy compression))" //
                            ,R"(svq3 - Sorenson Vector Quantizer 3 / Sorenson Video 3 / SVQ3 - (Lossy compression))" //
                            ,R"(targa - Truevision Targa image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(targa_y216 - Pinnacle TARGA CineWave YUV16 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(tdsc - TDSC - (Lossy compression))" //
                            ,R"(tgq - Electronic Arts TGQ video - (Lossy compression))" //
                            ,R"(tgv - Electronic Arts TGV video - (Lossy compression))" //
                            ,R"(theora - Theora - (Lossy compression))" //
                            ,R"(thp - Nintendo Gamecube THP video - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(tiertexseqvideo - Tiertex Limited SEQ video - (Lossy compression))" //
                            ,R"(tiff - TIFF image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(tmv - 8088flex TMV - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(tqi - Electronic Arts TQI video - (Lossy compression))" //
                            ,R"(truemotion1 - Duck TrueMotion 1.0 - (Lossy compression))" //
                            ,R"(truemotion2 - Duck TrueMotion 2.0 - (Lossy compression))" //
                            ,R"(truemotion2rt - Duck TrueMotion 2.0 Real Time - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(tscc - TechSmith Screen Capture Codec - (Lossless compression))" //
                            ,R"(tscc2 - TechSmith Screen Codec 2 - (Lossy compression))" //
                            ,R"(txd - Renderware TXD (TeXture Dictionary) image - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ulti - IBM UltiMotion - (Lossy compression))" //
                            ,R"(utvideo - Ut Video - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v210x - Uncompressed 4:2:2 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v308 - Uncompressed packed 4:4:4 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v408 - Uncompressed packed QT 4:4:4:4 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(v410 - Uncompressed 4:4:4 10-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(vb - Beam Software VB - (Lossy compression))" //
                            ,R"(vble - VBLE Lossless Codec - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(vbn - Vizrt Binary Image - (Lossy compression))" //
                            ,R"(vc1 - SMPTE VC-1 - (Lossy compression))" //
                            ,R"(vc1image - Windows Media Video 9 Image v2 - (Lossy compression))" //
                            ,R"(vcr1 - ATI VCR1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(vixl - Miro VideoXL - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(vmdvideo - Sierra VMD video - (Lossy compression))" //
                            ,R"(vmnc - VMware Screen Codec / VMware Video - (Lossless compression))" //
                            ,R"(vnull - Null video codec - )" //
                            ,R"(vp3 - On2 VP3 - (Lossy compression))" //
                            ,R"(vp4 - On2 VP4 - (Lossy compression))" //
                            ,R"(vp5 - On2 VP5 - (Lossy compression))" //
                            ,R"(vp6 - On2 VP6 - (Lossy compression))" //
                            ,R"(vp6a - On2 VP6 (Flash version, with alpha channel) - (Lossy compression))" //
                            ,R"(vp6f - On2 VP6 (Flash version) - (Lossy compression))" //
                            ,R"(vp7 - On2 VP7 - (Lossy compression))" //
                            ,R"(vp8 - On2 VP8 - (Lossy compression))" //
                            ,R"(vp9 - Google VP9 - (Lossy compression))" //
                            ,R"(vqc - ViewQuest VQC - (Lossy compression))" //
                            ,R"(wbmp - WBMP (Wireless Application Protocol Bitmap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(wcmv - WinCAM Motion Video - (Lossless compression))" //
                            ,R"(webp - WebP - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(wmv1 - Windows Media Video 7 - (Lossy compression))" //
                            ,R"(wmv2 - Windows Media Video 8 - (Lossy compression))" //
                            ,R"(wmv3 - Windows Media Video 9 - (Lossy compression))" //
                            ,R"(wmv3image - Windows Media Video 9 Image - (Lossy compression))" //
                            ,R"(wnv1 - Winnov WNV1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wrapped_avframe - AVFrame to AVPacket passthrough - (Lossless compression))" //
                            ,R"(ws_vqa - Westwood Studios VQA (Vector Quantized Animation) video - (Lossy compression))" //
                            ,R"(xan_wc3 - Wing Commander III / Xan - (Lossy compression))" //
                            ,R"(xan_wc4 - Wing Commander IV / Xxan - (Lossy compression))" //
                            ,R"(xbin - eXtended BINary text - (Intra frame-only codec))" //
                            ,R"(xbm - XBM (X BitMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(xface - X-face image - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(xpm - XPM (X PixMap) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(xwd - XWD (X Window Dump) image - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(y41p - Uncompressed YUV 4:1:1 12-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(ylc - YUY2 Lossless Codec - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(yop - Psygnosis YOP Video - (Lossy compression))" //
                            ,R"(yuv4 - Uncompressed packed 4:2:0 - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(zerocodec - ZeroCodec Lossless Video - (Lossless compression))" //
                            ,R"(zlib - LCL (LossLess Codec Library) ZLIB - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(zmbv - Zip Motion Blocks Video - (Lossless compression))" //
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
                            ,R"(av1)" //
                            ,R"(avrn)" //
                            ,R"(avrp)" //
                            ,R"(avs)" //
                            ,R"(avs2)" //
                            ,R"(avs3)" //
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
                            ,R"(cmv)" //
                            ,R"(cpia)" //
                            ,R"(cri)" //
                            ,R"(cscd)" //
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
                            ,R"(flv1)" //
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
                            ,R"(hap)" //
                            ,R"(hdr)" //
                            ,R"(hevc)" //
                            ,R"(hnm4video)" //
                            ,R"(hq_hqa)" //
                            ,R"(hqx)" //
                            ,R"(huffyuv)" //
                            ,R"(hymt)" //
                            ,R"(idcin)" //
                            ,R"(idf)" //
                            ,R"(iff_ilbm)" //
                            ,R"(imm4)" //
                            ,R"(imm5)" //
                            ,R"(indeo2)" //
                            ,R"(indeo3)" //
                            ,R"(indeo4)" //
                            ,R"(indeo5)" //
                            ,R"(interplayvideo)" //
                            ,R"(ipu)" //
                            ,R"(jpeg2000)" //
                            ,R"(jpegls)" //
                            ,R"(jpegxl)" //
                            ,R"(jv)" //
                            ,R"(kgv1)" //
                            ,R"(kmvc)" //
                            ,R"(lagarith)" //
                            ,R"(loco)" //
                            ,R"(lscr)" //
                            ,R"(m101)" //
                            ,R"(mad)" //
                            ,R"(magicyuv)" //
                            ,R"(mdec)" //
                            ,R"(media100)" //
                            ,R"(mimic)" //
                            ,R"(mjpeg)" //
                            ,R"(mjpegb)" //
                            ,R"(mmvideo)" //
                            ,R"(mobiclip)" //
                            ,R"(motionpixels)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpeg4)" //
                            ,R"(msa1)" //
                            ,R"(mscc)" //
                            ,R"(msmpeg4v1)" //
                            ,R"(msmpeg4v2)" //
                            ,R"(msmpeg4v3)" //
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
                            ,R"(phm)" //
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
                            ,R"(qoi)" //
                            ,R"(qpeg)" //
                            ,R"(qtrle)" //
                            ,R"(r10k)" //
                            ,R"(r210)" //
                            ,R"(rasc)" //
                            ,R"(rawvideo)" //
                            ,R"(rl2)" //
                            ,R"(roq)" //
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
                            ,R"(smackvideo)" //
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
                            ,R"(tgq)" //
                            ,R"(tgv)" //
                            ,R"(theora)" //
                            ,R"(thp)" //
                            ,R"(tiertexseqvideo)" //
                            ,R"(tiff)" //
                            ,R"(tmv)" //
                            ,R"(tqi)" //
                            ,R"(truemotion1)" //
                            ,R"(truemotion2)" //
                            ,R"(truemotion2rt)" //
                            ,R"(tscc)" //
                            ,R"(tscc2)" //
                            ,R"(txd)" //
                            ,R"(ulti)" //
                            ,R"(utvideo)" //
                            ,R"(v210)" //
                            ,R"(v210x)" //
                            ,R"(v308)" //
                            ,R"(v408)" //
                            ,R"(v410)" //
                            ,R"(vb)" //
                            ,R"(vble)" //
                            ,R"(vbn)" //
                            ,R"(vc1)" //
                            ,R"(vc1image)" //
                            ,R"(vcr1)" //
                            ,R"(vixl)" //
                            ,R"(vmdvideo)" //
                            ,R"(vmnc)" //
                            ,R"(vnull)" //
                            ,R"(vp3)" //
                            ,R"(vp4)" //
                            ,R"(vp5)" //
                            ,R"(vp6)" //
                            ,R"(vp6a)" //
                            ,R"(vp6f)" //
                            ,R"(vp7)" //
                            ,R"(vp8)" //
                            ,R"(vp9)" //
                            ,R"(vqc)" //
                            ,R"(wbmp)" //
                            ,R"(wcmv)" //
                            ,R"(webp)" //
                            ,R"(wmv1)" //
                            ,R"(wmv2)" //
                            ,R"(wmv3)" //
                            ,R"(wmv3image)" //
                            ,R"(wnv1)" //
                            ,R"(wrapped_avframe)" //
                            ,R"(ws_vqa)" //
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

            QStringList CPreferences::availableAudioEncodingCodecsStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(aac - AAC (Advanced Audio Coding) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ac3 - ATSC A/52A (AC-3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_adx - SEGA CRI ADX ADPCM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_argo - ADPCM Argonaut Games - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_g722 - G.722 ADPCM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_g726 - G.726 ADPCM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_g726le - G.726 ADPCM little-endian - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_alp - ADPCM IMA High Voltage Software ALP - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_amv - ADPCM IMA AMV - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_apm - ADPCM IMA Ubisoft APM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_qt - ADPCM IMA QuickTime - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_ssi - ADPCM IMA Simon & Schuster Interactive - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_wav - ADPCM IMA WAV - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_ws - ADPCM IMA Westwood - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ms - ADPCM Microsoft - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_swf - ADPCM Shockwave Flash - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_yamaha - ADPCM Yamaha - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(alac - ALAC (Apple Lossless Audio Codec) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(amr_nb - AMR-NB (Adaptive Multi-Rate NarrowBand) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(anull - Null audio codec - )" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(comfortnoise - RFC 3389 Comfort Noise - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dfpwm - DFPWM (Dynamic Filter Pulse Width Modulation) - (Lossy compression))" //
                            ,R"(dts - DCA (DTS Coherent Acoustics) - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(eac3 - ATSC A/52B (AC-3, E-AC-3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(g723_1 - G.723.1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mlp - MLP (Meridian Lossless Packing) - (Lossless compression))" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mp3 - MP3 (MPEG audio layer 3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(nellymoser - Nellymoser Asao - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(opus - Opus (Opus Interactive Audio Codec) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(pcm_alaw - PCM A-law / G.711 A-law - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(pcm_bluray - PCM signed 16|20|24-bit big-endian for Blu-ray media - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_dvd - PCM signed 20|24-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f32be - PCM 32-bit floating point big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f32le - PCM 32-bit floating point little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f64be - PCM 64-bit floating point big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f64le - PCM 64-bit floating point little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_mulaw - PCM mu-law / G.711 mu-law - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(pcm_s16be - PCM signed 16-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s16be_planar - PCM signed 16-bit big-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s16le - PCM signed 16-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s16le_planar - PCM signed 16-bit little-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24be - PCM signed 24-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24daud - PCM D-Cinema audio signed 24-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24le - PCM signed 24-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24le_planar - PCM signed 24-bit little-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s32be - PCM signed 32-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s32le - PCM signed 32-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s32le_planar - PCM signed 32-bit little-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s64be - PCM signed 64-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s64le - PCM signed 64-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s8 - PCM signed 8-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s8_planar - PCM signed 8-bit planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u16be - PCM unsigned 16-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u16le - PCM unsigned 16-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u24be - PCM unsigned 24-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u24le - PCM unsigned 24-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u32be - PCM unsigned 32-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u32le - PCM unsigned 32-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u8 - PCM unsigned 8-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_vidc - PCM Archimedes VIDC - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ra_144 - RealAudio 1.0 (14.4K) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(roq_dpcm - DPCM id RoQ - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(s302m - SMPTE 302M - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(sbc - SBC (low-complexity subband codec) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(sonic - Sonic - (Intra frame-only codec))" //
                            ,R"(sonicls - Sonic lossless - (Intra frame-only codec))" //
                            ,R"(truehd - TrueHD - (Lossless compression))" //
                            ,R"(tta - TTA (True Audio) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(vorbis - Vorbis - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wavpack - WavPack - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(wmav1 - Windows Media Audio 1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wmav2 - Windows Media Audio 2 - (Intra frame-only codec) (Lossy compression))" //
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
                            ,R"(adpcm_adx)" //
                            ,R"(adpcm_argo)" //
                            ,R"(adpcm_g722)" //
                            ,R"(adpcm_g726)" //
                            ,R"(adpcm_g726le)" //
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
                            ,R"(amr_nb)" //
                            ,R"(anull)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(comfortnoise)" //
                            ,R"(dfpwm)" //
                            ,R"(dts)" //
                            ,R"(eac3)" //
                            ,R"(flac)" //
                            ,R"(g723_1)" //
                            ,R"(mlp)" //
                            ,R"(mp2)" //
                            ,R"(mp3)" //
                            ,R"(nellymoser)" //
                            ,R"(opus)" //
                            ,R"(pcm_alaw)" //
                            ,R"(pcm_bluray)" //
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
                            ,R"(ra_144)" //
                            ,R"(roq_dpcm)" //
                            ,R"(s302m)" //
                            ,R"(sbc)" //
                            ,R"(sonic)" //
                            ,R"(sonicls)" //
                            ,R"(truehd)" //
                            ,R"(tta)" //
                            ,R"(vorbis)" //
                            ,R"(wavpack)" //
                            ,R"(wmav1)" //
                            ,R"(wmav2)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableAudioDecodingCodecsStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(8svx_exp - 8SVX exponential - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(8svx_fib - 8SVX fibonacci - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(aac - AAC (Advanced Audio Coding) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(aac_latm - AAC LATM (Advanced Audio Coding LATM syntax) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ac3 - ATSC A/52A (AC-3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(acelp.kelvin - Sipro ACELP.KELVIN - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_4xm - ADPCM 4X Movie - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_adx - SEGA CRI ADX ADPCM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_afc - ADPCM Nintendo Gamecube AFC - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_agm - ADPCM AmuseGraphics Movie AGM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_aica - ADPCM Yamaha AICA - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_argo - ADPCM Argonaut Games - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ct - ADPCM Creative Technology - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_dtk - ADPCM Nintendo Gamecube DTK - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ea - ADPCM Electronic Arts - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ea_maxis_xa - ADPCM Electronic Arts Maxis CDROM XA - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ea_r1 - ADPCM Electronic Arts R1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ea_r2 - ADPCM Electronic Arts R2 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ea_r3 - ADPCM Electronic Arts R3 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ea_xas - ADPCM Electronic Arts XAS - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_g722 - G.722 ADPCM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_g726 - G.726 ADPCM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_g726le - G.726 ADPCM little-endian - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_acorn - ADPCM IMA Acorn Replay - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_alp - ADPCM IMA High Voltage Software ALP - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_amv - ADPCM IMA AMV - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_apc - ADPCM IMA CRYO APC - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_apm - ADPCM IMA Ubisoft APM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_cunning - ADPCM IMA Cunning Developments - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_dat4 - ADPCM IMA Eurocom DAT4 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_dk3 - ADPCM IMA Duck DK3 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_dk4 - ADPCM IMA Duck DK4 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_ea_eacs - ADPCM IMA Electronic Arts EACS - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_ea_sead - ADPCM IMA Electronic Arts SEAD - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_iss - ADPCM IMA Funcom ISS - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_moflex - ADPCM IMA MobiClip MOFLEX - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_mtf - ADPCM IMA Capcom's MT Framework - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_oki - ADPCM IMA Dialogic OKI - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_qt - ADPCM IMA QuickTime - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_rad - ADPCM IMA Radical - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_smjpeg - ADPCM IMA Loki SDL MJPEG - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_ssi - ADPCM IMA Simon & Schuster Interactive - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_wav - ADPCM IMA WAV - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ima_ws - ADPCM IMA Westwood - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_ms - ADPCM Microsoft - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_mtaf - ADPCM MTAF - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_psx - ADPCM Playstation - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_sbpro_2 - ADPCM Sound Blaster Pro 2-bit - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_sbpro_3 - ADPCM Sound Blaster Pro 2.6-bit - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_sbpro_4 - ADPCM Sound Blaster Pro 4-bit - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_swf - ADPCM Shockwave Flash - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_thp - ADPCM Nintendo THP - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_thp_le - ADPCM Nintendo THP (Little-Endian) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_vima - LucasArts VIMA audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_xa - ADPCM CDROM XA - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_xmd - ADPCM Konami XMD - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_yamaha - ADPCM Yamaha - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(adpcm_zork - ADPCM Zork - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(alac - ALAC (Apple Lossless Audio Codec) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(amr_nb - AMR-NB (Adaptive Multi-Rate NarrowBand) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(amr_wb - AMR-WB (Adaptive Multi-Rate WideBand) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(anull - Null audio codec - )" //
                            ,R"(apac - Marian's A-pac audio - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(ape - Monkey's Audio - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(atrac1 - ATRAC1 (Adaptive TRansform Acoustic Coding) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(atrac3 - ATRAC3 (Adaptive TRansform Acoustic Coding 3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(atrac3al - ATRAC3 AL (Adaptive TRansform Acoustic Coding 3 Advanced Lossless) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(atrac3p - ATRAC3+ (Adaptive TRansform Acoustic Coding 3+) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(atrac3pal - ATRAC3+ AL (Adaptive TRansform Acoustic Coding 3+ Advanced Lossless) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(atrac9 - ATRAC9 (Adaptive TRansform Acoustic Coding 9) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(avc - On2 Audio for Video Codec - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(binkaudio_dct - Bink Audio (DCT) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(binkaudio_rdft - Bink Audio (RDFT) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(bmv_audio - Discworld II BMV audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(bonk - Bonk audio - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(cbd2_dpcm - DPCM Cuberoot-Delta-Exact - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(comfortnoise - RFC 3389 Comfort Noise - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(cook - Cook / Cooker / Gecko (RealAudio G2) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(derf_dpcm - DPCM Xilam DERF - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dfpwm - DFPWM (Dynamic Filter Pulse Width Modulation) - (Lossy compression))" //
                            ,R"(dolby_e - Dolby E - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dsd_lsbf - DSD (Direct Stream Digital), least significant bit first - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dsd_lsbf_planar - DSD (Direct Stream Digital), least significant bit first, planar - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dsd_msbf - DSD (Direct Stream Digital), most significant bit first - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dsd_msbf_planar - DSD (Direct Stream Digital), most significant bit first, planar - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dsicinaudio - Delphine Software International CIN audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dss_sp - Digital Speech Standard - Standard Play mode (DSS SP) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(dst - DST (Direct Stream Transfer) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(dts - DCA (DTS Coherent Acoustics) - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(dvaudio - DV audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(eac3 - ATSC A/52B (AC-3, E-AC-3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(evrc - EVRC (Enhanced Variable Rate Codec) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(fastaudio - MobiClip FastAudio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(ftr - FTR Voice - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(g723_1 - G.723.1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(g729 - G.729 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(gremlin_dpcm - DPCM Gremlin - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(gsm - GSM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(gsm_ms - GSM Microsoft variant - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(hca - CRI HCA - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(hcom - HCOM Audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(iac - IAC (Indeo Audio Coder) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ilbc - iLBC (Internet Low Bitrate Codec) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(imc - IMC (Intel Music Coder) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(interplay_dpcm - DPCM Interplay - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(interplayacm - Interplay ACM - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mace3 - MACE (Macintosh Audio Compression/Expansion) 3:1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mace6 - MACE (Macintosh Audio Compression/Expansion) 6:1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(metasound - Voxware MetaSound - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(misc4 - Micronas SC-4 Audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mlp - MLP (Meridian Lossless Packing) - (Lossless compression))" //
                            ,R"(mp1 - MP1 (MPEG audio layer 1) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mp3 - MP3 (MPEG audio layer 3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mp3adu - ADU (Application Data Unit) MP3 (MPEG audio layer 3) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mp3on4 - MP3onMP4 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(mp4als - MPEG-4 Audio Lossless Coding (ALS) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(msnsiren - MSN Siren - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(musepack7 - Musepack SV7 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(musepack8 - Musepack SV8 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(nellymoser - Nellymoser Asao - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(opus - Opus (Opus Interactive Audio Codec) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(paf_audio - Amazing Studio Packed Animation File Audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(pcm_alaw - PCM A-law / G.711 A-law - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(pcm_bluray - PCM signed 16|20|24-bit big-endian for Blu-ray media - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_dvd - PCM signed 20|24-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f16le - PCM 16.8 floating point little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f24le - PCM 24.0 floating point little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f32be - PCM 32-bit floating point big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f32le - PCM 32-bit floating point little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f64be - PCM 64-bit floating point big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_f64le - PCM 64-bit floating point little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_lxf - PCM signed 20-bit little-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_mulaw - PCM mu-law / G.711 mu-law - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(pcm_s16be - PCM signed 16-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s16be_planar - PCM signed 16-bit big-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s16le - PCM signed 16-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s16le_planar - PCM signed 16-bit little-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24be - PCM signed 24-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24daud - PCM D-Cinema audio signed 24-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24le - PCM signed 24-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s24le_planar - PCM signed 24-bit little-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s32be - PCM signed 32-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s32le - PCM signed 32-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s32le_planar - PCM signed 32-bit little-endian planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s64be - PCM signed 64-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s64le - PCM signed 64-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s8 - PCM signed 8-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_s8_planar - PCM signed 8-bit planar - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_sga - PCM SGA - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u16be - PCM unsigned 16-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u16le - PCM unsigned 16-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u24be - PCM unsigned 24-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u24le - PCM unsigned 24-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u32be - PCM unsigned 32-bit big-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u32le - PCM unsigned 32-bit little-endian - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_u8 - PCM unsigned 8-bit - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(pcm_vidc - PCM Archimedes VIDC - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(qcelp - QCELP / PureVoice - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(qdm2 - QDesign Music Codec 2 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(qdmc - QDesign Music - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ra_144 - RealAudio 1.0 (14.4K) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ra_288 - RealAudio 2.0 (28.8K) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(ralf - RealAudio Lossless - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(rka - RKA (RK Audio) - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(roq_dpcm - DPCM id RoQ - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(s302m - SMPTE 302M - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(sbc - SBC (low-complexity subband codec) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(sdx2_dpcm - DPCM Squareroot-Delta-Exact - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(shorten - Shorten - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(sipr - RealAudio SIPR / ACELP.NET - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(siren - Siren - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(smackaudio - Smacker audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(sol_dpcm - DPCM Sol - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(sonic - Sonic - (Intra frame-only codec))" //
                            ,R"(speex - Speex - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(tak - TAK (Tom's lossless Audio Kompressor) - (Lossless compression))" //
                            ,R"(truehd - TrueHD - (Lossless compression))" //
                            ,R"(truespeech - DSP Group TrueSpeech - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(tta - TTA (True Audio) - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(twinvq - VQF TwinVQ - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(vmdaudio - Sierra VMD audio - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(vorbis - Vorbis - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wady_dpcm - DPCM Marble WADY - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wavarc - Waveform Archiver - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(wavesynth - Wave synthesis pseudo-codec - (Intra frame-only codec))" //
                            ,R"(wavpack - WavPack - (Intra frame-only codec) (Lossy compression) (Lossless compression))" //
                            ,R"(westwood_snd1 - Westwood Audio (SND1) - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wmalossless - Windows Media Audio Lossless - (Intra frame-only codec) (Lossless compression))" //
                            ,R"(wmapro - Windows Media Audio 9 Professional - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wmav1 - Windows Media Audio 1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wmav2 - Windows Media Audio 2 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(wmavoice - Windows Media Audio Voice - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(xan_dpcm - DPCM Xan - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(xma1 - Xbox Media Audio 1 - (Intra frame-only codec) (Lossy compression))" //
                            ,R"(xma2 - Xbox Media Audio 2 - (Intra frame-only codec) (Lossy compression))" //
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
                            ,R"(aac_latm)" //
                            ,R"(ac3)" //
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
                            ,R"(adpcm_g722)" //
                            ,R"(adpcm_g726)" //
                            ,R"(adpcm_g726le)" //
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
                            ,R"(adpcm_xmd)" //
                            ,R"(adpcm_yamaha)" //
                            ,R"(adpcm_zork)" //
                            ,R"(alac)" //
                            ,R"(amr_nb)" //
                            ,R"(amr_wb)" //
                            ,R"(anull)" //
                            ,R"(apac)" //
                            ,R"(ape)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(atrac1)" //
                            ,R"(atrac3)" //
                            ,R"(atrac3al)" //
                            ,R"(atrac3p)" //
                            ,R"(atrac3pal)" //
                            ,R"(atrac9)" //
                            ,R"(avc)" //
                            ,R"(binkaudio_dct)" //
                            ,R"(binkaudio_rdft)" //
                            ,R"(bmv_audio)" //
                            ,R"(bonk)" //
                            ,R"(cbd2_dpcm)" //
                            ,R"(comfortnoise)" //
                            ,R"(cook)" //
                            ,R"(derf_dpcm)" //
                            ,R"(dfpwm)" //
                            ,R"(dolby_e)" //
                            ,R"(dsd_lsbf)" //
                            ,R"(dsd_lsbf_planar)" //
                            ,R"(dsd_msbf)" //
                            ,R"(dsd_msbf_planar)" //
                            ,R"(dsicinaudio)" //
                            ,R"(dss_sp)" //
                            ,R"(dst)" //
                            ,R"(dts)" //
                            ,R"(dvaudio)" //
                            ,R"(eac3)" //
                            ,R"(evrc)" //
                            ,R"(fastaudio)" //
                            ,R"(flac)" //
                            ,R"(ftr)" //
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
                            ,R"(misc4)" //
                            ,R"(mlp)" //
                            ,R"(mp1)" //
                            ,R"(mp2)" //
                            ,R"(mp3)" //
                            ,R"(mp3adu)" //
                            ,R"(mp3on4)" //
                            ,R"(mp4als)" //
                            ,R"(msnsiren)" //
                            ,R"(musepack7)" //
                            ,R"(musepack8)" //
                            ,R"(nellymoser)" //
                            ,R"(opus)" //
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
                            ,R"(ra_144)" //
                            ,R"(ra_288)" //
                            ,R"(ralf)" //
                            ,R"(rka)" //
                            ,R"(roq_dpcm)" //
                            ,R"(s302m)" //
                            ,R"(sbc)" //
                            ,R"(sdx2_dpcm)" //
                            ,R"(shorten)" //
                            ,R"(sipr)" //
                            ,R"(siren)" //
                            ,R"(smackaudio)" //
                            ,R"(sol_dpcm)" //
                            ,R"(sonic)" //
                            ,R"(speex)" //
                            ,R"(tak)" //
                            ,R"(truehd)" //
                            ,R"(truespeech)" //
                            ,R"(tta)" //
                            ,R"(twinvq)" //
                            ,R"(vmdaudio)" //
                            ,R"(vorbis)" //
                            ,R"(wady_dpcm)" //
                            ,R"(wavarc)" //
                            ,R"(wavesynth)" //
                            ,R"(wavpack)" //
                            ,R"(westwood_snd1)" //
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

            QStringList CPreferences::availableSubtitleEncodingCodecsStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(ass - ASS (Advanced SSA) subtitle - )" //
                            ,R"(dvb_subtitle - DVB subtitles - )" //
                            ,R"(dvd_subtitle - DVD subtitles - )" //
                            ,R"(mov_text - MOV text - )" //
                            ,R"(subrip - SubRip subtitle - )" //
                            ,R"(text - raw UTF-8 text - )" //
                            ,R"(ttml - Timed Text Markup Language - )" //
                            ,R"(webvtt - WebVTT subtitle - )" //
                            ,R"(xsub - XSUB - )" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(ass)" //
                            ,R"(dvb_subtitle)" //
                            ,R"(dvd_subtitle)" //
                            ,R"(mov_text)" //
                            ,R"(subrip)" //
                            ,R"(text)" //
                            ,R"(ttml)" //
                            ,R"(webvtt)" //
                            ,R"(xsub)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableSubtitleDecodingCodecsStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(arib_caption - ARIB STD-B24 caption - )" //
                            ,R"(ass - ASS (Advanced SSA) subtitle - )" //
                            ,R"(dvb_subtitle - DVB subtitles - )" //
                            ,R"(dvb_teletext - DVB teletext - )" //
                            ,R"(dvd_subtitle - DVD subtitles - )" //
                            ,R"(eia_608 - EIA-608 closed captions - )" //
                            ,R"(hdmv_pgs_subtitle - HDMV Presentation Graphic Stream subtitles - )" //
                            ,R"(jacosub - JACOsub subtitle - )" //
                            ,R"(microdvd - MicroDVD subtitle - )" //
                            ,R"(mov_text - MOV text - )" //
                            ,R"(mpl2 - MPL2 subtitle - )" //
                            ,R"(pjs - PJS (Phoenix Japanimation Society) subtitle - )" //
                            ,R"(realtext - RealText subtitle - )" //
                            ,R"(sami - SAMI subtitle - )" //
                            ,R"(stl - Spruce subtitle format - )" //
                            ,R"(subrip - SubRip subtitle - )" //
                            ,R"(subviewer - SubViewer subtitle - )" //
                            ,R"(subviewer1 - SubViewer v1 subtitle - )" //
                            ,R"(text - raw UTF-8 text - )" //
                            ,R"(vplayer - VPlayer subtitle - )" //
                            ,R"(webvtt - WebVTT subtitle - )" //
                            ,R"(xsub - XSUB - )" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(arib_caption)" //
                            ,R"(ass)" //
                            ,R"(dvb_subtitle)" //
                            ,R"(dvb_teletext)" //
                            ,R"(dvd_subtitle)" //
                            ,R"(eia_608)" //
                            ,R"(hdmv_pgs_subtitle)" //
                            ,R"(jacosub)" //
                            ,R"(microdvd)" //
                            ,R"(mov_text)" //
                            ,R"(mpl2)" //
                            ,R"(pjs)" //
                            ,R"(realtext)" //
                            ,R"(sami)" //
                            ,R"(stl)" //
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

            QStringList CPreferences::availableVideoEncodersStatic( bool verbose ) const
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
                            ,R"(av1_amf - AMD AMF AV1 encoder (codec av1))" //
                            ,R"(av1_nvenc - NVIDIA NVENC av1 encoder (codec av1))" //
                            ,R"(av1_qsv - AV1 (Intel Quick Sync Video acceleration) (codec av1))" //
                            ,R"(avrp - Avid 1:1 10-bit RGB Packer)" //
                            ,R"(ayuv - Uncompressed packed MS 4:4:4:4)" //
                            ,R"(bitpacked - Bitpacked)" //
                            ,R"(bmp - BMP (Windows and OS/2 bitmap))" //
                            ,R"(cfhd - GoPro CineForm HD)" //
                            ,R"(cinepak - Cinepak)" //
                            ,R"(cljr - Cirrus Logic AccuPak)" //
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
                            ,R"(h264_amf - AMD AMF H.264 Encoder (codec h264))" //
                            ,R"(h264_mf - H264 via MediaFoundation (codec h264))" //
                            ,R"(h264_nvenc - NVIDIA NVENC H.264 encoder (codec h264))" //
                            ,R"(h264_qsv - H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (Intel Quick Sync Video acceleration) (codec h264))" //
                            ,R"(hdr - HDR (Radiance RGBE format) image)" //
                            ,R"(hevc_amf - AMD AMF HEVC encoder (codec hevc))" //
                            ,R"(hevc_mf - HEVC via MediaFoundation (codec hevc))" //
                            ,R"(hevc_nvenc - NVIDIA NVENC hevc encoder (codec hevc))" //
                            ,R"(hevc_qsv - HEVC (Intel Quick Sync Video acceleration) (codec hevc))" //
                            ,R"(huffyuv - Huffyuv / HuffYUV)" //
                            ,R"(jpeg2000 - JPEG 2000)" //
                            ,R"(jpegls - JPEG-LS)" //
                            ,R"(libaom-av1 - libaom AV1 (codec av1))" //
                            ,R"(libjxl - libjxl JPEG XL (codec jpegxl))" //
                            ,R"(libkvazaar - libkvazaar H.265 / HEVC (codec hevc))" //
                            ,R"(libopenh264 - OpenH264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264))" //
                            ,R"(libopenjpeg - OpenJPEG JPEG 2000 (codec jpeg2000))" //
                            ,R"(librav1e - librav1e AV1 (codec av1))" //
                            ,R"(libsvtav1 - SVT-AV1(Scalable Video Technology for AV1) encoder (codec av1))" //
                            ,R"(libtheora - libtheora Theora (codec theora))" //
                            ,R"(libvpx - libvpx VP8 (codec vp8))" //
                            ,R"(libvpx-vp9 - libvpx VP9 (codec vp9))" //
                            ,R"(libwebp - libwebp WebP image (codec webp))" //
                            ,R"(libwebp_anim - libwebp WebP image (codec webp))" //
                            ,R"(libx264 - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264))" //
                            ,R"(libx264rgb - libx264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 RGB (codec h264))" //
                            ,R"(libx265 - libx265 H.265 / HEVC (codec hevc))" //
                            ,R"(libxavs2 - libxavs2 AVS2-P2/IEEE1857.4 (codec avs2))" //
                            ,R"(libxvid - libxvidcore MPEG-4 part 2 (codec mpeg4))" //
                            ,R"(ljpeg - Lossless JPEG)" //
                            ,R"(magicyuv - MagicYUV video)" //
                            ,R"(mjpeg - MJPEG (Motion JPEG))" //
                            ,R"(mjpeg_qsv - MJPEG (Intel Quick Sync Video acceleration) (codec mjpeg))" //
                            ,R"(mpeg1video - MPEG-1 video)" //
                            ,R"(mpeg2_qsv - MPEG-2 video (Intel Quick Sync Video acceleration) (codec mpeg2video))" //
                            ,R"(mpeg2video - MPEG-2 video)" //
                            ,R"(mpeg4 - MPEG-4 part 2)" //
                            ,R"(msmpeg4 - MPEG-4 part 2 Microsoft variant version 3 (codec msmpeg4v3))" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2)" //
                            ,R"(msvideo1 - Microsoft Video-1)" //
                            ,R"(pam - PAM (Portable AnyMap) image)" //
                            ,R"(pbm - PBM (Portable BitMap) image)" //
                            ,R"(pcx - PC Paintbrush PCX image)" //
                            ,R"(pfm - PFM (Portable FloatMap) image)" //
                            ,R"(pgm - PGM (Portable GrayMap) image)" //
                            ,R"(pgmyuv - PGMYUV (Portable GrayMap YUV) image)" //
                            ,R"(phm - PHM (Portable HalfFloatMap) image)" //
                            ,R"(png - PNG (Portable Network Graphics) image)" //
                            ,R"(ppm - PPM (Portable PixelMap) image)" //
                            ,R"(prores - Apple ProRes)" //
                            ,R"(prores_aw - Apple ProRes (codec prores))" //
                            ,R"(prores_ks - Apple ProRes (iCodec Pro) (codec prores))" //
                            ,R"(qoi - QOI (Quite OK Image format) image)" //
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
                            ,R"(tiff - TIFF image)" //
                            ,R"(utvideo - Ut Video)" //
                            ,R"(v210 - Uncompressed 4:2:2 10-bit)" //
                            ,R"(v308 - Uncompressed packed 4:4:4)" //
                            ,R"(v408 - Uncompressed packed QT 4:4:4:4)" //
                            ,R"(v410 - Uncompressed 4:4:4 10-bit)" //
                            ,R"(vbn - Vizrt Binary Image)" //
                            ,R"(vc2 - SMPTE VC-2 (codec dirac))" //
                            ,R"(vnull - null video)" //
                            ,R"(vp9_qsv - VP9 video (Intel Quick Sync Video acceleration) (codec vp9))" //
                            ,R"(wbmp - WBMP (Wireless Application Protocol Bitmap) image)" //
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
                            ,R"(av1_amf)" //
                            ,R"(av1_nvenc)" //
                            ,R"(av1_qsv)" //
                            ,R"(avrp)" //
                            ,R"(ayuv)" //
                            ,R"(bitpacked)" //
                            ,R"(bmp)" //
                            ,R"(cfhd)" //
                            ,R"(cinepak)" //
                            ,R"(cljr)" //
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
                            ,R"(h264_amf)" //
                            ,R"(h264_mf)" //
                            ,R"(h264_nvenc)" //
                            ,R"(h264_qsv)" //
                            ,R"(hdr)" //
                            ,R"(hevc_amf)" //
                            ,R"(hevc_mf)" //
                            ,R"(hevc_nvenc)" //
                            ,R"(hevc_qsv)" //
                            ,R"(huffyuv)" //
                            ,R"(jpeg2000)" //
                            ,R"(jpegls)" //
                            ,R"(libaom-av1)" //
                            ,R"(libjxl)" //
                            ,R"(libkvazaar)" //
                            ,R"(libopenh264)" //
                            ,R"(libopenjpeg)" //
                            ,R"(librav1e)" //
                            ,R"(libsvtav1)" //
                            ,R"(libtheora)" //
                            ,R"(libvpx)" //
                            ,R"(libvpx-vp9)" //
                            ,R"(libwebp)" //
                            ,R"(libwebp_anim)" //
                            ,R"(libx264)" //
                            ,R"(libx264rgb)" //
                            ,R"(libx265)" //
                            ,R"(libxavs2)" //
                            ,R"(libxvid)" //
                            ,R"(ljpeg)" //
                            ,R"(magicyuv)" //
                            ,R"(mjpeg)" //
                            ,R"(mjpeg_qsv)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2_qsv)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpeg4)" //
                            ,R"(msmpeg4)" //
                            ,R"(msmpeg4v2)" //
                            ,R"(msvideo1)" //
                            ,R"(pam)" //
                            ,R"(pbm)" //
                            ,R"(pcx)" //
                            ,R"(pfm)" //
                            ,R"(pgm)" //
                            ,R"(pgmyuv)" //
                            ,R"(phm)" //
                            ,R"(png)" //
                            ,R"(ppm)" //
                            ,R"(prores)" //
                            ,R"(prores_aw)" //
                            ,R"(prores_ks)" //
                            ,R"(qoi)" //
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
                            ,R"(tiff)" //
                            ,R"(utvideo)" //
                            ,R"(v210)" //
                            ,R"(v308)" //
                            ,R"(v408)" //
                            ,R"(v410)" //
                            ,R"(vbn)" //
                            ,R"(vc2)" //
                            ,R"(vnull)" //
                            ,R"(vp9_qsv)" //
                            ,R"(wbmp)" //
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

            QStringList CPreferences::availableVideoDecodersStatic( bool verbose ) const
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
                            ,R"(av1 - Alliance for Open Media AV1)" //
                            ,R"(av1_cuvid - Nvidia CUVID AV1 decoder (codec av1))" //
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
                            ,R"(camstudio - CamStudio (codec cscd))" //
                            ,R"(camtasia - TechSmith Screen Capture Codec (codec tscc))" //
                            ,R"(cavs - Chinese AVS (Audio Video Standard) (AVS1-P2, JiZhun profile))" //
                            ,R"(cdgraphics - CD Graphics video)" //
                            ,R"(cdtoons - CDToons video)" //
                            ,R"(cdxl - Commodore CDXL video)" //
                            ,R"(cfhd - GoPro CineForm HD)" //
                            ,R"(cinepak - Cinepak)" //
                            ,R"(clearvideo - Iterated Systems ClearVideo)" //
                            ,R"(cljr - Cirrus Logic AccuPak)" //
                            ,R"(cllc - Canopus Lossless Codec)" //
                            ,R"(cpia - CPiA video format)" //
                            ,R"(cri - Cintel RAW)" //
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
                            ,R"(eacmv - Electronic Arts CMV video (codec cmv))" //
                            ,R"(eamad - Electronic Arts Madcow Video (codec mad))" //
                            ,R"(eatgq - Electronic Arts TGQ video (codec tgq))" //
                            ,R"(eatgv - Electronic Arts TGV video (codec tgv))" //
                            ,R"(eatqi - Electronic Arts TQI Video (codec tqi))" //
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
                            ,R"(h264_cuvid - Nvidia CUVID H264 decoder (codec h264))" //
                            ,R"(h264_qsv - H264 video (Intel Quick Sync Video acceleration) (codec h264))" //
                            ,R"(hap - Vidvox Hap)" //
                            ,R"(hdr - HDR (Radiance RGBE format) image)" //
                            ,R"(hevc - HEVC (High Efficiency Video Coding))" //
                            ,R"(hevc_cuvid - Nvidia CUVID HEVC decoder (codec hevc))" //
                            ,R"(hevc_qsv - HEVC video (Intel Quick Sync Video acceleration) (codec hevc))" //
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
                            ,R"(jpegls - JPEG-LS)" //
                            ,R"(jv - Bitmap Brothers JV video)" //
                            ,R"(kgv1 - Kega Game Video)" //
                            ,R"(kmvc - Karl Morton's video codec)" //
                            ,R"(lagarith - Lagarith lossless)" //
                            ,R"(libaom-av1 - libaom AV1 (codec av1))" //
                            ,R"(libdav1d - dav1d AV1 decoder by VideoLAN (codec av1))" //
                            ,R"(libdavs2 - libdavs2 AVS2-P2/IEEE1857.4 (codec avs2))" //
                            ,R"(libjxl - libjxl JPEG XL (codec jpegxl))" //
                            ,R"(libopenh264 - OpenH264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264))" //
                            ,R"(libopenjpeg - OpenJPEG JPEG 2000 (codec jpeg2000))" //
                            ,R"(libuavs3d - libuavs3d AVS3-P2/IEEE1857.10 (codec avs3))" //
                            ,R"(libvpx - libvpx VP8 (codec vp8))" //
                            ,R"(libvpx-vp9 - libvpx VP9 (codec vp9))" //
                            ,R"(loco - LOCO)" //
                            ,R"(lscr - LEAD Screen Capture)" //
                            ,R"(m101 - Matrox Uncompressed SD)" //
                            ,R"(magicyuv - MagicYUV video)" //
                            ,R"(mdec - Sony PlayStation MDEC (Motion DECoder))" //
                            ,R"(media100 - Media 100)" //
                            ,R"(mimic - Mimic)" //
                            ,R"(mjpeg - MJPEG (Motion JPEG))" //
                            ,R"(mjpeg_cuvid - Nvidia CUVID MJPEG decoder (codec mjpeg))" //
                            ,R"(mjpeg_qsv - MJPEG video (Intel Quick Sync Video acceleration) (codec mjpeg))" //
                            ,R"(mjpegb - Apple MJPEG-B)" //
                            ,R"(mmvideo - American Laser Games MM Video)" //
                            ,R"(mobiclip - MobiClip Video)" //
                            ,R"(motionpixels - Motion Pixels video)" //
                            ,R"(mpeg1_cuvid - Nvidia CUVID MPEG1VIDEO decoder (codec mpeg1video))" //
                            ,R"(mpeg1video - MPEG-1 video)" //
                            ,R"(mpeg2_cuvid - Nvidia CUVID MPEG2VIDEO decoder (codec mpeg2video))" //
                            ,R"(mpeg2_qsv - MPEG2VIDEO video (Intel Quick Sync Video acceleration) (codec mpeg2video))" //
                            ,R"(mpeg2video - MPEG-2 video)" //
                            ,R"(mpeg4 - MPEG-4 part 2)" //
                            ,R"(mpeg4_cuvid - Nvidia CUVID MPEG4 decoder (codec mpeg4))" //
                            ,R"(mpegvideo - MPEG-1 video (codec mpeg2video))" //
                            ,R"(msa1 - MS ATC Screen)" //
                            ,R"(mscc - Mandsoft Screen Capture Codec)" //
                            ,R"(msmpeg4 - MPEG-4 part 2 Microsoft variant version 3 (codec msmpeg4v3))" //
                            ,R"(msmpeg4v1 - MPEG-4 part 2 Microsoft variant version 1)" //
                            ,R"(msmpeg4v2 - MPEG-4 part 2 Microsoft variant version 2)" //
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
                            ,R"(phm - PHM (Portable HalfFloatMap) image)" //
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
                            ,R"(qoi - QOI (Quite OK Image format) image)" //
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
                            ,R"(theora - Theora)" //
                            ,R"(thp - Nintendo Gamecube THP video)" //
                            ,R"(tiertexseqvideo - Tiertex Limited SEQ video)" //
                            ,R"(tiff - TIFF image)" //
                            ,R"(tmv - 8088flex TMV)" //
                            ,R"(truemotion1 - Duck TrueMotion 1.0)" //
                            ,R"(truemotion2 - Duck TrueMotion 2.0)" //
                            ,R"(truemotion2rt - Duck TrueMotion 2.0 Real Time)" //
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
                            ,R"(vbn - Vizrt Binary Image)" //
                            ,R"(vc1 - SMPTE VC-1)" //
                            ,R"(vc1_cuvid - Nvidia CUVID VC1 decoder (codec vc1))" //
                            ,R"(vc1_qsv - VC1 video (Intel Quick Sync Video acceleration) (codec vc1))" //
                            ,R"(vc1image - Windows Media Video 9 Image v2)" //
                            ,R"(vcr1 - ATI VCR1)" //
                            ,R"(vmdvideo - Sierra VMD video)" //
                            ,R"(vmnc - VMware Screen Codec / VMware Video)" //
                            ,R"(vnull - null video)" //
                            ,R"(vp3 - On2 VP3)" //
                            ,R"(vp4 - On2 VP4)" //
                            ,R"(vp5 - On2 VP5)" //
                            ,R"(vp6 - On2 VP6)" //
                            ,R"(vp6a - On2 VP6 (Flash version, with alpha channel))" //
                            ,R"(vp6f - On2 VP6 (Flash version))" //
                            ,R"(vp7 - On2 VP7)" //
                            ,R"(vp8 - On2 VP8)" //
                            ,R"(vp8_cuvid - Nvidia CUVID VP8 decoder (codec vp8))" //
                            ,R"(vp8_qsv - VP8 video (Intel Quick Sync Video acceleration) (codec vp8))" //
                            ,R"(vp9 - Google VP9)" //
                            ,R"(vp9_cuvid - Nvidia CUVID VP9 decoder (codec vp9))" //
                            ,R"(vp9_qsv - VP9 video (Intel Quick Sync Video acceleration) (codec vp9))" //
                            ,R"(vqavideo - Westwood Studios VQA (Vector Quantized Animation) video (codec ws_vqa))" //
                            ,R"(vqc - ViewQuest VQC)" //
                            ,R"(wbmp - WBMP (Wireless Application Protocol Bitmap) image)" //
                            ,R"(wcmv - WinCAM Motion Video)" //
                            ,R"(webp - WebP image)" //
                            ,R"(wmv1 - Windows Media Video 7)" //
                            ,R"(wmv2 - Windows Media Video 8)" //
                            ,R"(wmv3 - Windows Media Video 9)" //
                            ,R"(wmv3image - Windows Media Video 9 Image)" //
                            ,R"(wnv1 - Winnov WNV1)" //
                            ,R"(wrapped_avframe - AVPacket to AVFrame passthrough)" //
                            ,R"(xan_wc3 - Wing Commander III / Xan)" //
                            ,R"(xan_wc4 - Wing Commander IV / Xxan)" //
                            ,R"(xbin - eXtended BINary text)" //
                            ,R"(xbm - XBM (X BitMap) image)" //
                            ,R"(xface - X-face image)" //
                            ,R"(xl - Miro VideoXL (codec vixl))" //
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
                            ,R"(av1)" //
                            ,R"(av1_cuvid)" //
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
                            ,R"(camstudio)" //
                            ,R"(camtasia)" //
                            ,R"(cavs)" //
                            ,R"(cdgraphics)" //
                            ,R"(cdtoons)" //
                            ,R"(cdxl)" //
                            ,R"(cfhd)" //
                            ,R"(cinepak)" //
                            ,R"(clearvideo)" //
                            ,R"(cljr)" //
                            ,R"(cllc)" //
                            ,R"(cpia)" //
                            ,R"(cri)" //
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
                            ,R"(eacmv)" //
                            ,R"(eamad)" //
                            ,R"(eatgq)" //
                            ,R"(eatgv)" //
                            ,R"(eatqi)" //
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
                            ,R"(h264_cuvid)" //
                            ,R"(h264_qsv)" //
                            ,R"(hap)" //
                            ,R"(hdr)" //
                            ,R"(hevc)" //
                            ,R"(hevc_cuvid)" //
                            ,R"(hevc_qsv)" //
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
                            ,R"(jpegls)" //
                            ,R"(jv)" //
                            ,R"(kgv1)" //
                            ,R"(kmvc)" //
                            ,R"(lagarith)" //
                            ,R"(libaom-av1)" //
                            ,R"(libdav1d)" //
                            ,R"(libdavs2)" //
                            ,R"(libjxl)" //
                            ,R"(libopenh264)" //
                            ,R"(libopenjpeg)" //
                            ,R"(libuavs3d)" //
                            ,R"(libvpx)" //
                            ,R"(libvpx-vp9)" //
                            ,R"(loco)" //
                            ,R"(lscr)" //
                            ,R"(m101)" //
                            ,R"(magicyuv)" //
                            ,R"(mdec)" //
                            ,R"(media100)" //
                            ,R"(mimic)" //
                            ,R"(mjpeg)" //
                            ,R"(mjpeg_cuvid)" //
                            ,R"(mjpeg_qsv)" //
                            ,R"(mjpegb)" //
                            ,R"(mmvideo)" //
                            ,R"(mobiclip)" //
                            ,R"(motionpixels)" //
                            ,R"(mpeg1_cuvid)" //
                            ,R"(mpeg1video)" //
                            ,R"(mpeg2_cuvid)" //
                            ,R"(mpeg2_qsv)" //
                            ,R"(mpeg2video)" //
                            ,R"(mpeg4)" //
                            ,R"(mpeg4_cuvid)" //
                            ,R"(mpegvideo)" //
                            ,R"(msa1)" //
                            ,R"(mscc)" //
                            ,R"(msmpeg4)" //
                            ,R"(msmpeg4v1)" //
                            ,R"(msmpeg4v2)" //
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
                            ,R"(phm)" //
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
                            ,R"(qoi)" //
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
                            ,R"(theora)" //
                            ,R"(thp)" //
                            ,R"(tiertexseqvideo)" //
                            ,R"(tiff)" //
                            ,R"(tmv)" //
                            ,R"(truemotion1)" //
                            ,R"(truemotion2)" //
                            ,R"(truemotion2rt)" //
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
                            ,R"(vbn)" //
                            ,R"(vc1)" //
                            ,R"(vc1_cuvid)" //
                            ,R"(vc1_qsv)" //
                            ,R"(vc1image)" //
                            ,R"(vcr1)" //
                            ,R"(vmdvideo)" //
                            ,R"(vmnc)" //
                            ,R"(vnull)" //
                            ,R"(vp3)" //
                            ,R"(vp4)" //
                            ,R"(vp5)" //
                            ,R"(vp6)" //
                            ,R"(vp6a)" //
                            ,R"(vp6f)" //
                            ,R"(vp7)" //
                            ,R"(vp8)" //
                            ,R"(vp8_cuvid)" //
                            ,R"(vp8_qsv)" //
                            ,R"(vp9)" //
                            ,R"(vp9_cuvid)" //
                            ,R"(vp9_qsv)" //
                            ,R"(vqavideo)" //
                            ,R"(vqc)" //
                            ,R"(wbmp)" //
                            ,R"(wcmv)" //
                            ,R"(webp)" //
                            ,R"(wmv1)" //
                            ,R"(wmv2)" //
                            ,R"(wmv3)" //
                            ,R"(wmv3image)" //
                            ,R"(wnv1)" //
                            ,R"(wrapped_avframe)" //
                            ,R"(xan_wc3)" //
                            ,R"(xan_wc4)" //
                            ,R"(xbin)" //
                            ,R"(xbm)" //
                            ,R"(xface)" //
                            ,R"(xl)" //
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

            QStringList CPreferences::availableAudioEncodersStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(aac - AAC (Advanced Audio Coding))" //
                            ,R"(aac_mf - AAC via MediaFoundation (codec aac))" //
                            ,R"(ac3 - ATSC A/52A (AC-3))" //
                            ,R"(ac3_fixed - ATSC A/52A (AC-3) (codec ac3))" //
                            ,R"(ac3_mf - AC3 via MediaFoundation (codec ac3))" //
                            ,R"(adpcm_adx - SEGA CRI ADX ADPCM)" //
                            ,R"(adpcm_argo - ADPCM Argonaut Games)" //
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
                            ,R"(anull - null audio)" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth))" //
                            ,R"(comfortnoise - RFC 3389 comfort noise generator)" //
                            ,R"(dfpwm - DFPWM1a audio)" //
                            ,R"(eac3 - ATSC A/52 E-AC-3)" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec))" //
                            ,R"(g722 - G.722 ADPCM (codec adpcm_g722))" //
                            ,R"(g723_1 - G.723.1)" //
                            ,R"(g726 - G.726 ADPCM (codec adpcm_g726))" //
                            ,R"(g726le - G.726 little endian ADPCM ("right-justified") (codec adpcm_g726le))" //
                            ,R"(libmp3lame - libmp3lame MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(libopencore_amrnb - OpenCORE AMR-NB (Adaptive Multi-Rate Narrow-Band) (codec amr_nb))" //
                            ,R"(libopus - libopus Opus (codec opus))" //
                            ,R"(libtwolame - libtwolame MP2 (MPEG audio layer 2) (codec mp2))" //
                            ,R"(libvorbis - libvorbis (codec vorbis))" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2))" //
                            ,R"(mp2fixed - MP2 fixed point (MPEG audio layer 2) (codec mp2))" //
                            ,R"(mp3_mf - MP3 via MediaFoundation (codec mp3))" //
                            ,R"(nellymoser - Nellymoser Asao)" //
                            ,R"(pcm_alaw - PCM A-law / G.711 A-law)" //
                            ,R"(pcm_bluray - PCM signed 16|20|24-bit big-endian for Blu-ray media)" //
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
                            ,R"(tta - TTA (True Audio))" //
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
                            ,R"(aac_mf)" //
                            ,R"(ac3)" //
                            ,R"(ac3_fixed)" //
                            ,R"(ac3_mf)" //
                            ,R"(adpcm_adx)" //
                            ,R"(adpcm_argo)" //
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
                            ,R"(anull)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(comfortnoise)" //
                            ,R"(dfpwm)" //
                            ,R"(eac3)" //
                            ,R"(flac)" //
                            ,R"(g722)" //
                            ,R"(g723_1)" //
                            ,R"(g726)" //
                            ,R"(g726le)" //
                            ,R"(libmp3lame)" //
                            ,R"(libopencore_amrnb)" //
                            ,R"(libopus)" //
                            ,R"(libtwolame)" //
                            ,R"(libvorbis)" //
                            ,R"(mp2)" //
                            ,R"(mp2fixed)" //
                            ,R"(mp3_mf)" //
                            ,R"(nellymoser)" //
                            ,R"(pcm_alaw)" //
                            ,R"(pcm_bluray)" //
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
                            ,R"(tta)" //
                            ,R"(wavpack)" //
                            ,R"(wmav1)" //
                            ,R"(wmav2)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableAudioDecodersStatic( bool verbose ) const
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
                            ,R"(adpcm_xmd - ADPCM Konami XMD)" //
                            ,R"(adpcm_yamaha - ADPCM Yamaha)" //
                            ,R"(adpcm_zork - ADPCM Zork)" //
                            ,R"(alac - ALAC (Apple Lossless Audio Codec))" //
                            ,R"(als - MPEG-4 Audio Lossless Coding (ALS) (codec mp4als))" //
                            ,R"(amrnb - AMR-NB (Adaptive Multi-Rate NarrowBand) (codec amr_nb))" //
                            ,R"(amrwb - AMR-WB (Adaptive Multi-Rate WideBand) (codec amr_wb))" //
                            ,R"(anull - null audio)" //
                            ,R"(apac - Marian's A-pac audio)" //
                            ,R"(ape - Monkey's Audio)" //
                            ,R"(aptx - aptX (Audio Processing Technology for Bluetooth))" //
                            ,R"(aptx_hd - aptX HD (Audio Processing Technology for Bluetooth))" //
                            ,R"(atrac1 - ATRAC1 (Adaptive TRansform Acoustic Coding))" //
                            ,R"(atrac3 - ATRAC3 (Adaptive TRansform Acoustic Coding 3))" //
                            ,R"(atrac3al - ATRAC3 AL (Adaptive TRansform Acoustic Coding 3 Advanced Lossless))" //
                            ,R"(atrac3plus - ATRAC3+ (Adaptive TRansform Acoustic Coding 3+) (codec atrac3p))" //
                            ,R"(atrac3plusal - ATRAC3+ AL (Adaptive TRansform Acoustic Coding 3+ Advanced Lossless) (codec atrac3pal))" //
                            ,R"(atrac9 - ATRAC9 (Adaptive TRansform Acoustic Coding 9))" //
                            ,R"(binkaudio_dct - Bink Audio (DCT))" //
                            ,R"(binkaudio_rdft - Bink Audio (RDFT))" //
                            ,R"(bmv_audio - Discworld II BMV audio)" //
                            ,R"(bonk - Bonk audio)" //
                            ,R"(cbd2_dpcm - DPCM Cuberoot-Delta-Exact)" //
                            ,R"(comfortnoise - RFC 3389 comfort noise generator)" //
                            ,R"(cook - Cook / Cooker / Gecko (RealAudio G2))" //
                            ,R"(dca - DCA (DTS Coherent Acoustics) (codec dts))" //
                            ,R"(derf_dpcm - DPCM Xilam DERF)" //
                            ,R"(dfpwm - DFPWM1a audio)" //
                            ,R"(dolby_e - Dolby E)" //
                            ,R"(dsd_lsbf - DSD (Direct Stream Digital), least significant bit first)" //
                            ,R"(dsd_lsbf_planar - DSD (Direct Stream Digital), least significant bit first, planar)" //
                            ,R"(dsd_msbf - DSD (Direct Stream Digital), most significant bit first)" //
                            ,R"(dsd_msbf_planar - DSD (Direct Stream Digital), most significant bit first, planar)" //
                            ,R"(dsicinaudio - Delphine Software International CIN audio)" //
                            ,R"(dss_sp - Digital Speech Standard - Standard Play mode (DSS SP))" //
                            ,R"(dst - DST (Digital Stream Transfer))" //
                            ,R"(dvaudio - Ulead DV Audio)" //
                            ,R"(eac3 - ATSC A/52B (AC-3, E-AC-3))" //
                            ,R"(evrc - EVRC (Enhanced Variable Rate Codec))" //
                            ,R"(fastaudio - MobiClip FastAudio)" //
                            ,R"(flac - FLAC (Free Lossless Audio Codec))" //
                            ,R"(ftr - FTR Voice)" //
                            ,R"(g722 - G.722 ADPCM (codec adpcm_g722))" //
                            ,R"(g723_1 - G.723.1)" //
                            ,R"(g726 - G.726 ADPCM (codec adpcm_g726))" //
                            ,R"(g726le - G.726 ADPCM little-endian (codec adpcm_g726le))" //
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
                            ,R"(libopencore_amrnb - OpenCORE AMR-NB (Adaptive Multi-Rate Narrow-Band) (codec amr_nb))" //
                            ,R"(libopencore_amrwb - OpenCORE AMR-WB (Adaptive Multi-Rate Wide-Band) (codec amr_wb))" //
                            ,R"(libopus - libopus Opus (codec opus))" //
                            ,R"(libvorbis - libvorbis (codec vorbis))" //
                            ,R"(mace3 - MACE (Macintosh Audio Compression/Expansion) 3:1)" //
                            ,R"(mace6 - MACE (Macintosh Audio Compression/Expansion) 6:1)" //
                            ,R"(metasound - Voxware MetaSound)" //
                            ,R"(misc4 - Micronas SC-4 Audio)" //
                            ,R"(mlp - MLP (Meridian Lossless Packing))" //
                            ,R"(mp1 - MP1 (MPEG audio layer 1))" //
                            ,R"(mp1float - MP1 (MPEG audio layer 1) (codec mp1))" //
                            ,R"(mp2 - MP2 (MPEG audio layer 2))" //
                            ,R"(mp2float - MP2 (MPEG audio layer 2) (codec mp2))" //
                            ,R"(mp3 - MP3 (MPEG audio layer 3))" //
                            ,R"(mp3adu - ADU (Application Data Unit) MP3 (MPEG audio layer 3))" //
                            ,R"(mp3adufloat - ADU (Application Data Unit) MP3 (MPEG audio layer 3) (codec mp3adu))" //
                            ,R"(mp3float - MP3 (MPEG audio layer 3) (codec mp3))" //
                            ,R"(mp3on4 - MP3onMP4)" //
                            ,R"(mp3on4float - MP3onMP4 (codec mp3on4))" //
                            ,R"(mpc7 - Musepack SV7 (codec musepack7))" //
                            ,R"(mpc8 - Musepack SV8 (codec musepack8))" //
                            ,R"(msnsiren - MSN Siren)" //
                            ,R"(nellymoser - Nellymoser Asao)" //
                            ,R"(on2avc - On2 Audio for Video Codec (codec avc))" //
                            ,R"(opus - Opus)" //
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
                            ,R"(ralf - RealAudio Lossless)" //
                            ,R"(real_144 - RealAudio 1.0 (14.4K) (codec ra_144))" //
                            ,R"(real_288 - RealAudio 2.0 (28.8K) (codec ra_288))" //
                            ,R"(rka - RKA (RK Audio))" //
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
                            ,R"(tak - TAK (Tom's lossless Audio Kompressor))" //
                            ,R"(truehd - TrueHD)" //
                            ,R"(truespeech - DSP Group TrueSpeech)" //
                            ,R"(tta - TTA (True Audio))" //
                            ,R"(twinvq - VQF TwinVQ)" //
                            ,R"(vmdaudio - Sierra VMD audio)" //
                            ,R"(vorbis - Vorbis)" //
                            ,R"(wady_dpcm - DPCM Marble WADY)" //
                            ,R"(wavarc - Waveform Archiver)" //
                            ,R"(wavesynth - Wave synthesis pseudo-codec)" //
                            ,R"(wavpack - WavPack)" //
                            ,R"(wmalossless - Windows Media Audio Lossless)" //
                            ,R"(wmapro - Windows Media Audio 9 Professional)" //
                            ,R"(wmav1 - Windows Media Audio 1)" //
                            ,R"(wmav2 - Windows Media Audio 2)" //
                            ,R"(wmavoice - Windows Media Audio Voice)" //
                            ,R"(ws_snd1 - Westwood Audio (SND1) (codec westwood_snd1))" //
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
                            ,R"(adpcm_xmd)" //
                            ,R"(adpcm_yamaha)" //
                            ,R"(adpcm_zork)" //
                            ,R"(alac)" //
                            ,R"(als)" //
                            ,R"(amrnb)" //
                            ,R"(amrwb)" //
                            ,R"(anull)" //
                            ,R"(apac)" //
                            ,R"(ape)" //
                            ,R"(aptx)" //
                            ,R"(aptx_hd)" //
                            ,R"(atrac1)" //
                            ,R"(atrac3)" //
                            ,R"(atrac3al)" //
                            ,R"(atrac3plus)" //
                            ,R"(atrac3plusal)" //
                            ,R"(atrac9)" //
                            ,R"(binkaudio_dct)" //
                            ,R"(binkaudio_rdft)" //
                            ,R"(bmv_audio)" //
                            ,R"(bonk)" //
                            ,R"(cbd2_dpcm)" //
                            ,R"(comfortnoise)" //
                            ,R"(cook)" //
                            ,R"(dca)" //
                            ,R"(derf_dpcm)" //
                            ,R"(dfpwm)" //
                            ,R"(dolby_e)" //
                            ,R"(dsd_lsbf)" //
                            ,R"(dsd_lsbf_planar)" //
                            ,R"(dsd_msbf)" //
                            ,R"(dsd_msbf_planar)" //
                            ,R"(dsicinaudio)" //
                            ,R"(dss_sp)" //
                            ,R"(dst)" //
                            ,R"(dvaudio)" //
                            ,R"(eac3)" //
                            ,R"(evrc)" //
                            ,R"(fastaudio)" //
                            ,R"(flac)" //
                            ,R"(ftr)" //
                            ,R"(g722)" //
                            ,R"(g723_1)" //
                            ,R"(g726)" //
                            ,R"(g726le)" //
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
                            ,R"(libopencore_amrnb)" //
                            ,R"(libopencore_amrwb)" //
                            ,R"(libopus)" //
                            ,R"(libvorbis)" //
                            ,R"(mace3)" //
                            ,R"(mace6)" //
                            ,R"(metasound)" //
                            ,R"(misc4)" //
                            ,R"(mlp)" //
                            ,R"(mp1)" //
                            ,R"(mp1float)" //
                            ,R"(mp2)" //
                            ,R"(mp2float)" //
                            ,R"(mp3)" //
                            ,R"(mp3adu)" //
                            ,R"(mp3adufloat)" //
                            ,R"(mp3float)" //
                            ,R"(mp3on4)" //
                            ,R"(mp3on4float)" //
                            ,R"(mpc7)" //
                            ,R"(mpc8)" //
                            ,R"(msnsiren)" //
                            ,R"(nellymoser)" //
                            ,R"(on2avc)" //
                            ,R"(opus)" //
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
                            ,R"(ralf)" //
                            ,R"(real_144)" //
                            ,R"(real_288)" //
                            ,R"(rka)" //
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
                            ,R"(tak)" //
                            ,R"(truehd)" //
                            ,R"(truespeech)" //
                            ,R"(tta)" //
                            ,R"(twinvq)" //
                            ,R"(vmdaudio)" //
                            ,R"(vorbis)" //
                            ,R"(wady_dpcm)" //
                            ,R"(wavarc)" //
                            ,R"(wavesynth)" //
                            ,R"(wavpack)" //
                            ,R"(wmalossless)" //
                            ,R"(wmapro)" //
                            ,R"(wmav1)" //
                            ,R"(wmav2)" //
                            ,R"(wmavoice)" //
                            ,R"(ws_snd1)" //
                            ,R"(xan_dpcm)" //
                            ,R"(xma1)" //
                            ,R"(xma2)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableSubtitleEncodersStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(ass - ASS (Advanced SubStation Alpha) subtitle)" //
                            ,R"(dvbsub - DVB subtitles (codec dvb_subtitle))" //
                            ,R"(dvdsub - DVD subtitles (codec dvd_subtitle))" //
                            ,R"(mov_text - 3GPP Timed Text subtitle)" //
                            ,R"(srt - SubRip subtitle (codec subrip))" //
                            ,R"(ssa - ASS (Advanced SubStation Alpha) subtitle (codec ass))" //
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
                             R"(ass)" //
                            ,R"(dvbsub)" //
                            ,R"(dvdsub)" //
                            ,R"(mov_text)" //
                            ,R"(srt)" //
                            ,R"(ssa)" //
                            ,R"(subrip)" //
                            ,R"(text)" //
                            ,R"(ttml)" //
                            ,R"(webvtt)" //
                            ,R"(xsub)" //
                        } );
                    return defaultValue;
                }
            }

            QStringList CPreferences::availableSubtitleDecodersStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(ass - ASS (Advanced SubStation Alpha) subtitle)" //
                            ,R"(cc_dec - Closed Caption (EIA-608 / CEA-708) (codec eia_608))" //
                            ,R"(dvbsub - DVB subtitles (codec dvb_subtitle))" //
                            ,R"(dvdsub - DVD subtitles (codec dvd_subtitle))" //
                            ,R"(jacosub - JACOsub subtitle)" //
                            ,R"(libaribb24 - libaribb24 ARIB STD-B24 caption decoder (codec arib_caption))" //
                            ,R"(libzvbi_teletextdec - Libzvbi DVB teletext decoder (codec dvb_teletext))" //
                            ,R"(microdvd - MicroDVD subtitle)" //
                            ,R"(mov_text - 3GPP Timed Text subtitle)" //
                            ,R"(mpl2 - MPL2 subtitle)" //
                            ,R"(pgssub - HDMV Presentation Graphic Stream subtitles (codec hdmv_pgs_subtitle))" //
                            ,R"(pjs - PJS subtitle)" //
                            ,R"(realtext - RealText subtitle)" //
                            ,R"(sami - SAMI subtitle)" //
                            ,R"(srt - SubRip subtitle (codec subrip))" //
                            ,R"(ssa - ASS (Advanced SubStation Alpha) subtitle (codec ass))" //
                            ,R"(stl - Spruce subtitle format)" //
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
                             R"(ass)" //
                            ,R"(cc_dec)" //
                            ,R"(dvbsub)" //
                            ,R"(dvdsub)" //
                            ,R"(jacosub)" //
                            ,R"(libaribb24)" //
                            ,R"(libzvbi_teletextdec)" //
                            ,R"(microdvd)" //
                            ,R"(mov_text)" //
                            ,R"(mpl2)" //
                            ,R"(pgssub)" //
                            ,R"(pjs)" //
                            ,R"(realtext)" //
                            ,R"(sami)" //
                            ,R"(srt)" //
                            ,R"(ssa)" //
                            ,R"(stl)" //
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

            NSABUtils::TCodecToEncoderDecoderMap CPreferences::getCodecToEncoderMapStatic() const
            {
                static auto defaultValue = NSABUtils::TCodecToEncoderDecoderMap(
                    {
                         {
                            NSABUtils::EFormatType::eVideo, std::unordered_multimap< QString, QString >
                             ( {
                                 { R"(a64_multi)", R"(a64multi)" } //
                                ,{ R"(av1)", R"(libaom-av1)" } //
                                ,{ R"(av1)", R"(librav1e)" } //
                                ,{ R"(av1)", R"(libsvtav1)" } //
                                ,{ R"(av1)", R"(av1_nvenc)" } //
                                ,{ R"(av1)", R"(av1_qsv)" } //
                                ,{ R"(av1)", R"(av1_amf)" } //
                                ,{ R"(a64_multi5)", R"(a64multi5)" } //
                                ,{ R"(avs2)", R"(libxavs2)" } //
                                ,{ R"(dirac)", R"(vc2)" } //
                                ,{ R"(flv1)", R"(flv)" } //
                                ,{ R"(h264)", R"(libx264)" } //
                                ,{ R"(h264)", R"(libx264rgb)" } //
                                ,{ R"(h264)", R"(libopenh264)" } //
                                ,{ R"(h264)", R"(h264_amf)" } //
                                ,{ R"(h264)", R"(h264_mf)" } //
                                ,{ R"(h264)", R"(h264_nvenc)" } //
                                ,{ R"(h264)", R"(h264_qsv)" } //
                                ,{ R"(hevc)", R"(libx265)" } //
                                ,{ R"(hevc)", R"(hevc_amf)" } //
                                ,{ R"(hevc)", R"(hevc_mf)" } //
                                ,{ R"(hevc)", R"(hevc_nvenc)" } //
                                ,{ R"(hevc)", R"(hevc_qsv)" } //
                                ,{ R"(hevc)", R"(libkvazaar)" } //
                                ,{ R"(jpeg2000)", R"(jpeg2000)" } //
                                ,{ R"(jpeg2000)", R"(libopenjpeg)" } //
                                ,{ R"(jpegxl)", R"(libjxl)" } //
                                ,{ R"(mjpeg)", R"(mjpeg)" } //
                                ,{ R"(mjpeg)", R"(mjpeg_qsv)" } //
                                ,{ R"(mpeg2video)", R"(mpeg2video)" } //
                                ,{ R"(mpeg2video)", R"(mpeg2_qsv)" } //
                                ,{ R"(prores)", R"(prores)" } //
                                ,{ R"(prores)", R"(prores_aw)" } //
                                ,{ R"(prores)", R"(prores_ks)" } //
                                ,{ R"(mpeg4)", R"(mpeg4)" } //
                                ,{ R"(mpeg4)", R"(libxvid)" } //
                                ,{ R"(msmpeg4v3)", R"(msmpeg4)" } //
                                ,{ R"(roq)", R"(roqvideo)" } //
                                ,{ R"(theora)", R"(libtheora)" } //
                                ,{ R"(vp8)", R"(libvpx)" } //
                                ,{ R"(vp9)", R"(libvpx-vp9)" } //
                                ,{ R"(vp9)", R"(vp9_qsv)" } //
                                ,{ R"(webp)", R"(libwebp_anim)" } //
                                ,{ R"(webp)", R"(libwebp)" } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eAudio, std::unordered_multimap< QString, QString >
                             ( {
                                 { R"(aac)", R"(aac)" } //
                                ,{ R"(aac)", R"(aac_mf)" } //
                                ,{ R"(ac3)", R"(ac3)" } //
                                ,{ R"(ac3)", R"(ac3_fixed)" } //
                                ,{ R"(ac3)", R"(ac3_mf)" } //
                                ,{ R"(adpcm_g726le)", R"(g726le)" } //
                                ,{ R"(adpcm_g722)", R"(g722)" } //
                                ,{ R"(vorbis)", R"(vorbis)" } //
                                ,{ R"(vorbis)", R"(libvorbis)" } //
                                ,{ R"(adpcm_g726)", R"(g726)" } //
                                ,{ R"(amr_nb)", R"(libopencore_amrnb)" } //
                                ,{ R"(dts)", R"(dca)" } //
                                ,{ R"(mp2)", R"(mp2)" } //
                                ,{ R"(mp2)", R"(mp2fixed)" } //
                                ,{ R"(mp2)", R"(libtwolame)" } //
                                ,{ R"(mp3)", R"(libmp3lame)" } //
                                ,{ R"(mp3)", R"(mp3_mf)" } //
                                ,{ R"(opus)", R"(opus)" } //
                                ,{ R"(opus)", R"(libopus)" } //
                                ,{ R"(ra_144)", R"(real_144)" } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eSubtitle, std::unordered_multimap< QString, QString >
                             ( {
                                 { R"(subrip)", R"(srt)" } //
                                ,{ R"(subrip)", R"(subrip)" } //
                                ,{ R"(ass)", R"(ssa)" } //
                                ,{ R"(ass)", R"(ass)" } //
                                ,{ R"(dvb_subtitle)", R"(dvbsub)" } //
                                ,{ R"(dvd_subtitle)", R"(dvdsub)" } //
                            } )
                        }
                    } );
                return defaultValue;
            }

            NSABUtils::TCodecToEncoderDecoderMap CPreferences::getCodecToDecoderMapStatic() const
            {
                static auto defaultValue = NSABUtils::TCodecToEncoderDecoderMap(
                    {
                         {
                            NSABUtils::EFormatType::eVideo, std::unordered_multimap< QString, QString >
                             ( {
                                 { R"(av1)", R"(libdav1d)" } //
                                ,{ R"(av1)", R"(libaom-av1)" } //
                                ,{ R"(av1)", R"(av1)" } //
                                ,{ R"(av1)", R"(av1_cuvid)" } //
                                ,{ R"(av1)", R"(av1_qsv)" } //
                                ,{ R"(cmv)", R"(eacmv)" } //
                                ,{ R"(avs2)", R"(libdavs2)" } //
                                ,{ R"(avs3)", R"(libuavs3d)" } //
                                ,{ R"(roq)", R"(roqvideo)" } //
                                ,{ R"(cscd)", R"(camstudio)" } //
                                ,{ R"(flv1)", R"(flv)" } //
                                ,{ R"(h264)", R"(h264)" } //
                                ,{ R"(h264)", R"(h264_qsv)" } //
                                ,{ R"(h264)", R"(libopenh264)" } //
                                ,{ R"(h264)", R"(h264_cuvid)" } //
                                ,{ R"(hevc)", R"(hevc)" } //
                                ,{ R"(hevc)", R"(hevc_qsv)" } //
                                ,{ R"(hevc)", R"(hevc_cuvid)" } //
                                ,{ R"(mjpeg)", R"(mjpeg)" } //
                                ,{ R"(mjpeg)", R"(mjpeg_cuvid)" } //
                                ,{ R"(mjpeg)", R"(mjpeg_qsv)" } //
                                ,{ R"(idcin)", R"(idcinvideo)" } //
                                ,{ R"(iff_ilbm)", R"(iff)" } //
                                ,{ R"(smackvideo)", R"(smackvid)" } //
                                ,{ R"(jpeg2000)", R"(jpeg2000)" } //
                                ,{ R"(jpeg2000)", R"(libopenjpeg)" } //
                                ,{ R"(vixl)", R"(xl)" } //
                                ,{ R"(mpeg1video)", R"(mpeg1video)" } //
                                ,{ R"(mpeg1video)", R"(mpeg1_cuvid)" } //
                                ,{ R"(jpegxl)", R"(libjxl)" } //
                                ,{ R"(mad)", R"(eamad)" } //
                                ,{ R"(mpeg2video)", R"(mpeg2video)" } //
                                ,{ R"(mpeg2video)", R"(mpegvideo)" } //
                                ,{ R"(mpeg2video)", R"(mpeg2_qsv)" } //
                                ,{ R"(mpeg2video)", R"(mpeg2_cuvid)" } //
                                ,{ R"(tgv)", R"(eatgv)" } //
                                ,{ R"(mpeg4)", R"(mpeg4)" } //
                                ,{ R"(mpeg4)", R"(mpeg4_cuvid)" } //
                                ,{ R"(ws_vqa)", R"(vqavideo)" } //
                                ,{ R"(msmpeg4v3)", R"(msmpeg4)" } //
                                ,{ R"(tgq)", R"(eatgq)" } //
                                ,{ R"(tqi)", R"(eatqi)" } //
                                ,{ R"(tscc)", R"(camtasia)" } //
                                ,{ R"(ulti)", R"(ultimotion)" } //
                                ,{ R"(vc1)", R"(vc1)" } //
                                ,{ R"(vc1)", R"(vc1_qsv)" } //
                                ,{ R"(vc1)", R"(vc1_cuvid)" } //
                                ,{ R"(vp8)", R"(vp8)" } //
                                ,{ R"(vp8)", R"(libvpx)" } //
                                ,{ R"(vp8)", R"(vp8_cuvid)" } //
                                ,{ R"(vp8)", R"(vp8_qsv)" } //
                                ,{ R"(vp9)", R"(vp9)" } //
                                ,{ R"(vp9)", R"(libvpx-vp9)" } //
                                ,{ R"(vp9)", R"(vp9_cuvid)" } //
                                ,{ R"(vp9)", R"(vp9_qsv)" } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eAudio, std::unordered_multimap< QString, QString >
                             ( {
                                 { R"(aac)", R"(aac)" } //
                                ,{ R"(aac)", R"(aac_fixed)" } //
                                ,{ R"(ac3)", R"(ac3)" } //
                                ,{ R"(ac3)", R"(ac3_fixed)" } //
                                ,{ R"(adpcm_g726le)", R"(g726le)" } //
                                ,{ R"(adpcm_g722)", R"(g722)" } //
                                ,{ R"(vorbis)", R"(vorbis)" } //
                                ,{ R"(vorbis)", R"(libvorbis)" } //
                                ,{ R"(adpcm_g726)", R"(g726)" } //
                                ,{ R"(amr_nb)", R"(amrnb)" } //
                                ,{ R"(amr_nb)", R"(libopencore_amrnb)" } //
                                ,{ R"(amr_wb)", R"(amrwb)" } //
                                ,{ R"(amr_wb)", R"(libopencore_amrwb)" } //
                                ,{ R"(atrac3p)", R"(atrac3plus)" } //
                                ,{ R"(atrac3pal)", R"(atrac3plusal)" } //
                                ,{ R"(avc)", R"(on2avc)" } //
                                ,{ R"(dts)", R"(dca)" } //
                                ,{ R"(mp1)", R"(mp1)" } //
                                ,{ R"(mp1)", R"(mp1float)" } //
                                ,{ R"(mp2)", R"(mp2)" } //
                                ,{ R"(mp2)", R"(mp2float)" } //
                                ,{ R"(mp3)", R"(mp3float)" } //
                                ,{ R"(mp3)", R"(mp3)" } //
                                ,{ R"(ra_288)", R"(real_288)" } //
                                ,{ R"(mp3adu)", R"(mp3adufloat)" } //
                                ,{ R"(mp3adu)", R"(mp3adu)" } //
                                ,{ R"(westwood_snd1)", R"(ws_snd1)" } //
                                ,{ R"(mp3on4)", R"(mp3on4float)" } //
                                ,{ R"(mp3on4)", R"(mp3on4)" } //
                                ,{ R"(mp4als)", R"(als)" } //
                                ,{ R"(musepack7)", R"(mpc7)" } //
                                ,{ R"(musepack8)", R"(mpc8)" } //
                                ,{ R"(opus)", R"(opus)" } //
                                ,{ R"(opus)", R"(libopus)" } //
                                ,{ R"(ra_144)", R"(real_144)" } //
                                ,{ R"(smackaudio)", R"(smackaud)" } //
                            } )
                        }
                        ,{
                            NSABUtils::EFormatType::eSubtitle, std::unordered_multimap< QString, QString >
                             ( {
                                 { R"(arib_caption)", R"(libaribb24)" } //
                                ,{ R"(ass)", R"(ssa)" } //
                                ,{ R"(ass)", R"(ass)" } //
                                ,{ R"(dvb_subtitle)", R"(dvbsub)" } //
                                ,{ R"(dvb_teletext)", R"(libzvbi_teletextdec)" } //
                                ,{ R"(dvd_subtitle)", R"(dvdsub)" } //
                                ,{ R"(eia_608)", R"(cc_dec)" } //
                                ,{ R"(hdmv_pgs_subtitle)", R"(pgssub)" } //
                                ,{ R"(subrip)", R"(srt)" } //
                                ,{ R"(subrip)", R"(subrip)" } //
                            } )
                        }
                    } );
                return defaultValue;
            }

            QStringList CPreferences::availableHWAccelsStatic( bool verbose ) const
            {
                if ( verbose )
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(cuda - Use the nVidia Cuda Video acceleration for video transcoding.)" //
                            ,R"(d3d11va - Use D3D11VA (DirectX Video Acceleration) API hardware acceleration.)" //
                            ,R"(dxva2 - Use DXVA2 (DirectX Video Acceleration) API hardware acceleration.)" //
                            ,R"(opencl - Use OpenCL hardware acceleration for API video transcoding.))" //
                            ,R"(qsv - Use the Intel QuickSync Video acceleration for video transcoding.)" //
                            ,R"(vulkan - Use the Vulkan hardware acceleration API for video transcoding.))" //
                        } );
                    return defaultValue;
                }
                else
                {
                    static auto defaultValue =
                        QStringList(
                        {
                             R"(cuda)" //
                            ,R"(d3d11va)" //
                            ,R"(dxva2)" //
                            ,R"(opencl)" //
                            ,R"(qsv)" //
                            ,R"(vulkan)" //
                        } );
                    return defaultValue;
                }
            }

        }
    }
}