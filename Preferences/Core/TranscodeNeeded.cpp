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

#include "TranscodeNeeded.h"
#include "Preferences.h"
#include "SABUtils/MediaInfo.h"

#include <QFileInfo>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NCore
        {
            /*
            * 
            *   VIDEO and Audio with "Add ACC" disabled
                 TransCode Enabled | Is Format | Format Change | Only On Format Wrong || transcode
                         0              X          X                X                 ||   0
                         1              1          X                1                 ||   0

                         1              0          0                0                 ||   1
                         1              0          0                1                 ||   0
                         1              0          1                0                 ||   1
                         1              0          1                1                 ||   1

                Audio with Add ACC Enabled
                 TransCode Enabled | Has AAC   | Format Change | Add AAC        || transcode
                         0              X          X                X           ||   0
                         1              1          X                X           ||   0

                         1              0          X                0           ||   0
                         1              0          X                1           ||   1

            */

            STranscodeNeeded::STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const CPreferences *prefs ) :
                fMediaInfo( mediaInfo )
            {
                fWrongContainer = fWrongVideoCodec = fBitrateTooHigh = fWrongAudioCodec = fDefaultAudioNotAAC = false;
                if ( !mediaInfo || !mediaInfo->aOK() || mediaInfo->isQueued() )
                    return;

                fWrongContainer = prefs->getConvertMediaContainer() && !prefs->isEncoderFormat( mediaInfo, prefs->getConvertMediaToContainer() );
                fWrongVideoCodec = !mediaInfo->hasVideoCodec( prefs->getTranscodeToVideoCodec(), prefs->getMediaFormats() ) && ( fWrongContainer || !prefs->getOnlyTranscodeVideoOnFormatChange() );

                if ( prefs->getGenerateLowBitrateVideo() )
                {
                    auto averageBitrateTarget = prefs->getTargetBitrate( fMediaInfo, false, true );
                    fBitrateTooHigh = mediaInfo->getOverallBitRate() > averageBitrateTarget;
                }

                if ( prefs->getGenerateNon4kVideo() )
                {
                    fVideoResolutionTooHigh = mediaInfo->isGreaterThanHDResolution();
                }

                if ( !prefs->getTranscodeVideo() )
                    fWrongVideoCodec = fBitrateTooHigh = fVideoResolutionTooHigh = false;

                fDefaultAudioNotAAC = prefs->getTranscodeAudio() && ( mediaInfo->numAudioStreams() != 0 ) && prefs->getAddAACAudioCodec() && !mediaInfo->isDefaultAudioCodecAAC( prefs->getMediaFormats(), 6 );
                fWrongAudioCodec = prefs->getTranscodeAudio() && ( mediaInfo->numAudioStreams() != 0 ) && ( ( !mediaInfo->isCodec( "aac", prefs->getTranscodeToAudioCodec(), prefs->getMediaFormats() ) && !mediaInfo->isDefaultAudioCodec( prefs->getTranscodeToAudioCodec(), prefs->getMediaFormats() ) && ( fWrongContainer || !prefs->getOnlyTranscodeAudioOnFormatChange() ) ) );
            }

            STranscodeNeeded::STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo ) :
                STranscodeNeeded( mediaInfo, CPreferences::instance() )
            {
            }

            bool STranscodeNeeded::isLoaded() const
            {
                return fMediaInfo && fMediaInfo->aOK() && !fMediaInfo->isQueued();
            }

            std::optional< QString > STranscodeNeeded::getFormatMessage() const
            {
                if ( wrongContainer() )
                {
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using a %2 container</p>" ).arg( QFileInfo( fMediaInfo->fileName() ).fileName() ).arg( NPreferences::NCore::CPreferences::instance()->getConvertMediaToContainer() );
                    return msg;
                }
                return {};
            }

            std::optional< QString > STranscodeNeeded::getVideoCodecMessage() const
            {
                if ( wrongVideoCodec() )
                {
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using the '%2' video codec</p>" ).arg( QFileInfo( fMediaInfo->fileName() ).fileName() ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToVideoCodec() );
                    return msg;
                }
                return {};
            }

            std::optional< QString > STranscodeNeeded::getBitrateMessage() const
            {
                if ( bitrateTooHigh() )
                {
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> overall bit rate is higher than '%2'</p>" ).arg( QFileInfo( fMediaInfo->fileName() ).fileName() ).arg( NPreferences::NCore::CPreferences::instance()->getTargetBitrateDisplayString( fMediaInfo ) );
                    return msg;
                }
                return {};
            }

            std::optional< QString > STranscodeNeeded::getVideoResolutionMessage() const
            {
                if ( resolutionTooHigh() )
                {
                    auto resolution = fMediaInfo->getResolution();
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> resolution higher than HD resolution (1920x1080)</p>" ).arg( QFileInfo( fMediaInfo->fileName() ).fileName() );
                    return msg;
                }
                return {};
            }

            std::optional< QString > STranscodeNeeded::getAudioCodecMessage() const
            {
                if ( bitrateTooHigh() )
                {
                    auto targetAudioCodec = defaultAudioNotAAC51() ? "AAC 5.1" : ( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b>'s overall bitrate is too high, removing all audio streams except the default an transcoding the audio track to the '%2' audio codec</p>" ).arg( QFileInfo( fMediaInfo->fileName() ).fileName() ).arg( targetAudioCodec );
                    return msg;
                }

                if ( defaultAudioNotAAC51() || wrongAudioCodec() )
                {
                    auto targetAudioCodec = defaultAudioNotAAC51() ? "AAC 5.1" : ( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b>'s default audio track is not the '%2' audio codec</p>" ).arg( QFileInfo( fMediaInfo->fileName() ).fileName() ).arg( targetAudioCodec );
                    return msg;
                }

                return {};
            }

            QStringList STranscodeNeeded::getActions() const
            {
                QStringList actions;

                if ( wrongContainer() )
                    actions << QObject::tr( "Convert to Container: '%1'" ).arg( NPreferences::NCore::CPreferences::instance()->getConvertMediaToContainer() );
                if ( wrongVideoCodec() )
                    actions << QObject::tr( "Transcode video to the '%1' codec" ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToVideoCodec() );
                if ( defaultAudioNotAAC51() || wrongAudioCodec() )
                    actions << QObject::tr( "Transcode the default audio stream to the '%1' codec" ).arg( defaultAudioNotAAC51() ? "AAC 5.1" : NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );

                return actions;
            }

            QStringList STranscodeNeeded::getHighBitrateAction() const
            {
                QStringList actions;

                if ( bitrateTooHigh() )
                {
                    actions << QObject::tr( "Transcode to an average overall bitrate of %1, removing all audio except default stream" ).arg( NPreferences::NCore::CPreferences::instance()->getTargetBitrateDisplayString( fMediaInfo ) );
                    actions << getActions();
                }

                return actions;
            }

            QStringList STranscodeNeeded::getHighResolutionAction() const
            {
                QStringList actions;

                if ( resolutionTooHigh() )
                {
                    actions << QObject::tr( "Transcode video to a resolution of 1080p" );
                    actions << getActions();
                }

                return actions;
            }

            QString STranscodeNeeded::getProgressLabelHeader( const QString &from, const QStringList &mergedFiles, const QString &to, const QStringList & actions ) const
            {
                if ( actions.isEmpty() )
                    return {};

                QString msg;
                msg += QObject::tr( "<b>Source:</b><ul><li>%1</li></ul><b>Output:</b><ul><li>%2</li>" ).arg( from ).arg( to );
                for ( auto &&ii : mergedFiles )
                {
                    msg += QString( "<li>%1</li>\n" ).arg( ii );
                }
                msg += "</ul>";

                if ( !actions.isEmpty() )
                {
                    msg += QObject::tr( "<b>Transcoding Actions:</b>" );
                    msg += "<ul>";
                    for ( auto &&ii : actions )
                        msg += QString( "<li>%1</li>\n" ).arg( ii );
                    msg += "<ul>";
                }

                return msg;
            }

            QString STranscodeNeeded::getProgressLabelHeader( const QString &from, const QStringList &mergedFiles, const QString &to ) const
            {
                return getProgressLabelHeader( from, mergedFiles, to, getActions() );
            }

            QString STranscodeNeeded::getHighResolutionProgressLabelHeader( const QString &from, const QStringList &mergedFiles, const QString &to ) const
            {
                return getProgressLabelHeader( from, mergedFiles, to, getHighResolutionAction() );
            }

            QString STranscodeNeeded::getHighBitrateProgressLabelHeader( const QString &from, const QStringList &mergedFiles, const QString &to ) const
            {
                return getProgressLabelHeader( from, mergedFiles, to, getHighBitrateAction() );
            }
        }
    }
}
