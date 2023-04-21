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

#include "BasePrefPage.h"
#include "SABUtils/StringUtils.h"

#include <QInputDialog>
#include <QRegularExpression>
#include <QStringListModel>
#include <QListView>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CBasePrefPage::CBasePrefPage( QWidget *parent ) :
                QWidget( parent )
            {
            }

            CBasePrefPage::~CBasePrefPage()
            {
            }

            void CBasePrefPage::addString( const QString &title, const QString &label, QStringListModel *model, QListView *listView, bool splitWords )
            {
                auto text = QInputDialog::getText( this, title, label ).trimmed();
                if ( text.isEmpty() )
                    return;

                QStringList words;
                if ( splitWords )
                    words = text.split( QRegularExpression( "\\s" ), NSABUtils::NStringUtils::TSkipEmptyParts );
                else
                    words.push_back( text );

                auto strings = model->stringList();

                for ( auto &&ii : words )
                {
                    ii = ii.trimmed();
                    strings.removeAll( ii );
                }

                strings << words;
                model->setStringList( strings );
                listView->scrollTo( model->index( strings.count() - 1, 0 ) );
            }

            void CBasePrefPage::delString( QStringListModel *listModel, QListView *listView )
            {
                auto model = listView->selectionModel();
                if ( !model )
                    return;

                auto selected = model->selectedRows();
                if ( selected.isEmpty() )
                    return;

                auto strings = listModel->stringList();
                for ( auto &&ii : selected )
                {
                    auto text = ii.data().toString();
                    strings.removeAll( text );
                }

                listModel->setStringList( strings );
                listView->scrollTo( listModel->index( selected.front().row(), 0 ) );
            }
        }
    }
}
