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

#include "ProcessConfirm.h"
#include "ui_ProcessConfirm.h"

#include "SABUtils/QtUtils.h"

#include <QAbstractButton>
#include <QMessageBox>

namespace NMediaManager
{
    namespace NUi
    {
        CProcessConfirm::CProcessConfirm( const QString &title, const QString &label, QWidget *parent ) :
            QDialog( parent ),
            fImpl( new Ui::CProcessConfirm )
        {
            fImpl->setupUi( this );

            setWindowFlags( this->windowFlags() & ~Qt::WindowContextHelpButtonHint );

            QSizePolicy sizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred );
            sizePolicy.setHorizontalStretch( 0 );
            sizePolicy.setVerticalStretch( 0 );
            sizePolicy.setHeightForWidth( fImpl->iconLabel->sizePolicy().hasHeightForWidth() );

            fImpl->iconLabel->setSizePolicy( sizePolicy );
            fImpl->iconLabel->setVisible( false );

            setIconLabel( QMessageBox::Information );

            connect( fImpl->buttonBox, &QDialogButtonBox::clicked, this, &CProcessConfirm::slotButtonClicked );

            setLabel( label );
            setTitle( title );
        }

        CProcessConfirm::~CProcessConfirm()
        {
        }

        void CProcessConfirm::setTitle( const QString &text )
        {
            setWindowTitle( text );
        }

        void CProcessConfirm::setLabel( const QString &text )
        {
            fImpl->label->setText( text );
        }

        void CProcessConfirm::setModel( QAbstractItemModel *model )
        {
            fImpl->transformations->setModel( model );
            NSABUtils::expandAll( fImpl->transformations );
            auto width = NSABUtils::autoSize( fImpl->transformations );
            if ( fImpl->transformations->width() < width )
            {
                fImpl->transformations->setMinimumWidth( width );
            }
        }

        void CProcessConfirm::setIconLabel( const QMessageBox::Icon &icon )
        {
            fImpl->iconLabel->setVisible( true );
            auto pm = QMessageBox::standardIcon( icon );
            fImpl->iconLabel->setPixmap( pm );
            fImpl->iconLabel->setVisible( !pm.isNull() );
            if ( icon == QMessageBox::Icon::Critical )
                QApplication::beep();
        }

        void CProcessConfirm::setButtons( QDialogButtonBox::StandardButtons buttons )
        {
            fImpl->buttonBox->setStandardButtons( buttons );
        }

        void CProcessConfirm::slotButtonClicked( QAbstractButton *btn )
        {
            if ( ( fImpl->buttonBox->buttonRole( btn ) == QDialogButtonBox::ApplyRole ) || ( fImpl->buttonBox->buttonRole( btn ) == QDialogButtonBox::YesRole ) )
            {
                accept();
            }
        }
    }
}