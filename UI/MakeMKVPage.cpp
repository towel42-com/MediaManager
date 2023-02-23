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

#include "MakeMKVPage.h"

#include "Preferences/Core/Preferences.h"
#include "Models/MakeMKVModel.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QRegularExpression>

namespace NMediaManager
{
    namespace NUi
    {
        CMakeMKVPage::CMakeMKVPage( QWidget *parent ) :
            CBasePage( "Make MKV", parent )
        {
        }

        CMakeMKVPage::~CMakeMKVPage()
        {
        }

        NModels::CDirModel *CMakeMKVPage::createDirModel()
        {
            return new NModels::CMakeMKVModel( this );
        }

        QString CMakeMKVPage::secondaryProgressLabel() const
        {
            return tr( "Current (seconds):" );
        }

        QString CMakeMKVPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CMakeMKVPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CMakeMKVPage::actionTitleName() const
        {
            return tr( "Creating MKV..." );
        }

        QString CMakeMKVPage::actionCancelName() const
        {
            return tr( "Abort Creating MKV" );
        }

        QString CMakeMKVPage::actionErrorName() const
        {
            return tr( "Error while Creating MKV:" );
        }

        void CMakeMKVPage::postProcessLog( const QString &string )
        {
            // time=00:00:00.00
            auto regEx = QRegularExpression( "[Tt]ime\\=\\s*(?<hours>\\d{2}):(?<mins>\\d{2}):(?<secs>\\d{2})" );

            auto pos = string.lastIndexOf( regEx );
            if ( pos == -1 )
                return;

            auto match = regEx.match( string, pos );
            if ( !match.hasMatch() )
                return;

            auto hours = match.captured( "hours" );
            auto mins = match.captured( "mins" );
            auto secs = match.captured( "secs" );

            int numSeconds = 0;
            if ( !hours.isEmpty() )
            {
                bool aOK;
                int curr = hours.toInt( &aOK );
                if ( aOK )
                    numSeconds += curr * 60 * 60;
            }

            if ( !mins.isEmpty() )
            {
                bool aOK;
                int curr = mins.toInt( &aOK );
                if ( aOK )
                    numSeconds += curr * 60;
            }

            if ( !secs.isEmpty() )
            {
                bool aOK;
                int curr = secs.toInt( &aOK );
                if ( aOK )
                    numSeconds += curr;
            }

            fProgressDlg->setSecondaryValue( numSeconds );
        }
    }
}
