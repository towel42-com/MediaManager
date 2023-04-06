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

#include "TranscodeGeneralSettings.h"
#include "Preferences/Core/Preferences.h"

#include "ui_TranscodeGeneralSettings.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CTranscodeGeneralSettings::CTranscodeGeneralSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CTranscodeGeneralSettings )
            {
                fImpl->setupUi( this );

                auto verbose = NPreferences::NCore::CPreferences::instance()->availableEncoderMediaFormats( true );
                auto terse = NPreferences::NCore::CPreferences::instance()->availableEncoderMediaFormats( false );

                Q_ASSERT( verbose.count() == terse.count() );
                for ( int ii = 0; ii < terse.count(); ++ii )
                {
                    fImpl->mediaFormats->addItem( verbose[ ii ], terse[ ii ] );
                }
            }

            CTranscodeGeneralSettings::~CTranscodeGeneralSettings()
            {
            }

            void CTranscodeGeneralSettings::load()
            {
                fImpl->convertMediaFormat->setChecked( NPreferences::NCore::CPreferences::instance()->getConvertMediaContainer() );

                auto pos = fImpl->mediaFormats->findData( NPreferences::NCore::CPreferences::instance()->getConvertMediaToContainer() );
                fImpl->mediaFormats->setCurrentIndex( pos );
                fImpl->forceTranscode->setChecked( NPreferences::NCore::CPreferences::instance()->getForceTranscode() );
            }

            void CTranscodeGeneralSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setConvertMediaToContainer( fImpl->mediaFormats->currentData().toString() );
                NPreferences::NCore::CPreferences::instance()->setConvertMediaContainer( fImpl->convertMediaFormat->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setForceTranscode( fImpl->forceTranscode->isChecked() );
            }
       }
    }
}