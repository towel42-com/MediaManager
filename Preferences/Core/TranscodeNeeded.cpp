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
                 TransCode Enabled | Is Format | Format Change | Only On Format || transcode
                         0              X          X                X           ||   0
                         1              1          X                X           ||   0

                         1              0          0                0           ||   1
                         1              0          0                1           ||   0
                         1              0          1                0           ||   1
                         1              0          1                1           ||   1

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
                fFormat = fVideo = fAudio = fMissingAAC = false;
                if ( !mediaInfo )
                    return;

                fFormat = prefs->getConvertMediaContainer() && !prefs->isEncoderFormat( mediaInfo, prefs->getConvertMediaToContainer() );
                fVideo = prefs->getTranscodeVideo() && !mediaInfo->hasVideoCodec( prefs->getTranscodeToVideoCodec(), prefs->getMediaFormats() ) && ( fFormat || !prefs->getOnlyTranscodeVideoOnFormatChange() );
                fMissingAAC = prefs->getTranscodeAudio() && prefs->getAddAACAudioCodec() && !mediaInfo->hasAACCodec( prefs->getMediaFormats(), 6 );
                fAudio = prefs->getTranscodeAudio()
                         && ( !mediaInfo->isCodec( "aac", prefs->getTranscodeToAudioCodec(), prefs->getMediaFormats() ) && !mediaInfo->hasAudioCodec( prefs->getTranscodeToAudioCodec(), prefs->getMediaFormats() )
                              && ( fFormat || !prefs->getOnlyTranscodeAudioOnFormatChange() ) );
            }

            STranscodeNeeded::STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo ) :
                STranscodeNeeded( mediaInfo, CPreferences::instance() )
            {
            }

            std::optional< QString > STranscodeNeeded::getFormatMessage() const
            {
                if ( formatChangeNeeded() )
                {
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using a %2 container</p>" )
                                   .arg( QFileInfo( fMediaInfo->fileName() ).fileName() )
                                   .arg( NPreferences::NCore::CPreferences::instance()->getConvertMediaToContainer() );
                    return msg;
                }
                return {};
            }

            std::optional< QString > STranscodeNeeded::getVideoCodecMessage() const
            {
                if ( videoTranscodeNeeded() )
                {
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using the '%2' video codec</p>" )
                                   .arg( QFileInfo( fMediaInfo->fileName() ).fileName() )
                                   .arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToVideoCodec() );
                    return msg;
                }
                return {};
            }

            std::optional< QString > STranscodeNeeded::getAudioCodecMessage() const
            {
                if ( addAACAudioCodec() )
                {
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> is missing the 'AAC 5.1' audio codec</p>" ).arg( QFileInfo( fMediaInfo->fileName() ).fileName() );
                    return msg;
                }

                if ( audioTranscodeNeeded() )
                {
                    auto msg = QObject::tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using the '%2' audio codec</p>" )
                                   .arg( QFileInfo( fMediaInfo->fileName() ).fileName() )
                                   .arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );
                    return msg;
                }

                return {};
            }

            QString STranscodeNeeded::getMessage( const QString &from, const QString &to ) const
            {
                QStringList msgs;
                msgs << QObject::tr( "Convert to file from '%1' => '%2'" ).arg( from ).arg( to );
                if ( formatChangeNeeded() )
                    msgs << QObject::tr( "Convert to Container: %1" ).arg( NPreferences::NCore::CPreferences::instance()->getConvertMediaToContainer() );
                if ( videoTranscodeNeeded() )
                    msgs << QObject::tr( "Transcode video to the %1 codec" ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToVideoCodec() );
                if ( addAACAudioCodec() )
                    msgs << QObject::tr( "Add the AAC 5.1 codec to audio" );
                if ( audioTranscodeNeeded() )
                    msgs << QObject::tr( "Transcode audio to %1 codec" ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );

                if ( msgs.length() > 2 )
                    msgs.back() = " and " + msgs.back();
                auto msg = msgs.join( ", " );
                return msg;
            }

            QString STranscodeNeeded::getProgressLabelHeader( const QString &from, const QString &to ) const
            {
                QStringList msgs;
                if ( formatChangeNeeded() )
                    msgs << QObject::tr( "Converting to Container: %1" ).arg( NPreferences::NCore::CPreferences::instance()->getConvertMediaToContainer() );
                if ( videoTranscodeNeeded() )
                    msgs << QObject::tr( "Transcode video to the %1 codec" ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToVideoCodec() );
                if ( addAACAudioCodec() )
                    msgs << QObject::tr( "Add the AAC 5.1 codec to audio" );
                if ( audioTranscodeNeeded() )
                    msgs << QObject::tr( "Transcode audio to %1 codec" ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );

                if ( msgs.length() > 2 )
                    msgs.back() = " and " + msgs.back();
                auto msg = msgs.join( ", " );

                msg += QObject::tr( "<ul><li>%2</li>to<li>%3</li></ul>" ).arg( from ).arg( to );

                return msg;
            }

        }
    }
}
