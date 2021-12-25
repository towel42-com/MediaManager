// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#ifndef _MERGESRTPAGE_H
#define _MERGESRTPAGE_H

#include <QWidget>
class CDoubleProgressDlg;

namespace NMediaManager
{
    namespace NCore
    {
        class CDirModel;
    }
}

namespace NMediaManager
{
    namespace NUi
    {
        namespace Ui { class CMergeSRTPage; };
        class CMergeSRTPage : public QWidget
        {
            Q_OBJECT
        public:
            CMergeSRTPage( QWidget *parent = 0 );
            ~CMergeSRTPage();

            void load( const QString & dirName );
            void load();

            void run();
            bool canRun() const;

            void setSetupProgressDlgFunc( std::function< std::shared_ptr< CDoubleProgressDlg >( const QString &title, const QString &cancelButtonText, int max ) > setupFunc, std::function< void() > clearFunc );
        public Q_SLOTS:
            void slotLoadFinished( bool canceled );
            void slotProcessingStarted();

            void showResults();

        Q_SIGNALS:
            void sigLoading();
            void sigLoadFinished( bool canceled );
            void sigStartStayAwake();
            void sigStopStayAwake();
        private:
            void setupProgressDlg( const QString &title, const QString &cancelButtonText, int max );
            void clearProgressDlg();

            void loadSettings();
            void saveSettings();

            std::function< std::shared_ptr< CDoubleProgressDlg >( const QString &title, const QString &cancelButtonText, int max ) > fSetupProgressFunc;
            std::function< void() > fClearProgressFunc;
            std::shared_ptr< CDoubleProgressDlg > fProgressDlg;
            QString fDirName;
            std::unique_ptr< NCore::CDirModel > fModel;
            std::unique_ptr< Ui::CMergeSRTPage > fImpl;
        };
    }
}
#endif 
