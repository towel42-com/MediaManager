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

#include "ValidateMKVPage.h"
#include "Models/ValidateMKVModel.h"

#include "Preferences/Core/Preferences.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QRegularExpression>
#include <QTreeView>

namespace NMediaManager
{
    namespace NUi
    {
        CValidateMKVPage::CValidateMKVPage( QWidget *parent ) :
            CBasePage( "Validate MKV", parent )
        {
        }

        CValidateMKVPage::~CValidateMKVPage()
        {
        }

        NModels::CDirModel *CValidateMKVPage::createDirModel()
        {
            return new NModels::CValidateMKVModel( this );
        }

        QString CValidateMKVPage::secondaryProgressLabel() const
        {
            return tr( "Current (Stage):" );
        }

        QString CValidateMKVPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CValidateMKVPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CValidateMKVPage::actionTitleName() const
        {
            return tr( "Validating MKV..." );
        }

        QString CValidateMKVPage::actionCancelName() const
        {
            return tr( "Abort Validating MKV" );
        }

        QString CValidateMKVPage::actionErrorName() const
        {
            return tr( "Error while Validating MKV:" );
        }
    }
}
