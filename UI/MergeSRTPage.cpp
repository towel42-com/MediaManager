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

#include "MergeSRTPage.h"
#include "Models/MergeSRTModel.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QRegularExpression>

namespace NMediaManager
{
    namespace NUi
    {
        CMergeSRTPage::CMergeSRTPage( QWidget * parent )
            : CBasePage( "Merge SRT", parent )
        {
        }

        CMergeSRTPage::~CMergeSRTPage()
        {
        }

        NModels::CDirModel * CMergeSRTPage::createDirModel()
        {
            return new NModels::CMergeSRTModel( this );
        }

        void CMergeSRTPage::postProcessLog( const QString & string )
        {
            auto regEx = QRegularExpression( "[Pp]rogress\\:\\s*(?<percent>\\d+)\\%" );
            auto pos = string.lastIndexOf( regEx );
            if ( pos == -1 )
                return;

            auto match = regEx.match( string, pos );
            if ( !match.hasMatch() )
                return;
            auto percent = match.captured( "percent" );
            if ( !percent.isEmpty() )
            {
                bool aOK = false;
                int percentVal = percent.toInt( &aOK );
                if ( aOK )
                    fProgressDlg->setSecondaryValue( percentVal );
            }
        }

        QStringList CMergeSRTPage::dirModelFilter() const
        {
            return QStringList() << "*.mkv";
        }

        QString CMergeSRTPage::secondaryProgressLabel() const
        {
            return tr( "Current Movie:" );
        }

        QString CMergeSRTPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CMergeSRTPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CMergeSRTPage::actionTitleName() const
        {
            return tr( "Merging SRT Files into MKV..." );
        }

        QString CMergeSRTPage::actionCancelName() const
        {
            return tr( "Abort Merge" );
        }

        QString CMergeSRTPage::actionErrorName() const
        {
            return tr( "Error while Merging SRT file into MKV:" );
        }
    }
}

