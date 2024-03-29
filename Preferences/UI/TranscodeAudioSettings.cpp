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

#include "TranscodeAudioSettings.h"
#include "Preferences/Core/Preferences.h"
#include "ui_TranscodeAudioSettings.h"

#include "SABUtils/UtilityModels.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CTranscodeAudioSettings::CTranscodeAudioSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CTranscodeAudioSettings )
            {
                fImpl->setupUi( this );

                fVerbose = NPreferences::NCore::CPreferences::instance()->availableAudioEncoders( true );
                fTerse = NPreferences::NCore::CPreferences::instance()->availableAudioEncoders( false );

                Q_ASSERT( fVerbose.count() == fTerse.count() );
                for ( int ii = 0; ii < fTerse.count(); ++ii )
                {
                    fImpl->audioCodec->addItem( fVerbose[ ii ], fTerse[ ii ] );
                }

                connect(
                    fImpl->addAACAudioCodec, &QCheckBox::toggled,
                    [ this ]() 
                    {
                        fImpl->onlyTranscodeAudioOnFormatChange->setEnabled( !fImpl->addAACAudioCodec->isChecked() );
                        fImpl->audioCodec->setEnabled( !fImpl->addAACAudioCodec->isChecked() );
                    } );
            }

            CTranscodeAudioSettings::~CTranscodeAudioSettings()
            {
            }

            void CTranscodeAudioSettings::load()
            {
                fImpl->transcodeAudio->setChecked( NPreferences::NCore::CPreferences::instance()->getTranscodeAudio() );

                auto pos = fImpl->audioCodec->findData( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );
                fImpl->audioCodec->setCurrentIndex( pos );

                fImpl->onlyTranscodeAudioOnFormatChange->setChecked( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeAudioOnFormatChange() );
                fImpl->addAACAudioCodec->setChecked( !NPreferences::NCore::CPreferences::instance()->getAddAACAudioCodec() );
                fImpl->addAACAudioCodec->setChecked( NPreferences::NCore::CPreferences::instance()->getAddAACAudioCodec() );
            }

            void CTranscodeAudioSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setTranscodeAudio( fImpl->transcodeAudio->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setTranscodeToAudioCodec( fImpl->audioCodec->currentData().toString() );
                NPreferences::NCore::CPreferences::instance()->setOnlyTranscodeAudioOnFormatChange( fImpl->onlyTranscodeAudioOnFormatChange->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setAddAACAudioCodec( fImpl->addAACAudioCodec->isChecked() );
            }
       }
    }
}