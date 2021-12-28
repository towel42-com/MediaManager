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

namespace NSABUtils
{
    class CKeyValuePairModel;
}
class QStringListModel;
class QLineEdit;
namespace NMediaManager
{
    namespace NUi
    {
        namespace Ui { class CPreferences; };
        class CPreferences : public QDialog
        {
            Q_OBJECT
        public:
            CPreferences( QWidget * parent = 0 );
            ~CPreferences();

        public Q_SLOTS:
            void slotAddKnownString();
            void slotDelKnownString();
            void slotAddExtraString();
            void slotDelExtraString();
            void slotAddAbbreviation();
            void slotDelAbbreviation();

            void slotSelectMKVMergeExe();
            void slotSelectMKVPropEditExe();
            void slotSelectFFMpegExe();
            void slotSelectFFProbeExe();

            void slotFFToolChanged();
            void slotMKVNixToolChanged();

            void accept() override;
        private:
            void mkvnixToolChanged( QLineEdit * le );
            void fftoolToolChanged( QLineEdit * le );

            void loadSettings();
            void saveSettings();

            void updateOtherTool( QObject * sender, const std::pair< QLineEdit *, QString > & lhs, const std::pair< QLineEdit *, QString > & rhs );

            QStringListModel * fKnownStringModel{ nullptr };
            QStringListModel * fExtraStringModel{ nullptr };
            NSABUtils::CKeyValuePairModel * fAbbreviationsModel{ nullptr };
            std::unique_ptr< Ui::CPreferences > fImpl;
        };
    }
}
#endif 
