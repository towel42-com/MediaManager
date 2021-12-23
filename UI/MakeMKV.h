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

#ifndef _MAKEMKV_H
#define _MAKEMKV_H

#include <QWidget>
#include <optional>
//class CDoubleProgressDlg;
//class QTreeView;
//class QLineEdit;
//class CDelayLineEdit;
//class QToolBar;
//class QSpinBox;

//namespace NMediaManager
//{
//    namespace NCore
//    {
//        struct SSearchTMDBInfo;
//        struct SSearchResult;
//        class CDirModel;
//        class CSearchTMDB;
//    }
//}

namespace NMediaManager
{
    namespace NUi
    {
        namespace Ui { class CMakeMKV; };
        class CMakeMKV : public QWidget
        {
            Q_OBJECT
        public:
            CMakeMKV( QWidget *parent = 0 );
            ~CMakeMKV();

            void load( const QString &fileName );
            void load();

            void run();
            bool canRun() const;

            //void setTreatAsTVByDefault( bool value );
            //void setExactMatchesOnly( bool value );

            //void setSetupProgressDlgFunc( std::function< std::shared_ptr< CDoubleProgressDlg >( const QString &title, const QString &cancelButtonText, int max ) > setupFunc, std::function< void() > clearFunc );
        Q_SIGNALS:
            //void sigLoading();
            //void sigLoadFinished( bool canceled );
        public Q_SLOTS:
            //void slotDoubleClicked( const QModelIndex &idx );
            //void slotAutoSearchForNewNames();
            //void slotAutoSearchFinished( const QString &path, bool searchesRemaining );

            //void slotLoadFinished( bool canceled );
        private:
            //[[nodiscard]] bool autoSearchForNewNames( QModelIndex rootIdx );
            //void setupProgressDlg( const QString &title, const QString &cancelButtonText, int max );
            //void clearProgressDlg();

            //void loadSettings();
            //void saveSettings();

            //std::unique_ptr< NCore::CDirModel > fModel;
            //NCore::CSearchTMDB *fSearchTMDB{ nullptr };
            //std::function< std::shared_ptr< CDoubleProgressDlg >( const QString &title, const QString &cancelButtonText, int max ) > fSetupProgressFunc;
            //std::function< void() > fClearProgressFunc;
            //std::shared_ptr< CDoubleProgressDlg >fProgressDlg;
            //uint64_t fSearchesCompleted{ 0 };
            //QString fDirName;

            std::unique_ptr< Ui::CMakeMKV > fImpl;
        };
    }
}
#endif 