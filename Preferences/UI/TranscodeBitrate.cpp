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

#include "TranscodeVideoQuality.h"
#include "QualityExperiment.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/FFMpegFormats.h"

#include "ui_TranscodeVideoQuality.h"

#include <QIntValidator>
#include <QWidgetAction>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CTranscodeVideoQuality::CTranscodeVideoQuality( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CTranscodeVideoQuality )
            {
                fImpl->setupUi( this );
                fImpl->target4kBitrate->setSuffix( "(kbps)" );
                fImpl->targetHDBitrate->setSuffix( "(kbps)" );
                fImpl->targetSubHDBitrate->setSuffix( "(kbps)" );
            }

            CTranscodeVideoQuality::~CTranscodeVideoQuality()
            {
            }

            void CTranscodeVideoQuality::load()
            {
                fImpl->target4kBitrate->setText( QString::number( NPreferences::NCore::CPreferences::instance()->getTarget4kBitrate() ) );
                fImpl->targetHDBitrate->setText( QString::number( NPreferences::NCore::CPreferences::instance()->getTargetHDBitrate() ) );
                fImpl->targetSubHDBitrate->setText( QString::number( NPreferences::NCore::CPreferences::instance()->getTargetSubHDBitrate() ) );
                fImpl->greaterThan4kDivisor->setValue( NPreferences::NCore::CPreferences::instance()->getGreaterThan4kDivisor() );

                fImpl->resolutionThreshold->setValue( NPreferences::NCore::CPreferences::instance()->getResolutionThresholdPercentage() );

                connect( fImpl->experiment, &QPushButton::clicked, this, &CTranscodeVideoQuality::slotExperiment );
            }

            void CTranscodeVideoQuality::save()
            {
                NPreferences::NCore::CPreferences::instance()->setTarget4kBitrate( fImpl->target4kBitrate->text().toDouble() );
                NPreferences::NCore::CPreferences::instance()->setTargetHDBitrate( fImpl->targetHDBitrate->text().toDouble() );
                NPreferences::NCore::CPreferences::instance()->setTargetSubHDBitrate( fImpl->targetSubHDBitrate->text().toDouble() );
                NPreferences::NCore::CPreferences::instance()->setGreaterThan4kDivisor( fImpl->greaterThan4kDivisor->value() );

                NPreferences::NCore::CPreferences::instance()->setResolutionThresholdPercentage( fImpl->resolutionThreshold->value() );
            }

            void CTranscodeVideoQuality::slotExperiment()
            {
                CQualityExperiment dlg( this );
                dlg.setGreaterThan4kDivisor( fImpl->greaterThan4kDivisor->value() );
                dlg.setTarget4kBitrate( fImpl->target4kBitrate->text() );
                dlg.setTargetHDBitrate( fImpl->targetHDBitrate->text() );
                dlg.setTargetSubHDBitrate( fImpl->targetSubHDBitrate->text() );
                dlg.setResolutionThreshold( fImpl->resolutionThreshold->value() );

                if ( dlg.exec() == QDialog::Accepted )
                {
                    fImpl->greaterThan4kDivisor->setValue( dlg.greaterThan4kDivisor() );
                    fImpl->target4kBitrate->setText( dlg.getTarget4kBitrate() );
                    fImpl->targetHDBitrate->setText( dlg.getTargetHDBitrate() );
                    fImpl->targetSubHDBitrate->setText( dlg.getTargetSubHDBitrate() );
                    fImpl->resolutionThreshold->setValue( dlg.resolutionThreshold() );
                }
            }
        }
    }
}