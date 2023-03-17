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

#include "MakeMKVVideoSettings.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/MediaInfo.h"

#include "ui_MakeMKVVideoSettings.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CMakeMKVVideoSettings::CMakeMKVVideoSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CMakeMKVVideoSettings )
            {
                fImpl->setupUi( this );
                connect( fImpl->losslessTranscoding, &QCheckBox::toggled, this, &CMakeMKVVideoSettings::slotLosslessChanged );
                connect( fImpl->useCRF, &QGroupBox::toggled, this, &CMakeMKVVideoSettings::slotCRFChanged );

                connect( fImpl->useExplicitCRF, &QCheckBox::toggled, this, &CMakeMKVVideoSettings::slotUseExplicitCRFChanged );
                connect( fImpl->usePreset, &QCheckBox::toggled, this, &CMakeMKVVideoSettings::slotUsePresetChanged );
                connect( fImpl->useTune, &QCheckBox::toggled, this, &CMakeMKVVideoSettings::slotUseTuneChanged );
                connect( fImpl->useProfile, &QCheckBox::toggled, this, &CMakeMKVVideoSettings::slotUseProfileChanged );
                connect( fImpl->videoCodec, qOverload< int >( &QComboBox::currentIndexChanged ), this, &CMakeMKVVideoSettings::slotCodecChanged );
                connect( fImpl->hwAccel, qOverload< int >( &QComboBox::currentIndexChanged ), this, &CMakeMKVVideoSettings::slotHWAccelChanged );

                fImpl->intelGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasIntelGPU() );
                fImpl->nvidiaGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasNVidiaGPU() );
                fImpl->amdGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasAMDGPU() );

                connect( fImpl->intelGPUTranscode, &QRadioButton::toggled, this, &CMakeMKVVideoSettings::slotExplicitCodecChanged );
                connect( fImpl->nvidiaGPUTranscode, &QRadioButton::toggled, this, &CMakeMKVVideoSettings::slotExplicitCodecChanged );
                connect( fImpl->amdGPUTranscode, &QRadioButton::toggled, this, &CMakeMKVVideoSettings::slotExplicitCodecChanged );
                connect( fImpl->softwareTranscode, &QRadioButton::toggled, this, &CMakeMKVVideoSettings::slotExplicitCodecChanged );

                fVerboseEncoders = NPreferences::NCore::CPreferences::instance()->availableVideoEncoders( true );
                fTerseEncoders = NPreferences::NCore::CPreferences::instance()->availableVideoEncoders( false );
                Q_ASSERT( fVerboseEncoders.count() == fTerseEncoders.count() );
                for ( int ii = 0; ii < fTerseEncoders.count(); ++ii )
                {
                    fImpl->videoCodec->addItem( fVerboseEncoders[ ii ], fTerseEncoders[ ii ] );
                }

                fVerboseHWAccels = NPreferences::NCore::CPreferences::instance()->availableHWAccels( true );
                fTerseHWAccels = NPreferences::NCore::CPreferences::instance()->availableHWAccels( false );

                Q_ASSERT( fTerseHWAccels.count() == fVerboseHWAccels.count() );
                for ( int ii = 0; ii < fVerboseHWAccels.count(); ++ii )
                {
                    fImpl->hwAccel->addItem( fVerboseHWAccels[ ii ], fTerseHWAccels[ ii ] );
                }
            }

            CMakeMKVVideoSettings::~CMakeMKVVideoSettings()
            {
            }

            void CMakeMKVVideoSettings::load()
            {
                fImpl->transcodeVideo->setChecked( NPreferences::NCore::CPreferences::instance()->getTranscodeVideo() );
                fImpl->losslessTranscoding->setChecked( NPreferences::NCore::CPreferences::instance()->getLosslessTranscoding() );
                fImpl->useCRF->setChecked( NPreferences::NCore::CPreferences::instance()->getUseCRF() );
                fImpl->useExplicitCRF->setChecked( NPreferences::NCore::CPreferences::instance()->getUseExplicitCRF() );
                fImpl->explicitCRF->setValue( NPreferences::NCore::CPreferences::instance()->getExplicitCRF() );
                fImpl->usePreset->setChecked( NPreferences::NCore::CPreferences::instance()->getUsePreset() );
                fImpl->preset->setCurrentIndex( NPreferences::NCore::CPreferences::instance()->getPreset() );
                fImpl->useTune->setChecked( NPreferences::NCore::CPreferences::instance()->getUseTune() );
                fImpl->tune->setCurrentIndex( NPreferences::NCore::CPreferences::instance()->getTune() );
                fImpl->useProfile->setChecked( NPreferences::NCore::CPreferences::instance()->getUseProfile() );
                fImpl->profile->setCurrentIndex( NPreferences::NCore::CPreferences::instance()->getProfile() );
                fImpl->onlyTranscodeVideoOnFormatChange->setChecked( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeVideoOnFormatChange() );

                selectVideoCodec( NPreferences::NCore::CPreferences::instance()->getTranscodeToVideoCodec() );

                slotLosslessChanged();
                slotCRFChanged();
                slotUseExplicitCRFChanged();
                slotUsePresetChanged();
                slotUseTuneChanged();
                slotUseProfileChanged();
                slotCodecChanged();
                slotHWAccelChanged();
            }

            void CMakeMKVVideoSettings::selectVideoCodec( const QString & curr )
            {
                auto pos = fImpl->videoCodec->findData( curr );
                fImpl->videoCodec->setCurrentIndex( pos );
            }

            void CMakeMKVVideoSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setTranscodeToVideoCodec( fImpl->videoCodec->currentData().toString() );

                NPreferences::NCore::CPreferences::instance()->setTranscodeVideo( fImpl->transcodeVideo->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setLosslessTranscoding( fImpl->losslessTranscoding->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setUseCRF( fImpl->useCRF->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setUseExplicitCRF( fImpl->useExplicitCRF->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setExplicitCRF( fImpl->explicitCRF->value() );
                NPreferences::NCore::CPreferences::instance()->setUsePreset( fImpl->usePreset->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setPreset( static_cast< NMediaManager::NPreferences::NCore::EMakeMKVPreset >( fImpl->preset->currentIndex() ) );
                NPreferences::NCore::CPreferences::instance()->setUseTune( fImpl->useTune->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setTune( static_cast< NMediaManager::NPreferences::NCore::EMakeMKVTune >( fImpl->tune->currentIndex() ) );
                NPreferences::NCore::CPreferences::instance()->setUseProfile( fImpl->useProfile->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setProfile( static_cast< NMediaManager::NPreferences::NCore::EMakeMKVProfile >( fImpl->profile->currentIndex() ) );

                NPreferences::NCore::CPreferences::instance()->setOnlyTranscodeVideoOnFormatChange( fImpl->onlyTranscodeVideoOnFormatChange->isChecked() );
            }

            void CMakeMKVVideoSettings::slotLosslessChanged()
            {
                fImpl->useCRF->setChecked( !fImpl->losslessTranscoding->isChecked() );
            }

            void CMakeMKVVideoSettings::slotCRFChanged()
            {
                fImpl->losslessTranscoding->setChecked( !fImpl->useCRF->isChecked() );
            }

            void CMakeMKVVideoSettings::slotUseExplicitCRFChanged()
            {
                fImpl->explicitCRF->setEnabled( fImpl->useExplicitCRF->isChecked() );
            }

            void CMakeMKVVideoSettings::slotUsePresetChanged()
            {
                fImpl->preset->setEnabled( fImpl->usePreset->isChecked() );
            }

            void CMakeMKVVideoSettings::slotUseTuneChanged()
            {
                fImpl->tune->setEnabled( fImpl->useTune->isChecked() );
            }

            void CMakeMKVVideoSettings::slotUseProfileChanged()
            {
                fImpl->profile->setEnabled( fImpl->useProfile->isChecked() );
            }

            void CMakeMKVVideoSettings::slotCodecChanged()
            {
                auto currentCodec = fImpl->videoCodec->currentData().toString();
                bool isH265 = NSABUtils::CMediaInfo::isHEVCCodec( currentCodec );
                fImpl->h265Options->setEnabled( isH265 );

                auto hwAccel = NPreferences::NCore::CPreferences::instance()->getTranscodeHWAccel( currentCodec );
                auto pos = fImpl->hwAccel->findData( hwAccel );
                fImpl->hwAccel->setCurrentIndex( pos );


                if ( currentCodec == "hevc_qsv" )
                {
                    fImpl->intelGPUTranscode->setChecked( true );
                }
                else if ( currentCodec == "hevc_nvenc" )
                {
                    fImpl->nvidiaGPUTranscode->setChecked( true );
                }
                else if ( currentCodec == "hevc_amf" )
                {
                    fImpl->amdGPUTranscode->setChecked( true );
                }
                else if ( currentCodec == "libx265" )
                {
                    fImpl->softwareTranscode->setChecked( true );
                }
                else
                {
                    fImpl->intelGPUTranscode->setAutoExclusive( false );
                    fImpl->nvidiaGPUTranscode->setAutoExclusive( false );
                    fImpl->amdGPUTranscode->setAutoExclusive( false );
                    fImpl->softwareTranscode->setAutoExclusive( false );

                    fImpl->intelGPUTranscode->setChecked( false );
                    fImpl->nvidiaGPUTranscode->setChecked( false );
                    fImpl->amdGPUTranscode->setChecked( false );
                    fImpl->softwareTranscode->setChecked( false );

                    fImpl->intelGPUTranscode->setAutoExclusive( true );
                    fImpl->nvidiaGPUTranscode->setAutoExclusive( true );
                    fImpl->amdGPUTranscode->setAutoExclusive( true );
                    fImpl->softwareTranscode->setAutoExclusive( true );
                }
            }

            void CMakeMKVVideoSettings::slotExplicitCodecChanged()
            {
                if ( fImpl->intelGPUTranscode->isChecked() )
                    selectVideoCodec( "hevc_qsv" );
                if ( fImpl->nvidiaGPUTranscode->isChecked() )
                    selectVideoCodec( "hevc_nvenc" );
                if ( fImpl->amdGPUTranscode->isChecked() )
                    selectVideoCodec( "hevc_amf" );
                if ( fImpl->softwareTranscode->isChecked() )
                    selectVideoCodec( "libx265" );
            }

            void CMakeMKVVideoSettings::slotHWAccelChanged()
            {
                auto hwAccel = fImpl->hwAccel->currentData().toString();
                if ( hwAccel.isEmpty() )
                    return;
                auto codec = NPreferences::NCore::CPreferences::instance()->getCodecForHWAccel( hwAccel );
                selectVideoCodec( codec );
            }
        }
    }
}