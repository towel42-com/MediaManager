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

#include "MakeMKVSettings.h"
#include "Preferences/Core/Preferences.h"

#include "ui_MakeMKVSettings.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CMakeMKVSettings::CMakeMKVSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CMakeMKVSettings )
            {
                fImpl->setupUi( this );
                connect( fImpl->losslessTranscoding, &QCheckBox::toggled, this, &CMakeMKVSettings::slotLosslessChanged );
                connect( fImpl->useCRF, &QGroupBox::toggled, this, &CMakeMKVSettings::slotCRFChanged );

                connect( fImpl->useExplicitCRF, &QCheckBox::toggled, this, &CMakeMKVSettings::slotUseExplicitCRFChanged );
                connect( fImpl->usePreset, &QCheckBox::toggled, this, &CMakeMKVSettings::slotUsePresetChanged );
                connect( fImpl->useTune, &QCheckBox::toggled, this, &CMakeMKVSettings::slotUseTuneChanged );
                connect( fImpl->useProfile, &QCheckBox::toggled, this, &CMakeMKVSettings::slotUseProfileChanged );

                fImpl->intelGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasIntelGPU() );
                fImpl->nvidiaGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasNVidiaGPU() );

                slotLosslessChanged();
                slotCRFChanged();
                slotUseExplicitCRFChanged();
                slotUsePresetChanged();
                slotUseTuneChanged();
                slotUseProfileChanged();
            }

            CMakeMKVSettings::~CMakeMKVSettings()
            {
            }

            void CMakeMKVSettings::load()
            {
                fImpl->intelGPUTranscode->setChecked( NPreferences::NCore::CPreferences::instance()->getIntelGPUTranscode() );
                fImpl->nvidiaGPUTranscode->setChecked( NPreferences::NCore::CPreferences::instance()->getNVidiaGPUTranscode() );
                fImpl->softwareTranscode->setChecked( NPreferences::NCore::CPreferences::instance()->getSoftwareTranscode() );

                fImpl->convertToH265->setChecked( NPreferences::NCore::CPreferences::instance()->getConvertToH265() );
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
            }

            void CMakeMKVSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setIntelGPUTranscode( fImpl->intelGPUTranscode->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setNVidiaGPUTranscode( fImpl->nvidiaGPUTranscode->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setSoftwareTranscode( fImpl->softwareTranscode->isChecked() );

                NPreferences::NCore::CPreferences::instance()->setConvertToH265( fImpl->convertToH265->isChecked() );
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
            }

            void CMakeMKVSettings::slotLosslessChanged()
            {
                fImpl->useCRF->setChecked( !fImpl->losslessTranscoding->isChecked() );
            }

            void CMakeMKVSettings::slotCRFChanged()
            {
                fImpl->losslessTranscoding->setChecked( !fImpl->useCRF->isChecked() );
            }

            void CMakeMKVSettings::slotUseExplicitCRFChanged()
            {
                fImpl->explicitCRF->setEnabled( fImpl->useExplicitCRF->isChecked() );
            }

            void CMakeMKVSettings::slotUsePresetChanged()
            {
                fImpl->preset->setEnabled( fImpl->usePreset->isChecked() );
            }

            void CMakeMKVSettings::slotUseTuneChanged()
            {
                fImpl->tune->setEnabled( fImpl->useTune->isChecked() );
            }

            void CMakeMKVSettings::slotUseProfileChanged()
            {
                fImpl->profile->setEnabled( fImpl->useProfile->isChecked() );
            }

        }
    }
}