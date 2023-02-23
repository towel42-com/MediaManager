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

#include "Extensions.h"
#include "ui_Extensions.h"
#include "Preferences/Core/Preferences.h"

#include "SABUtils/ButtonEnabler.h"

#include <QInputDialog>
namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CExtensions::CExtensions( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CExtensions )
            {
                fImpl->setupUi( this );
                new NSABUtils::CButtonEnabler( fImpl->videoExtensions, fImpl->delVideoExt );
                new NSABUtils::CButtonEnabler( fImpl->subtitleExtensions, fImpl->delSubExt );

                connect( fImpl->recomputeBtn, &QPushButton::clicked, [ this ]() { loadExtensions( fImpl->videoExtensions, NPreferences::NCore::CPreferences::instance()->defaultVideoExtensions( true ) ); } );

                connect( fImpl->addVideoExt, &QPushButton::clicked, [ this ]() { addExtension( tr( "Video Extension" ), tr( "Extension:" ), fImpl->videoExtensions ); } );

                connect( fImpl->delVideoExt, &QPushButton::clicked, [ this ]() { delExtension( fImpl->videoExtensions ); } );

                connect( fImpl->addSubExt, &QPushButton::clicked, [ this ]() { addExtension( tr( "Subtitle Extension" ), tr( "Extension:" ), fImpl->subtitleExtensions ); } );

                connect( fImpl->delSubExt, &QPushButton::clicked, [ this ]() { delExtension( fImpl->subtitleExtensions ); } );
            }

            CExtensions::~CExtensions()
            {
            }

            void CExtensions::load()
            {
                loadExtensions( fImpl->videoExtensions, NPreferences::NCore::CPreferences::instance()->getVideoExtensions() );
                loadExtensions( fImpl->subtitleExtensions, NPreferences::NCore::CPreferences::instance()->getSubtitleExtensions() );
            }

            void CExtensions::save()
            {
                NPreferences::NCore::CPreferences::instance()->setMediaExtensions( getExtensions( fImpl->videoExtensions ) );
                NPreferences::NCore::CPreferences::instance()->setSubtitleExtensions( getExtensions( fImpl->subtitleExtensions ) );
            }

            void CExtensions::loadExtensions( QListWidget *listWidget, QStringList &extensions )
            {
                listWidget->clear();
                extensions.sort();
                for ( auto &&ii : extensions )
                    ii = ii.toLower();
                for ( auto &&ii : extensions )
                {
                    new QListWidgetItem( ii, listWidget );
                }
            }

            QStringList CExtensions::getExtensions( QListWidget *listWidget )
            {
                QStringList retVal;
                for ( int ii = 0; ii < listWidget->count(); ++ii )
                    retVal << listWidget->item( ii )->text();
                return retVal;
            }

            void CExtensions::addExtension( const QString &title, const QString &label, QListWidget *listWidget )
            {
                auto ext = QInputDialog::getText( this, title, label );
                if ( ext.isEmpty() )
                    return;

                ext = ext.toLower();
                auto tmp = ext.split( "." );
                if ( tmp.isEmpty() )
                    return;
                if ( tmp[ 0 ] != "*" )
                    tmp.push_front( "*" );
                ext = tmp.join( "." );
                new QListWidgetItem( ext, listWidget );
                listWidget->sortItems();
            }

            void CExtensions::delExtension( QListWidget *listWidget )
            {
                if ( !listWidget )
                    return;
                auto item = listWidget->currentItem();
                if ( !item )
                    return;
                delete item;
            }

        }
    }
}
