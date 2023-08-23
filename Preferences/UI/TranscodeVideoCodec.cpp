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

#include "TranscodeVideoCodec.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/FFMpegFormats.h"

#include "ui_TranscodeVideoCodec.h"

#include <QIntValidator>
#include <QWidgetAction>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CTranscodeVideoCodec::CTranscodeVideoCodec( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CTranscodeVideoCodec )
            {
                fImpl->setupUi( this );

                connect( fImpl->useAvgBitrateLabel, &QLabel::linkActivated, [ this ]() { emit sigOpenBitratePage(); } );

                connect( fImpl->losslessEncoding, &QRadioButton::toggled, this, &CTranscodeVideoCodec::slotQualityGoalMetricChanged );
                connect( fImpl->useAvgBitrate, &QRadioButton::toggled, this, &CTranscodeVideoCodec::slotQualityGoalMetricChanged );
                connect( fImpl->useCRF, &QRadioButton::toggled, this, &CTranscodeVideoCodec::slotQualityGoalMetricChanged );

                connect( fImpl->explicitCRFSlider, &QSlider::valueChanged, this, &CTranscodeVideoCodec::slotExplicitCRFSliderChanged );

                connect( fImpl->usePreset, &QCheckBox::toggled, this, &CTranscodeVideoCodec::slotUsePresetChanged );
                connect( fImpl->useTune, &QCheckBox::toggled, this, &CTranscodeVideoCodec::slotUseTuneChanged );
                connect( fImpl->useProfile, &QCheckBox::toggled, this, &CTranscodeVideoCodec::slotUseProfileChanged );
                connect( fImpl->videoCodec, qOverload< int >( &QComboBox::currentIndexChanged ), this, &CTranscodeVideoCodec::slotCodecChanged );
                connect( fImpl->hwAccel, qOverload< int >( &QComboBox::currentIndexChanged ), this, &CTranscodeVideoCodec::slotHWAccelChanged );

                fImpl->intelGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasIntelGPU() );
                fImpl->nvidiaGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasNVidiaGPU() );
                fImpl->amdGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasAMDGPU() );

                connect( fImpl->intelGPUTranscode, &QRadioButton::toggled, this, &CTranscodeVideoCodec::slotExplicitCodecChanged );
                connect( fImpl->nvidiaGPUTranscode, &QRadioButton::toggled, this, &CTranscodeVideoCodec::slotExplicitCodecChanged );
                connect( fImpl->amdGPUTranscode, &QRadioButton::toggled, this, &CTranscodeVideoCodec::slotExplicitCodecChanged );
                connect( fImpl->softwareTranscode, &QRadioButton::toggled, this, &CTranscodeVideoCodec::slotExplicitCodecChanged );

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

            CTranscodeVideoCodec::~CTranscodeVideoCodec()
            {
            }

            void CTranscodeVideoCodec::load()
            {
                fImpl->losslessEncoding->setChecked( NPreferences::NCore::CPreferences::instance()->getLosslessEncoding() );
                fImpl->useAvgBitrate->setChecked( NPreferences::NCore::CPreferences::instance()->getUseTargetBitrate() );
                fImpl->useCRF->setChecked( NPreferences::NCore::CPreferences::instance()->getUseCRF() );
                setCurrentCRFValue( NPreferences::NCore::CPreferences::instance()->getCRF() );

                fImpl->usePreset->setChecked( NPreferences::NCore::CPreferences::instance()->getUsePreset() );
                fImpl->preset->setCurrentIndex( NPreferences::NCore::CPreferences::instance()->getPreset() );
                fImpl->useTune->setChecked( NPreferences::NCore::CPreferences::instance()->getUseTune() );
                fImpl->tune->setCurrentIndex( NPreferences::NCore::CPreferences::instance()->getTune() );
                fImpl->useProfile->setChecked( NPreferences::NCore::CPreferences::instance()->getUseProfile() );
                fImpl->profile->setCurrentIndex( NPreferences::NCore::CPreferences::instance()->getProfile() );

                selectVideoCodec( NPreferences::NCore::CPreferences::instance()->getTranscodeToVideoCodec() );

                slotQualityGoalMetricChanged();
                slotUsePresetChanged();
                slotUseTuneChanged();
                slotUseProfileChanged();
                slotCodecChanged();
                slotHWAccelChanged();
                slotExplicitCRFSliderChanged();
            }

            void CTranscodeVideoCodec::save()
            {
                NPreferences::NCore::CPreferences::instance()->setTranscodeToVideoCodec( fImpl->videoCodec->currentData().toString() );

                NPreferences::NCore::CPreferences::instance()->setLosslessEncoding( fImpl->losslessEncoding->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setUseTargetBitrate( fImpl->useAvgBitrate->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setUseCRF( fImpl->useCRF->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setCRF( getCurrentCRFValue() );

                NPreferences::NCore::CPreferences::instance()->setUsePreset( fImpl->usePreset->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setPreset( static_cast< NMediaManager::NPreferences::NCore::ETranscodePreset >( fImpl->preset->currentIndex() ) );
                NPreferences::NCore::CPreferences::instance()->setUseTune( fImpl->useTune->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setTune( static_cast< NMediaManager::NPreferences::NCore::ETranscodeTune >( fImpl->tune->currentIndex() ) );
                NPreferences::NCore::CPreferences::instance()->setUseProfile( fImpl->useProfile->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setProfile( static_cast< NMediaManager::NPreferences::NCore::ETranscodeProfile >( fImpl->profile->currentIndex() ) );

                NPreferences::NCore::CPreferences::instance()->setTranscodeToVideoCodec( fImpl->videoCodec->currentData().toString() );
            }

            double CTranscodeVideoCodec::getCurrentCRFValue() const
            {
                auto value = fImpl->explicitCRFSlider->value();
                return 1.0 * value / 2.0;
            }
            
            void CTranscodeVideoCodec::setCurrentCRFValue( double value )
            {
                fImpl->explicitCRFSlider->setValue( 2 * value );   // from 0/51 but slider is 0 to 102
            }

            void CTranscodeVideoCodec::selectVideoCodec( const QString &curr )
            {
                auto pos = fImpl->videoCodec->findData( curr );
                fImpl->videoCodec->setCurrentIndex( pos );
            }

            void CTranscodeVideoCodec::slotQualityGoalMetricChanged()
            {
                fImpl->explicitCRFSlider->setEnabled( fImpl->useCRF->isChecked() );
            }

            void CTranscodeVideoCodec::slotExplicitCRFSliderChanged()
            {
                auto value = fImpl->explicitCRFSlider->value() / 2.0;
                fImpl->useCRF->setText( tr( "Constant Quality (CRF)?: %1" ).arg( value ) );
            }
            void CTranscodeVideoCodec::slotUsePresetChanged()
            {
                fImpl->preset->setEnabled( fImpl->usePreset->isChecked() );
            }

            void CTranscodeVideoCodec::slotUseTuneChanged()
            {
                fImpl->tune->setEnabled( fImpl->useTune->isChecked() );
            }

            void CTranscodeVideoCodec::slotUseProfileChanged()
            {
                fImpl->profile->setEnabled( fImpl->useProfile->isChecked() );
            }

            void CTranscodeVideoCodec::slotCodecChanged()
            {
                auto currentCodec = fImpl->videoCodec->currentData().toString();
                bool isH265 = NPreferences::NCore::CPreferences::instance()->getMediaFormats()->isHEVCCodec( currentCodec );
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

            void CTranscodeVideoCodec::slotExplicitCodecChanged()
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

            void CTranscodeVideoCodec::slotHWAccelChanged()
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