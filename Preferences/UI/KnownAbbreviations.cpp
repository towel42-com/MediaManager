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

#include "KnownAbbreviations.h"
#include "Preferences/Core/Preferences.h"

#include "ui_KnownAbbreviations.h"

#include <QInputDialog>

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/UtilityModels.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CKnownAbbreviations::CKnownAbbreviations( QWidget * parent )
                : CBasePrefPage( parent ),
                fImpl( new Ui::CKnownAbbreviations )
            {
                fImpl->setupUi( this );

                connect( fImpl->btnAddAbbreviation, &QToolButton::clicked, this, &CKnownAbbreviations::slotAddAbbreviation );
                connect( fImpl->btnDelAbbreviation, &QToolButton::clicked, this, &CKnownAbbreviations::slotDelAbbreviation );

                fAbbreviationsModel = new NSABUtils::CKeyValuePairModel( this );
                fImpl->knownAbbreviations->setModel( fAbbreviationsModel );

                new NSABUtils::CButtonEnabler( fImpl->knownAbbreviations, fImpl->btnDelAbbreviation );
            }

            CKnownAbbreviations::~CKnownAbbreviations()
            {
            }

            void CKnownAbbreviations::slotAddAbbreviation()
            {
                auto text = QInputDialog::getText( this, tr( "Add Abbreviation in the form Abbreviation=FullText" ), tr( "String:" ) );
                if ( text.isEmpty() )
                    return;
                text = text.trimmed();

                fAbbreviationsModel->addRow( text );
                fImpl->knownAbbreviations->scrollTo( fAbbreviationsModel->index( fAbbreviationsModel->rowCount() - 1, 0 ) );
            }

            void CKnownAbbreviations::slotDelAbbreviation()
            {
                auto model = fImpl->knownAbbreviations->selectionModel();
                if ( !model )
                    return;
                auto selected = model->selectedRows();
                if ( selected.isEmpty() )
                    return;
                fAbbreviationsModel->removeRow( selected.front().row() );
                fImpl->knownAbbreviations->scrollTo( fAbbreviationsModel->index( selected.front().row(), 0 ) );
            }

            void CKnownAbbreviations::load()
            {
                fAbbreviationsModel->setValues( NPreferences::NCore::CPreferences::instance()->getKnownAbbreviations() );
            }

            void CKnownAbbreviations::save()
            {
                NPreferences::NCore::CPreferences::instance()->setKnownAbbreviations( fAbbreviationsModel->data() );
            }
        }
    }
}
