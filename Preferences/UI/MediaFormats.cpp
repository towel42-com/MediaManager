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

#include "MediaFormats.h"
#include "ui_MediaFormats.h"
#include "Preferences/Core/Preferences.h"

#include <QInputDialog>
#include <QProgressDialog>
namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CMediaFormats::CMediaFormats( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CMediaFormats )
            {
                fImpl->setupUi( this );

                connect( fImpl->recomputeBtn, &QPushButton::clicked, [ this ]() 
                    { 
                        QProgressDialog dlg( tr( "Loading Formats and Codecs"), tr( "Cancel" ), 0, 0, this );
                        dlg.setAutoClose( false );
                        dlg.setAutoReset( false );
                        dlg.setWindowModality( Qt::WindowModal );
                        dlg.show();

                        NPreferences::NCore::CPreferences::instance()->recomputeSupportedFormats( &dlg );
                        if ( !dlg.wasCanceled() )
                            load();
                    } );
            }

            CMediaFormats::~CMediaFormats()
            {
            }

            void CMediaFormats::load()
            {
                loadMediaFormats( fImpl->supportedFormats, NPreferences::NCore::CPreferences::instance()->availableMediaFormats( true ) );
                loadMediaFormats( fImpl->videoExtensions, NPreferences::NCore::CPreferences::instance()->getVideoExtensions() );
                loadMediaFormats( fImpl->subtitleExtensions, NPreferences::NCore::CPreferences::instance()->getSubtitleExtensions() );
            }

            void CMediaFormats::save()
            {
            }

            void CMediaFormats::loadMediaFormats( QListWidget *listWidget, QStringList &formats )
            {
                listWidget->clear();
                formats.sort();
                for ( auto &&ii : formats )
                {
                    new QListWidgetItem( ii, listWidget );
                }
            }
        }
    }
}
