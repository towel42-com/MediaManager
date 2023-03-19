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

#include "FFMpegInfo.h"
#include "ui_FFMpegInfo.h"
#include "Preferences/Core/Preferences.h"

#include <QInputDialog>
#include <QProgressDialog>
namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CFFMpegInfo::CFFMpegInfo( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CFFMpegInfo )
            {
                fImpl->setupUi( this );

                connect(
                    fImpl->recomputeBtn, &QPushButton::clicked,
                    [ this ]()
                    {
                        QProgressDialog dlg( tr( "Loading Formats and Codecs" ), tr( "Cancel" ), 0, 0, this );
                        dlg.setAutoClose( false );
                        dlg.setAutoReset( false );
                        dlg.setWindowModality( Qt::WindowModal );
                        dlg.show();

                        NPreferences::NCore::CPreferences::instance()->recomputeSupportedFormats( &dlg );
                        if ( !dlg.wasCanceled() )
                            load();
                    } );

                fImpl->tabWidget->setCurrentIndex( 0 );
            }

            CFFMpegInfo::~CFFMpegInfo()
            {
            }

            void CFFMpegInfo::load()
            {
                loadInfo( fImpl->supportedFormats, fImpl->supportedFormatsGB, NPreferences::NCore::CPreferences::instance()->availableEncoderMediaFormats( true ) );

                loadInfo( fImpl->videoEncodingCodecs, fImpl->videoEncodingCodecsGB, NPreferences::NCore::CPreferences::instance()->availableVideoEncodingCodecs( true ) );
                loadInfo( fImpl->videoDecodingCodecs, fImpl->videoDecodingCodecsGB, NPreferences::NCore::CPreferences::instance()->availableVideoDecodingCodecs( true ) );
                loadInfo( fImpl->videoEncoders, fImpl->videoEncodersGB, NPreferences::NCore::CPreferences::instance()->availableVideoEncoders( true ) );
                loadInfo( fImpl->videoDecoders, fImpl->videoDecodersGB, NPreferences::NCore::CPreferences::instance()->availableVideoDecoders( true ) );
                loadInfo( fImpl->videoExtensions, fImpl->videoExtensionsGB, NPreferences::NCore::CPreferences::instance()->getVideoEncoderExtensions() );

                loadInfo( fImpl->audioEncodingCodecs, fImpl->audioEncodingCodecsGB, NPreferences::NCore::CPreferences::instance()->availableAudioEncodingCodecs( true ) );
                loadInfo( fImpl->audioDecodingCodecs, fImpl->audioDecodingCodecsGB, NPreferences::NCore::CPreferences::instance()->availableAudioDecodingCodecs( true ) );
                loadInfo( fImpl->audioEncoders, fImpl->audioEncodersGB, NPreferences::NCore::CPreferences::instance()->availableAudioEncoders( true ) );
                loadInfo( fImpl->audioDecoders, fImpl->audioDecodersGB, NPreferences::NCore::CPreferences::instance()->availableAudioDecoders( true ) );
                loadInfo( fImpl->audioExtensions, fImpl->audioExtensionsGB, NPreferences::NCore::CPreferences::instance()->getAudioEncoderExtensions() );

                loadInfo( fImpl->subtitleEncodingCodecs, fImpl->subtitleEncodingCodecsGB, NPreferences::NCore::CPreferences::instance()->availableSubtitleEncodingCodecs( true ) );
                loadInfo( fImpl->subtitleDecodingCodecs, fImpl->subtitleDecodingCodecsGB, NPreferences::NCore::CPreferences::instance()->availableSubtitleDecodingCodecs( true ) );
                loadInfo( fImpl->subtitleEncoders, fImpl->subtitleEncodersGB, NPreferences::NCore::CPreferences::instance()->availableSubtitleEncoders( true ) );
                loadInfo( fImpl->subtitleDecoders, fImpl->subtitleDecodersGB, NPreferences::NCore::CPreferences::instance()->availableSubtitleDecoders( true ) );
                loadInfo( fImpl->subtitleExtensions, fImpl->subtitleExtensionsGB, NPreferences::NCore::CPreferences::instance()->getSubtitleEncoderExtensions() );

                loadInfo( fImpl->hwAccels, fImpl->hwAccelsGB, NPreferences::NCore::CPreferences::instance()->availableHWAccels( true ) );
            }

            void CFFMpegInfo::save()
            {
            }

            void CFFMpegInfo::loadInfo( QListWidget *listWidget, QGroupBox *gb, QStringList info )
            {
                listWidget->clear();
                info.sort();
                for ( auto &&ii : info )
                {
                    new QListWidgetItem( ii, listWidget );
                }
                if ( gb )
                {
                    auto currText = gb->title();
                    auto pos = currText.indexOf( '(' );
                    if ( pos != -1 )
                        currText = currText.left( pos - 1 );

                    auto text = QString( "%1 (%2)" ).arg( currText ).arg( info.count() );
                    gb->setTitle( text );
                }
            }
        }
    }
}
