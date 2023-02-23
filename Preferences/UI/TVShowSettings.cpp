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

#include "TVShowSettings.h"
#include "Preferences/Core/Preferences.h"

#include "ui_TVShowSettings.h"

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
            CTVShowSettings::CTVShowSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CTVShowSettings )
            {
                fImpl->setupUi( this );
            }

            CTVShowSettings::~CTVShowSettings()
            {
            }

            void CTVShowSettings::load()
            {
                fImpl->seasonOutDirPattern->setText( NPreferences::NCore::CPreferences::instance()->getSeasonOutDirPattern() );
                fImpl->tvOutFilePattern->setText( NPreferences::NCore::CPreferences::instance()->getTVOutFilePattern() );
                fImpl->tvOutDirPattern->setText( NPreferences::NCore::CPreferences::instance()->getTVOutDirPattern( false ) );
            }

            void CTVShowSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setSeasonOutDirPattern( fImpl->seasonOutDirPattern->text() );
                NPreferences::NCore::CPreferences::instance()->setTVOutFilePattern( fImpl->tvOutFilePattern->text() );
                NPreferences::NCore::CPreferences::instance()->setTVOutDirPattern( fImpl->tvOutDirPattern->text() );
            }
        }
    }
}