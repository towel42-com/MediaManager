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

#include "GeneralSettings.h"
#include "Preferences/Core/Preferences.h"

#include "ui_GeneralSettings.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/UtilityModels.h"
#include "SABUtils/QtUtils.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CGeneralSettings::CGeneralSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CGeneralSettings )
            {
                fImpl->setupUi( this );
            }

            CGeneralSettings::~CGeneralSettings()
            {
            }

            void CGeneralSettings::load()
            {
                fImpl->loadMediaInfo->setChecked( NPreferences::NCore::CPreferences::instance()->getLoadMediaInfo() );
                fImpl->backgroundLoadMediaInfo->setChecked( NPreferences::NCore::CPreferences::instance()->getBackgroundLoadMediaInfo() );
            }

            void CGeneralSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setLoadMediaInfo( fImpl->loadMediaInfo->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setBackgroundLoadMediaInfo( fImpl->backgroundLoadMediaInfo->isChecked() );
            }
        }
    }
}