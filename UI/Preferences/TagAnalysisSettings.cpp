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

#include "TransformationSettings.h"
#include "Core/Preferences.h"

#include "ui_TransformationSettings.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/UtilityModels.h"
#include "SABUtils/QtUtils.h"

namespace NMediaManager
{
    namespace NUi
    {
        CTransformationSettings::CTransformationSettings( QWidget * parent )
            : CBasePrefPage( parent ),
            fImpl( new Ui::CTransformationSettings )
        {
            fImpl->setupUi( this );
        }

        CTransformationSettings::~CTransformationSettings()
        {
        }


        void CTransformationSettings::load()
        {
            fImpl->treatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->exactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );
            fImpl->verifyMediaTags->setChecked(NCore::CPreferences::instance()->getVerifyMediaTags());
            fImpl->verifyMediaTitle->setChecked(NCore::CPreferences::instance()->getVerifyMediaTitle());
            fImpl->verifyMediaDate->setChecked(NCore::CPreferences::instance()->getVerifyMediaDate());
        }

        void CTransformationSettings::save()
        {
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->treatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly(fImpl->exactMatchesOnly->isChecked());
            NCore::CPreferences::instance()->setVerifyMediaTags(fImpl->verifyMediaTags->isChecked());
            NCore::CPreferences::instance()->setVerifyMediaTitle(fImpl->verifyMediaTitle->isChecked());
            NCore::CPreferences::instance()->setVerifyMediaDate(fImpl->verifyMediaDate->isChecked());
        }
    }
}
