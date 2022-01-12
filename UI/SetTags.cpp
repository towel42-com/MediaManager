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

#include "SetTags.h"
#include "ui_SetTags.h"
#include "Core/Preferences.h"
#include "SABUtils/MKVUtils.h"

#include <QMessageBox>
#include <QTimer>


namespace NMediaManager
{
    namespace NUi
    {
        CSetTags::CSetTags( const QString & fileName, QWidget * parent )
            : QDialog( parent ),
            fImpl( new Ui::CSetTags )
        {
            fImpl->setupUi( this );
            fImpl->fileName->setText( fileName );
            fImpl->fileName->setReadOnly( true );
            QTimer::singleShot( 0, this, &CSetTags::slotLoadTags );
        }

        CSetTags::~CSetTags()
        {}

        void CSetTags::slotLoadTags()
        {
            auto tags = NSABUtils::getMediaTags( fImpl->fileName->text(), NCore::CPreferences::instance()->getFFProbeEXE() );
            fImpl->title->setText( tags["title"] );
            fImpl->releaseDate->setText( tags["date_recorded"] );
            fImpl->comments->setText( tags["comment"] );
        }

        void CSetTags::accept()
        {
            std::unordered_map< QString, QString > tags =
            {
                { "TITLE", fImpl->title->text() }
                ,{ "YEAR", fImpl->releaseDate->text() }
                ,{ "COMMENT", fImpl->comments->text() }
            };

            QString msg;
            if ( !NSABUtils::setMediaTags( fImpl->fileName->text(), tags, NCore::CPreferences::instance()->getMKVPropEditEXE(), &msg ) )
            {
                QMessageBox::critical( this, tr( "Problem setting tags" ), msg, QMessageBox::Ok );
                return;
            }
            QDialog::accept();
        }
    }
}