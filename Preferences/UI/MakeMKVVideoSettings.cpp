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

                fImpl->intelGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasIntelGPU() );
                fImpl->nvidiaGPUTranscode->setEnabled( NPreferences::NCore::CPreferences::instance()->hasNVidiaGPU() );

                slotLosslessChanged();
                slotCRFChanged();
                slotUseExplicitCRFChanged();
                slotUsePresetChanged();
                slotUseTuneChanged();
                slotUseProfileChanged();
            }

            CMakeMKVVideoSettings::~CMakeMKVVideoSettings()
            {
            }

            void CMakeMKVVideoSettings::load()
            {
                fImpl->intelGPUTranscode->setChecked( NPreferences::NCore::CPreferences::instance()->getIntelGPUTranscode() );
                fImpl->nvidiaGPUTranscode->setChecked( NPreferences::NCore::CPreferences::instance()->getNVidiaGPUTranscode() );
                fImpl->softwareTranscode->setChecked( NPreferences::NCore::CPreferences::instance()->getSoftwareTranscode() );

                fImpl->convertToH265->setChecked( NPreferences::NCore::CPreferences::instance()->getTranscodeToH265() );
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
            }

            void CMakeMKVVideoSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setIntelGPUTranscode( fImpl->intelGPUTranscode->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setNVidiaGPUTranscode( fImpl->nvidiaGPUTranscode->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setSoftwareTranscode( fImpl->softwareTranscode->isChecked() );

                NPreferences::NCore::CPreferences::instance()->setTranscodeToH265( fImpl->convertToH265->isChecked() );
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

        }
    }
}