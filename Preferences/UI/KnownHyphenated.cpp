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

#include "KnownHyphenated.h"
#include "Preferences/Core/Preferences.h"

#include "ui_KnownHyphenated.h"

#include <QInputDialog>

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/UtilityModels.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CKnownHyphenated::CKnownHyphenated( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CKnownHyphenated )
            {
                fImpl->setupUi( this );

                connect( fImpl->btnAddHyphenated, &QToolButton::clicked, this, &CKnownHyphenated::slotAddHyphenated );
                connect( fImpl->btnDelHyphenated, &QToolButton::clicked, this, &CKnownHyphenated::slotDelHyphenated );

                fKnownHyphenatedModel = new QStringListModel( this );
                fImpl->knownHyphenated->setModel( fKnownHyphenatedModel );

                new NSABUtils::CButtonEnabler( fImpl->knownHyphenated, fImpl->btnDelHyphenated );
            }

            CKnownHyphenated::~CKnownHyphenated()
            {
            }

            void CKnownHyphenated::slotAddHyphenated()
            {
                addString( tr( "Add Hyphenated String" ), tr( "String:" ), fKnownHyphenatedModel, fImpl->knownHyphenated, true );
            }

            void CKnownHyphenated::slotDelHyphenated()
            {
                delString( fKnownHyphenatedModel, fImpl->knownHyphenated );
            }

            void CKnownHyphenated::load()
            {
                fKnownHyphenatedModel->setStringList( NPreferences::NCore::CPreferences::instance()->getKnownHyphenated() );
            }

            void CKnownHyphenated::save()
            {
                NPreferences::NCore::CPreferences::instance()->setKnownHyphenated( fKnownHyphenatedModel->stringList() );
            }
        }
    }
}
