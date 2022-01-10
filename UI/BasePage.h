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

#ifndef _BASEPAGE_H
#define _BASEPAGE_H

#include <QWidget>
namespace NSABUtils
{
    class CDoubleProgressDlg;
}
class QTreeView;
class QPlainTextEdit;
class QMenu;
class QToolBar;

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
        namespace Ui { class CBasePage; };
        class CBasePage : public QWidget
        {
            Q_OBJECT
        public:
            CBasePage( const QString & pageName, QWidget *parent = 0 );
            ~CBasePage();

            virtual void load( const QString & dirName );
            virtual void load();

            virtual void run( const QModelIndex & idx );
            virtual bool canRun() const;

            QTreeView * filesView() const;
            QPlainTextEdit * log() const;

            virtual void appendSeparator();
            virtual void appendToLog( const QString & msg, bool stdOut );
            virtual void appendToLog( const QString & msg, std::pair<QString, bool> & previousText, bool stdOut );

            virtual void postProcessLog( const QString & string );
            virtual void doubleClicked( const QModelIndex & idx );
            virtual QMenu * contextMenu( const QModelIndex & idx );

            NSABUtils::CDoubleProgressDlg * progressDlg() const { return fProgressDlg; }

            virtual QMenu * menu() { return nullptr; }
            virtual QToolBar * toolBar() { return nullptr; }
            virtual void setActive( bool isActive ) { fIsActive = isActive; }
            virtual bool isActive() const { return fIsActive; }
            virtual bool isFileBased() const { return false; }
            virtual bool isDirBased() const { return true; }

            virtual QString selectFileFilter() const { return QString(); }
            virtual bool eventFilter( QObject * obj, QEvent * event ) override;
        public Q_SLOTS:
            void slotLoadFinished( bool canceled );
            void slotProcessingStarted();

            void showResults();
            virtual void slotDoubleClicked( const QModelIndex & idx ) final;
            virtual void slotContextMenu( const QPoint & pt ) final;
        protected Q_SLOTS:
            virtual void slotPostInit();
        Q_SIGNALS:
            void sigLoading();
            void sigLoadFinished( bool canceled );
            void sigStartStayAwake();
            void sigStopStayAwake();
        protected:
            virtual QString getPageName() const final { return fPageName; }
            virtual NCore::CDirModel * createDirModel() = 0;
            virtual bool useSecondaryProgressBar() const { return false; }
            virtual QString secondaryProgressLabel() const;

            virtual QStringList dirModelFilter() const = 0;

            virtual QString loadTitleName() const = 0;
            virtual QString loadCancelName() const = 0;

            virtual QString actionTitleName() const = 0;
            virtual QString actionCancelName() const = 0;
            virtual QString actionErrorName() const = 0;
            virtual void postNonQueuedRun( bool finalStep );
            virtual void postLoadFinished( bool /*canceled*/ ) {}
            virtual void setupModel();

            void setupProgressDlg( const QString & title, const QString & cancelButtonText, int max, int eventsPerPath=1);
            void clearProgressDlg( bool canceled );

            virtual void loadSettings();
            virtual void saveSettings();

            QString fPageName;
            QString fDirName;

            bool fIsActive{ false };
            
            NSABUtils::CDoubleProgressDlg * fProgressDlg{ nullptr };
            std::unique_ptr< NCore::CDirModel > fModel;
            std::unique_ptr< Ui::CBasePage > fImpl;
        };
    }
}
#endif 
