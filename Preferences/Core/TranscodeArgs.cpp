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
#include "TranscodeNeeded.h"
#include "Core/LanguageInfo.h"
#include "SABUtils/MediaInfo.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NCore
        {
            QStringList CPreferences::getTranscodeArgs( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &srcName, const QString &destName, const std::list< NMediaManager::NCore::SLanguageInfo > &srtFiles ) const
            {
                auto transcodeNeeded = STranscodeNeeded( mediaInfo, this );

                if ( !transcodeNeeded.transcodeNeeded() && srtFiles.empty() )
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
                    retVal   //
                        << "-hwaccel" << hwAccel   //
                        << "-hwaccel_output_format" << hwAccel   //
                        ;
                }

                retVal << "-i" << srcName;   //
                for ( auto &&ii : srtFiles )
                    retVal << "-i" << ii.path();

                retVal << "-map_metadata"
                       << "0"   //
                       << "-map_chapters"
                       << "0";

                if ( transcodeNeeded.formatOnly() || !transcodeNeeded.transcodeNeeded() )
                {
                    // already HVEC but wrong container, just copy
                    retVal << "-map" << "0:v?" << "-c:v" << "copy"   //
                           << "-map" << "0:a?" << "-c:a" << "copy"   //
                        ;
                }
                else
                {
                    QString videoCodec;
                    if ( !transcodeNeeded.videoTranscodeNeeded() )
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

                    if ( !transcodeNeeded.audioTranscodeNeeded() && !transcodeNeeded.addAACAudioCodec() )
                    {
                        retVal << "-map"
                               << "0:a?"   //
                               << "-c:a"
                               << "copy";
                    }
                    else
                    {
                        auto numAudioStreams = mediaInfo->numAudioStreams();
                        int streamNum = 0;
                        for ( int ii = 0; ii < numAudioStreams; ++ii )
                        {
                            retVal << "-map" << QString( "0:a:%1?" ).arg( ii );
                            if ( ii == 0 )   // transcode or copy the first audio stream
                            {
                                auto audioFormat = transcodeNeeded.addAACAudioCodec() ? "aac" : getTranscodeToAudioCodec();
                                retVal << QString( "-c:a:0" ) << audioFormat;
                                if ( transcodeNeeded.addAACAudioCodec() )
                                    retVal << "-ac:a:0"
                                           << "6";   // convert it to 5.1
                                retVal << QString( "-metadata:s:a:%1" ).arg( ii ) << QString( R"(title="Transcoded from Default Track")" ).arg( audioFormat );
                                streamNum++;
                                retVal << "-map" << QString( "0:a:%1?" ).arg( ii );
                            }
                            retVal << QString( "-c:a:%1" ).arg( streamNum++ ) << "copy";
                        }
                    }

                    if ( transcodeNeeded.videoTranscodeNeeded() && mediaInfo->isHEVCCodec( videoCodec, NPreferences::NCore::CPreferences::instance()->getMediaFormats() ) )
                    {
                        if ( getLosslessEncoding() )
                            retVal << "-x265-params"
                                   << "lossless=1";
                        if ( getUseExplicitCRF() )
                            retVal << "-crf" << QString::number( getExplicitCRF() );
                        if ( getUsePreset() )
                            retVal << "-preset" << toString( getPreset() );
                        if ( getUseTune() )
                            retVal << "-tune" << toString( getTune() );
                        if ( getUseProfile() )
                            retVal << "-profile:v" << toString( getProfile() );
                    }
                }

                auto numSubtitleStreams = mediaInfo->numSubtitleStreams();
                int subTitleStreamNum = 0;
                for ( int ii = 0; ii < numSubtitleStreams; ++ii )
                {
                    retVal << "-map" << QString( "0:s:%1?" ).arg( subTitleStreamNum );
                    retVal << QString( "-c:s:%1" ).arg( subTitleStreamNum++ ) << ( !isEncoderFormat( mediaInfo, "matroska" ) ? "srt" : "copy" );   // if the source is NOT MKV make sure the subtitles are SRT, otherwise just copy
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

                retVal << "-f" << getConvertMediaToContainer()   //
                       << destName;

                return retVal;
            }
        }
    }
}
