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

#include "GenerateBIFPage.h"

#include "Preferences/Core/Preferences.h"
#include "Models/GenerateBIFModel.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QRegularExpression>

namespace NMediaManager
{
    namespace NUi
    {
        CGenerateBIFPage::CGenerateBIFPage( QWidget * parent )
            : CBasePage( "Generate Thumbnails", parent )
        {
        }

        CGenerateBIFPage::~CGenerateBIFPage()
        {
        }

        NModels::CDirModel * CGenerateBIFPage::createDirModel()
        {
            return new NModels::CGenerateBIFModel( this );
        }

        QStringList CGenerateBIFPage::dirModelFilter() const
        {
            return QStringList() << "*.mkv";
        }

        QString CGenerateBIFPage::secondaryProgressLabel() const
        {
            return tr( "Current (seconds):" );
        }

        QString CGenerateBIFPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CGenerateBIFPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CGenerateBIFPage::actionTitleName() const
        {
            return tr( "Generating Thumbnail files..." );
        }

        QString CGenerateBIFPage::actionCancelName() const
        {
            return tr( "Abort Generating Thumbnail files" );
        }

        QString CGenerateBIFPage::actionErrorName() const
        {
            return tr( "Error while Generating Thumbnail files:" );
        }

        void CGenerateBIFPage::postProcessLog( const QString & string )
        {
            // Skip-Option - Write output: pkt_pts_time:2570 pkt_dts_time:2570 input_pts_time:2570.2
            // time=00:00:00.00
            auto regEx = QRegularExpression( R"(pkt_pts_[Tt]ime\:(?<secs>\d+))" );
            auto pos = string.lastIndexOf( regEx );
            if ( pos == -1 )
                return;

            auto match = regEx.match( string, pos );
            if ( !match.hasMatch() )
                return;

            auto secs = match.captured( "secs" );
            int numSeconds = 0;
            if ( !secs.isEmpty() )
            {
                bool aOK;
                int curr = secs.toInt( &aOK );
                if ( aOK )
                    numSeconds = curr;
            }

            fProgressDlg->setSecondaryValue( numSeconds );
        }
    }
}

