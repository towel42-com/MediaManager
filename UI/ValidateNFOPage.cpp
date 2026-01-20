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

#include "ValidateNFOPage.h"

#include "Preferences/Core/Preferences.h"
#include "Models/ValidateNFOModel.h"
#include "T42-Utils/DoubleProgressDlg.h"

#include <QRegularExpression>

namespace NMediaManager
{
    namespace NUi
    {
        CValidateNFOPage::CValidateNFOPage( QWidget *parent ) :
            CBasePage( "Validate NFO Files", parent )
        {
        }

        CValidateNFOPage::~CValidateNFOPage()
        {
        }

        NModels::CDirModel *CValidateNFOPage::createDirModel()
        {
            return new NModels::CValidateNFOModel( this );
        }

        QString CValidateNFOPage::secondaryProgressLabel() const
        {
            return tr( "Current (seconds):" );
        }

        QString CValidateNFOPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CValidateNFOPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CValidateNFOPage::actionTitleName() const
        {
            return tr( "Generating Thumbnail Videos..." );
        }

        QString CValidateNFOPage::actionCancelName() const
        {
            return tr( "Abort Generating Thumbnail Videos" );
        }

        QString CValidateNFOPage::actionErrorName() const
        {
            return tr( "Error while Generating Thumbnail Videos:" );
        }
    }
}
