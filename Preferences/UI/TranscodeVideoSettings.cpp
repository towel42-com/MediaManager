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

#include "TranscodeVideoSettings.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/FFMpegFormats.h"

#include "ui_TranscodeVideoSettings.h"

#include <QIntValidator>
#include <QWidgetAction>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CTranscodeVideoSettings::CTranscodeVideoSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CTranscodeVideoSettings )
            {
                fImpl->setupUi( this );
            }

            CTranscodeVideoSettings::~CTranscodeVideoSettings()
            {
            }

            void CTranscodeVideoSettings::load()
            {
                fImpl->transcodeVideo->setChecked( NPreferences::NCore::CPreferences::instance()->getTranscodeVideo() );

                fImpl->onlyTranscodeVideoOnFormatChange->setChecked( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeVideoOnFormatChange() );
                fImpl->generateNon4kVideo->setChecked( NPreferences::NCore::CPreferences::instance()->getGenerateNon4kVideo() );

                fImpl->generateLowBitrateVideo->setChecked( NPreferences::NCore::CPreferences::instance()->getGenerateLowBitrateVideo() );
                fImpl->bitrateThreshold->setValue( NPreferences::NCore::CPreferences::instance()->getBitrateThresholdPercentage() );
            }

            void CTranscodeVideoSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setTranscodeVideo( fImpl->transcodeVideo->isChecked() );

                NPreferences::NCore::CPreferences::instance()->setOnlyTranscodeVideoOnFormatChange( fImpl->onlyTranscodeVideoOnFormatChange->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setGenerateNon4kVideo( fImpl->generateNon4kVideo->isChecked() );

                NPreferences::NCore::CPreferences::instance()->setGenerateLowBitrateVideo( fImpl->generateLowBitrateVideo->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setBitrateThresholdPercentage( fImpl->bitrateThreshold->value() );
            }
        }
    }
}