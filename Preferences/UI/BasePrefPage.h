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

#ifndef __UI_BASEPREFPAGE_H
#define __UI_BASEPREFPAGE_H

#include <QWidget>

class QStringListModel;
class QListView;

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            class CBasePrefPage : public QWidget
            {
                Q_OBJECT
            public:
                CBasePrefPage( QWidget * parent = nullptr );
                virtual ~CBasePrefPage() override;

                virtual void load() = 0;
                virtual void save() = 0;
                virtual QStringList pageName() const = 0;
            public Q_SLOTS:
            protected:
                void addString( const QString & title, const QString & label, QStringListModel * model, QListView * listView, bool splitWords );
                void delString( QStringListModel * listModel, QListView * listView );
            };
        }
    }
}
#endif 
