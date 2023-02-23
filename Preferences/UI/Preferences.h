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

#ifndef __UI_PREFERENCES_H
#define __UI_PREFERENCES_H

#include <QDialog>
#include <unordered_map>

class QTreeWidgetItem;
namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            class CBasePrefPage;
            namespace Ui
            {
                class CPreferences;
            };
            class CPreferences : public QDialog
            {
                Q_OBJECT
            public:
                CPreferences( QWidget *parent = nullptr );
                virtual ~CPreferences() override;
            public Q_SLOTS:
                void slotPageSelectorCurrChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );
                void slotPageSelectorItemActived( QTreeWidgetItem *item );
                void slotPageSelectorSelectionChanged();
                void slotValidatePreferences();
                void slotApply();
                void accept() override;

            private:
                static QString keyForItem( QTreeWidgetItem *item );

                void loadSettings();
                void saveSettings();

                void addPage( CBasePrefPage *page );
                void loadPages();

                std::unordered_map< QString, QTreeWidgetItem * > fItemMap;
                std::unordered_map< QTreeWidgetItem *, CBasePrefPage * > fPageMap;
                std::unique_ptr< Ui::CPreferences > fImpl;
            };
        }
    }
}
#endif
