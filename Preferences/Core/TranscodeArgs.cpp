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

#include "Preferences.h"
#include "TranscodeNeeded.h"
#include "Core/LanguageInfo.h"
#include "SABUtils/MediaInfo.h"

#include <QTextCodec>
#include <QFile>
#include <QDebug>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NCore
        {
            QStringList CPreferences::getTranscodeArgs( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &srcName, const QString &destName, const std::list< NMediaManager::NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NMediaManager::NCore::SLanguageInfo, QString > > &subIdxFiles ) const
            {
                return getTranscodeArgs( mediaInfo, srcName, destName, srtFiles, subIdxFiles, {}, {} );
            }

            QStringList CPreferences::getHighBitrateTranscodeArgs( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &srcName, const QString &destName, const std::list< NMediaManager::NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NMediaManager::NCore::SLanguageInfo, QString > > &subIdxFiles ) const
            {
                auto kbps = getTargetBitrate( mediaInfo, true, false );
                return getTranscodeArgs( mediaInfo, srcName, destName, srtFiles, subIdxFiles, {}, kbps );
            }

            QStringList CPreferences::getHighResolutionTranscodeArgs( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &srcName, const QString &destName, const std::list< NMediaManager::NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NMediaManager::NCore::SLanguageInfo, QString > > &subIdxFiles ) const
            {
                return getTranscodeArgs( mediaInfo, srcName, destName, srtFiles, subIdxFiles, NSABUtils::CMediaInfo::k1080pResolution.fResolution, {} );
            }

            QStringList CPreferences::getTranscodeArgs( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &srcName, const QString &destName, const std::list< NMediaManager::NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NMediaManager::NCore::SLanguageInfo, QString > > &subIdxFiles, const std::optional< std::pair< int, int > > &resolution, const std::optional< uint64_t > &bitrate ) const
            {
                auto transcodeNeeded = STranscodeNeeded( mediaInfo, this );

                if ( !transcodeNeeded.transcodeNeeded() && srtFiles.empty() && subIdxFiles.empty() && !resolution.has_value() && !bitrate.has_value() )
                    return {};

                auto retVal = QStringList()   //
                              << "-hide_banner"
                              << "-y"   //
                              << "-fflags"
                              << "+genpts"   //
                    ;
                auto hwAccel = getTranscodeHWAccel();
                if ( !hwAccel.isEmpty() )
                {
                    retVal << "-hwaccel" << hwAccel;
                    retVal << "-hwaccel_output_format" << hwAccel   //
                        ;
                }

                retVal << "-i" << srcName;   //

                for ( auto &&ii : srtFiles )
                {
                    //retVal << "-sub_charenc" << "cp1252";
                    retVal << "-i" << ii.path();

                    //QFile file( ii.path() );
                    //file.open( QFile::ReadOnly );
                    //if ( file.isOpen() )
                    //{
                    //    auto bom = file.peek( 4 );
                    //    qDebug() << ii.path() << "BOM:" << bom;
                    //    auto codec = QTextCodec::codecForUtfText( bom, nullptr );
                    //    if ( codec )
                    //    {
                    //        retVal << "-sub_charenc"
                    //               << "cp1252";
                    //         // codec->name();
                    //    }
                    //}
                }
                for ( auto &&subIDXPair : subIdxFiles )
                {
                    retVal << "-f"
                           << "vobsub"   //  must set the filename
                           << "-sub_name" << subIDXPair.second   //
                           << "-i" << subIDXPair.first.path()   //
                        ;
                }

                retVal << "-map_metadata"
                       << "0"   //
                       << "-map_chapters"
                       << "0";

                if ( !resolution.has_value() && !bitrate.has_value() && ( transcodeNeeded.containerOnly() || !transcodeNeeded.transcodeNeeded() ) )
                {
                    // already HVEC but wrong container, just copy
                    retVal << "-map"
                           << "0:v?"
                           << "-c:v"
                           << "copy"   //
                           << "-map"
                           << "0:a?"
                           << "-c:a"
                           << "copy"   //
                        ;
                }
                else
                {
                    QString videoCodec;
                    if ( !transcodeNeeded.wrongVideoCodec() && !bitrate.has_value() && !resolution.has_value() )
                    {
                        videoCodec = "copy";
                    }
                    else
                    {
                        videoCodec = getTranscodeToVideoCodec();
                    }

                    retVal << "-map"
                           << "0:v?"   //
                           << "-c:v" << videoCodec   //
                        ;

                    uint64_t defaultAudioStreamBitrate = mediaInfo->getDefaultAudioBitRate();
                    if ( mediaInfo->numAudioStreams() )
                    {
                        auto audioFormat = transcodeNeeded.defaultAudioNotAAC51() ? "aac" : getTranscodeToAudioCodec();
                        auto defaultAudioStreamNum = mediaInfo->defaultAudioStream();
                        int currAudioStreamNum = 0;
                        if ( !transcodeNeeded.bitrateTooHigh() && !transcodeNeeded.wrongAudioCodec() && !transcodeNeeded.defaultAudioNotAAC51() )
                        {
                            retVal << "-map"
                                   << "0:a?"   //
                                   << "-c:a"
                                   << "copy";
                        }
                        else
                        {
                            // transcode or copy the default audio stream
                            // and put it in the front
                            retVal << "-map" << QString( "0:a:%1?" ).arg( defaultAudioStreamNum );   // map from the default stream
                            retVal << QString( "-c:a:%1" ).arg( currAudioStreamNum ) << audioFormat;   // add a copy from the original stream to the front (since some players ignore the disposition
                            retVal << QString( "-disposition:a:%1" ).arg( currAudioStreamNum ) << "default";   // mark it as the default
                            if ( transcodeNeeded.defaultAudioNotAAC51() )
                            {
                                auto numChannels = std::min( mediaInfo->audioChannelCount( defaultAudioStreamNum ), 6 );
                                retVal << QString( "-ac:a:%1" ).arg( currAudioStreamNum ) << QString::number( numChannels );   // convert it to 5.1
                                if ( numChannels > 2 )
                                    audioFormat += QString( " %1.1" ).arg( numChannels - 1 );
                                else if ( numChannels == 2 )
                                    audioFormat += " stereo";
                                else
                                    audioFormat += " mono";
                            }
                            if ( transcodeNeeded.bitrateTooHigh() && defaultAudioStreamBitrate )
                                retVal << "-b:a" << QString( "%1" ).arg( defaultAudioStreamBitrate ) << "-maxrate" << QString( "%1" ).arg( static_cast< uint64_t >( defaultAudioStreamBitrate * 1.1 ) ) << "-bufsize" << QString( "%1" ).arg( defaultAudioStreamBitrate / 2 );

                            retVal << QString( "-metadata:s:a:%1" ).arg( currAudioStreamNum ) << QString( R"(title=Transcoded Default Track #%1 from '%2' to '%3')" ).arg( defaultAudioStreamNum ).arg( mediaInfo->getMediaTag( defaultAudioStreamNum, NSABUtils::EMediaTags::eAudioCodecDisp ) ).arg( audioFormat );   // set the metadata

                            currAudioStreamNum++;

                            if ( !transcodeNeeded.bitrateTooHigh() && ( transcodeNeeded.wrongAudioCodec() || transcodeNeeded.defaultAudioNotAAC51() ) )
                            {
                                auto numAudioStreams = mediaInfo->numAudioStreams();
                                for ( int ii = 0; ii < numAudioStreams; ++ii )
                                {
                                    retVal << "-map" << QString( "0:a:%1?" ).arg( ii );   // map this from the original stream
                                    retVal << QString( "-c:a:%1" ).arg( currAudioStreamNum ) << "copy";   // just copy the audio as the new stream
                                    retVal << QString( "-disposition:a:%1" ).arg( currAudioStreamNum++ ) << "0";   // its not the default and stream number is the new stream number
                                }
                            }
                        }
                    }

                    bool isHEVC = mediaInfo->isHEVCCodec( videoCodec, NPreferences::NCore::CPreferences::instance()->getMediaFormats() );

                    if ( transcodeNeeded.wrongVideoCodec() || bitrate.has_value() || resolution.has_value() )
                    {
                        if ( resolution.has_value() )
                        {
                            auto currRes = mediaInfo->getResolution();
                            auto widthDiff = 1.0 * std::abs( currRes.first - resolution.value().first ) / ( 1.0 * resolution.value().first );
                            auto heightDiff = 1.0 * std::abs( currRes.second - resolution.value().second ) / ( 1.0 * resolution.value().second );

                            auto scale = QString( "scale%1=%2:%3" ).arg( hwAccel.isEmpty() ? "" : ( "_" + hwAccel ) );
                            if ( widthDiff > heightDiff )
                                scale = scale.arg( resolution.value().first ).arg( -1 );
                            else
                                scale = scale.arg( -1 ).arg( resolution.value().second );
                            retVal << "-vf" << scale;
                        }

                        if ( bitrate.has_value() || getUseTargetBitrate() )
                        {
                            uint64_t lclBitrate = getTargetBitrate( mediaInfo, true, false );
                            if ( bitrate.has_value() )
                                lclBitrate = bitrate.value() - ( defaultAudioStreamBitrate / 1000 );
                            retVal << "-b:v" << QString( "%1k" ).arg( lclBitrate ) << "-maxrate" << QString( "%1k" ).arg( static_cast< uint64_t >( lclBitrate * 1.1 ) ) << "-bufsize" << QString( "%1k" ).arg( lclBitrate / 2 );
                        }
                        else if ( isHEVC )
                        {
                            if ( getLosslessEncoding() )
                            {
                                retVal << "-x265-params"
                                       << "lossless=1";
                            }
                            else if ( getUseCRF() )
                                retVal << "-crf" << QString::number( getCRF() );

                            if ( getUsePreset() )
                                retVal << "-preset" << toString( getPreset() );
                            if ( getUseTune() )
                                retVal << "-tune" << toString( getTune() );
                            if ( getUseProfile() )
                                retVal << "-profile:v" << toString( getProfile() );
                        }
                        if ( isHEVC )
                            retVal << "-tag:v"
                                   << "hvc1";
                    }

                    if ( srtFiles.empty() && subIdxFiles.empty() && !transcodeNeeded.wrongVideoCodec() )   // meaning if we arent adding srt or idx files, and the video codec is correct, we dont have to worry about transcoding subtitles
                    {
                        retVal << "-map"
                               << "0:s?"   //
                               << "-c:s"
                               << "copy";
                    }
                    else
                    {
                        auto numSubtitleStreams = mediaInfo->numSubtitleStreams();
                        auto subtitleCodecs = mediaInfo->allSubtitleCodecs();
                        int subTitleStreamNum = 0;
                        for ( int ii = 0; ii < numSubtitleStreams; ++ii )
                        {
                            retVal << "-map" << QString( "0:s:%1?" ).arg( subTitleStreamNum );

                            auto currCodec = subtitleCodecs[ ii ].toLower();
                            auto subTitleCodec = QString( "copy" );
                            if ( isEncoderFormat( mediaInfo, "matroska" ) )
                            {
                                if ( ( currCodec == "ass" ) || ( currCodec == "srt" ) || ( currCodec == "ssa" ) || ( currCodec == "hdmv_pgs_subtitle" ) || ( currCodec == "subrip" ) || ( currCodec == "xsub" ) || ( currCodec == "dvdsub" ) )
                                    subTitleCodec = QString( "copy" );
                                else
                                    subTitleCodec = "srt";
                            }
                            else if ( isEncoderFormat( mediaInfo, "mp4" ) )
                            {
                                if ( currCodec == "mov_text" )
                                    subTitleCodec = QString( "copy" );
                                else
                                    subTitleCodec = "mov_text";
                            }
                            else if ( isEncoderFormat( mediaInfo, "mov" ) )
                            {
                                if ( currCodec == "mov_text" )
                                    subTitleCodec = QString( "copy" );
                                else
                                    subTitleCodec = "mov_text";
                            }
                            retVal << QString( "-c:s:%1" ).arg( subTitleStreamNum++ ) << subTitleCodec;
                        }

                        int fileNum = 1;
                        for ( auto &&srtFile : srtFiles )
                        {
                            retVal << "-map" << QString( "%1:0?" ).arg( fileNum++ )   //
                                   << "-map" << QString( "0:s:%1?" ).arg( subTitleStreamNum )   //
                                   << QString( "-metadata:s:s:%1" ).arg( subTitleStreamNum ) << QString( "language=%1" ).arg( srtFile.isoCode() )   //
                                   << QString( "-metadata:s:s:%1" ).arg( subTitleStreamNum ) << QString( "handler_name=%1" ).arg( srtFile.language() )   //
                                   << QString( "-metadata:s:s:%1" ).arg( subTitleStreamNum ) << QString( "title=%1" ).arg( srtFile.displayName() )   //
                                ;
                            subTitleStreamNum++;
                        }

                        for ( auto &&subIdxPair : subIdxFiles )
                        {
                            retVal << "-map" << QString( "%1:0?" ).arg( fileNum++ )   //
                                   << "-map" << QString( "0:s:%1?" ).arg( subTitleStreamNum )   //
                                   << QString( "-c:s:%1" ).arg( subTitleStreamNum ) << "copy" << QString( "-metadata:s:s:%1" ).arg( subTitleStreamNum ) << QString( "language=%1" ).arg( subIdxPair.first.isoCode() )   //
                                   << QString( "-metadata:s:s:%1" ).arg( subTitleStreamNum ) << QString( "handler_name=%1" ).arg( subIdxPair.first.language() )   //
                                   << QString( "-metadata:s:s:%1" ).arg( subTitleStreamNum ) << QString( "title=%1" ).arg( subIdxPair.first.displayName() )   //
                                ;
                            subTitleStreamNum++;
                        }
                    }
                }
                retVal << "-f" << getConvertMediaToContainer()   //
                       << destName;

                return retVal;
            }
        }
    }
}
