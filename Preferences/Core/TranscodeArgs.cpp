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

                if ( !transcodeNeeded.transcodeNeeded() && srtFiles.empty() && subIdxFiles.empty() )
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

                if ( bitrate.has_value() )
                    retVal << "-b:v" << QString( "%1k" ).arg( bitrate.value() );

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

                if ( !bitrate.has_value() && ( transcodeNeeded.formatOnly() || !transcodeNeeded.transcodeNeeded() ) )
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
                    if ( !transcodeNeeded.videoCodecTranscodeNeeded() && !bitrate.has_value() && !resolution.has_value() )
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

                    if ( !transcodeNeeded.audioTranscodeNeeded() && !transcodeNeeded.defaultAudioNotAAC51() )
                    {
                        retVal << "-map"
                               << "0:a?"   //
                               << "-c:a"
                               << "copy";
                    }
                    else
                    {
                        auto defaultStreamNum = mediaInfo->defaultAudioStream();

                        // transcode or copy the default audio stream
                        // and put it in the front
                        auto audioFormat = transcodeNeeded.defaultAudioNotAAC51() ? "aac" : getTranscodeToAudioCodec();
                        retVal << "-map" << QString( "0:a:%1?" ).arg( defaultStreamNum );     // map from the default stream
                        int currStreamNum = 0;
                        retVal << QString( "-c:a:%1" ).arg( currStreamNum ) << audioFormat;          // add a copy from the original stream to the front (since some players ignore the disposition
                        retVal << QString( "-disposition:a:%1" ).arg( currStreamNum ) << "default";  // mark it as the default
                        if ( transcodeNeeded.defaultAudioNotAAC51() )
                        {
                            auto numChannels = std::min( mediaInfo->audioChannelCount( defaultStreamNum ), 6 );
                            retVal << QString( "-ac:a:%1" ).arg( currStreamNum ) << QString::number( numChannels );             // convert it to 5.1
                            audioFormat += QString( " %1.1" ).arg( numChannels - 1 );
                        }
                        retVal << QString( "-metadata:s:a:%1" ).arg( currStreamNum ) << QString( R"(title="Transcoded Default Track #%1 from '%2' to '%3'")" ).arg( defaultStreamNum ).arg( mediaInfo->getMediaTag( defaultStreamNum, NSABUtils::EMediaTags::eAudioCodecDisp ) ).arg( audioFormat );   // set the metadata

                        currStreamNum++;
                        auto numAudioStreams = mediaInfo->numAudioStreams();
                        for ( int ii = 0; ii < numAudioStreams; ++ii )
                        {
                            retVal << "-map" << QString( "0:a:%1?" ).arg( ii );              // map this from the original stream
                            retVal << QString( "-c:a:%1" ).arg( currStreamNum ) << "copy";            // just copy the audio as the new stream
                            retVal << QString( "-disposition:a:%1" ).arg( currStreamNum++ ) << "0";   // its not the default and stream number is the new stream number
                        }
                    }

                    if ( transcodeNeeded.videoCodecTranscodeNeeded() && mediaInfo->isHEVCCodec( videoCodec, NPreferences::NCore::CPreferences::instance()->getMediaFormats() ) )
                    {
                        if ( !bitrate.has_value() )
                        {
                            if ( getLosslessEncoding() )
                            {
                                retVal << "-x265-params"
                                       << "lossless=1";
                            }
                            else if ( getUseCRF() )
                                retVal << "-crf" << QString::number( getCRF() );
                            else if ( getUseTargetBitrate() )
                                retVal << "-b:v" << QString( "%1k" ).arg( getTargetBitrate( mediaInfo, true, false ) );
                        }

                        if ( getUsePreset() )
                            retVal << "-preset" << toString( getPreset() );
                        if ( getUseTune() )
                            retVal << "-tune" << toString( getTune() );
                        if ( getUseProfile() )
                            retVal << "-profile:v" << toString( getProfile() );
                        retVal << "-tag:v"
                               << "hvc1";
                    }
                }

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

                retVal << "-f" << getConvertMediaToContainer()   //
                       << destName;

                (void)bitrate;
                (void)resolution;
                return retVal;
            }
        }
    }
}
