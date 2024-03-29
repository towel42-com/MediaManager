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

#include "SkippedPaths.h"
#include "Preferences/Core/Preferences.h"

#include "ui_SkippedPaths.h"

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
            CSkippedPaths::CSkippedPaths( bool forMediaTransform, QWidget *parent ) :
                CBasePrefPage( parent ),
                fForMediaTransform( forMediaTransform ),
                fImpl( new Ui::CSkippedPaths )
            {
                fImpl->setupUi( this );

                connect( fImpl->btnAddSkipPathName, &QToolButton::clicked, this, &CSkippedPaths::slotAddSkipPathName );
                connect( fImpl->btnDelSkipPathName, &QToolButton::clicked, this, &CSkippedPaths::slotDelSkipPathName );

                fSkipPathNamesModel = new QStringListModel( this );
                fImpl->pathNamesToSkip->setModel( fSkipPathNamesModel );

                new NSABUtils::CButtonEnabler( fImpl->pathNamesToSkip, fImpl->btnDelSkipPathName );
            }

            CSkippedPaths::~CSkippedPaths()
            {
            }

            void CSkippedPaths::slotAddSkipPathName()
            {
                addString( tr( "Add Path Name to Skip" ), tr( "Path Name:" ), fSkipPathNamesModel, fImpl->pathNamesToSkip, false );
            }

            void CSkippedPaths::slotDelSkipPathName()
            {
                delString( fSkipPathNamesModel, fImpl->pathNamesToSkip );
            }

            void CSkippedPaths::load()
            {
                fSkipPathNamesModel->setStringList( NPreferences::NCore::CPreferences::instance()->getSkippedPaths( fForMediaTransform ) );
                fImpl->ignorePathNamesToSkip->setChecked( !NPreferences::NCore::CPreferences::instance()->getIgnorePathNamesToSkip( fForMediaTransform ) );
            }

            void CSkippedPaths::save()
            {
                NPreferences::NCore::CPreferences::instance()->setSkippedPaths( fForMediaTransform, fSkipPathNamesModel->stringList() );
                NPreferences::NCore::CPreferences::instance()->setIgnorePathNamesToSkip( fForMediaTransform, !fImpl->ignorePathNamesToSkip->isChecked() );
            }

            QStringList CSkippedPaths::pageName() const
            {
                if ( fForMediaTransform )
                    return QStringList( { "Paths", "Skipped Paths (Media Transform)" } );
                else
                    return QStringList( { "Paths", "Skipped Paths (Media Tagging)" } );
            }

        }
    }
}
