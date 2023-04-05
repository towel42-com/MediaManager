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

#include "TranscodePage.h"

#include "Preferences/Core/Preferences.h"
#include "Models/TranscodeModel.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/utils.h"

#include <QRegularExpression>

namespace NMediaManager
{
    namespace NUi
    {
        CTranscodePage::CTranscodePage( QWidget *parent ) :
            CBasePage( "Transcode Video", parent )
        {
        }

        CTranscodePage::~CTranscodePage()
        {
        }

        NModels::CDirModel *CTranscodePage::createDirModel()
        {
            return new NModels::CTranscodeModel( this );
        }

        QString CTranscodePage::secondaryProgressLabel() const
        {
            return tr( "Current:" );
        }

        QString CTranscodePage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CTranscodePage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CTranscodePage::actionTitleName() const
        {
            return tr( "Transcoding Videos..." );
        }

        QString CTranscodePage::actionCancelName() const
        {
            return tr( "Abort Transcoding Videos" );
        }

        QString CTranscodePage::actionErrorName() const
        {
            return tr( "Error while Transcoding Videos:" );
        }
    }
}
