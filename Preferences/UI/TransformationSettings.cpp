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
#include "Preferences/Core/Preferences.h"

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
    namespace NPreferences
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
                fImpl->treatAsTVShowByDefault->setChecked( NPreferences::NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
                fImpl->exactMatchesOnly->setChecked( NPreferences::NCore::CPreferences::instance()->getExactMatchesOnly() );
                fImpl->loadMediaInfo->setChecked( NPreferences::NCore::CPreferences::instance()->getLoadMediaInfo() );
                fImpl->onlyTransformDirectories->setChecked( NPreferences::NCore::CPreferences::instance()->getOnlyTransformDirectories() );
            }

            void CTransformationSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->treatAsTVShowByDefault->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->exactMatchesOnly->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setLoadMediaInfo( fImpl->loadMediaInfo->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setOnlyTransformDirectories( fImpl->onlyTransformDirectories->isChecked() );
            }
        }
    }
}