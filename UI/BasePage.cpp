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

#include "ui_BasePage.h"
#include "BasePage.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/QtUtils.h"

#include <QSettings>
#include <QMenu>

namespace NMediaManager
{
    namespace NUi
    {
        CBasePage::CBasePage( const QString & pageName, QWidget * parent )
            : QWidget( parent ),
            fPageName( pageName ),
            fImpl( new Ui::CBasePage )
        {
            fImpl->setupUi( this );

            fImpl->filesView->setExpandsOnDoubleClick( false );
            fImpl->filesView->setContextMenuPolicy( Qt::CustomContextMenu );
            connect( fImpl->filesView, &QTreeView::doubleClicked, this, &CBasePage::slotDoubleClicked );
            connect( fImpl->filesView, &QTreeView::customContextMenuRequested, this, &CBasePage::slotContextMenu );
       }

        void CBasePage::postInit()
        {
            loadSettings();
        }

        void CBasePage::setProgressDlg( NSABUtils::CDoubleProgressDlg * progressDlg )
        {
            fProgressDlg = progressDlg;
            connect( fProgressDlg, &NSABUtils::CDoubleProgressDlg::canceled,
                     [ this ]()
            {
                clearProgressDlg();
            } );
        }

        CBasePage::~CBasePage()
        {
            saveSettings();
        }

        void CBasePage::loadSettings()
        {
            fImpl->vsplitter->setSizes( QList< int >() << 100 << 0 );
        }

        void CBasePage::saveSettings()
        {
            QSettings settings;
            settings.beginGroup( getPageName() );
            settings.setValue( "Splitter", fImpl->vsplitter->saveState() );
        }

        NSABUtils::CDoubleProgressDlg * CBasePage::progressDlg() const
        {
            return fProgressDlg;
        }

        QTreeView * CBasePage::filesView() const
        {
            return fImpl->filesView;
        }

        QPlainTextEdit * CBasePage::log() const
        {
            return fImpl->log;
        }

        void CBasePage::clearProgressDlg()
        {
            fProgressDlg->reset();
            fProgressDlg->hide();
        }

        void CBasePage::setupProgressDlg( const QString & title, const QString & cancelButtonText, int max )
        {
            fProgressDlg->reset();

            fProgressDlg->setSingleProgressBarMode( !canRun() || !useSecondaryProgressBar() );
            if ( canRun() && useSecondaryProgressBar() )
            {
                fProgressDlg->setSecondaryProgressLabel( secondaryProgressLabel() );
                fProgressDlg->setSecondaryRange( 0, 100 );
                fProgressDlg->setSecondaryValue( 0 );
            }

            fProgressDlg->setWindowModality( Qt::WindowModal );
            fProgressDlg->setMinimumDuration( 0 );
            fProgressDlg->setAutoClose( false );
            fProgressDlg->setAutoReset( false );

            fProgressDlg->setValue( 0 );
            fProgressDlg->setWindowTitle( title );
            fProgressDlg->setCancelButtonText( cancelButtonText );
            fProgressDlg->setRange( 0, max );
            fProgressDlg->show();
        }

        bool CBasePage::canRun() const
        {
            return fModel && fModel->rowCount() != 0;
        }

        void CBasePage::slotLoadFinished( bool canceled )
        {
            emit sigLoadFinished( canceled );
            emit sigStopStayAwake();

            postLoadFinished( canceled );
            clearProgressDlg();
        }

        void CBasePage::slotProcessingStarted()
        {
            showResults();
        }

        void CBasePage::load( const QString & dirName )
        {
            fDirName = dirName;
            load();
        }

        void CBasePage::load()
        {
            if ( !fModel )
                fModel.reset( createDirModel() );
            fModel->clear();
            fImpl->filesView->setModel( fModel.get() );
            connect( fModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CBasePage::slotLoadFinished );
            connect( fModel.get(), &NCore::CDirModel::sigProcessingStarted, this, &CBasePage::slotProcessingStarted );
            setupModel();
            setupProgressDlg( loadTitleName(), loadCancelName(), 1 );

            emit sigStartStayAwake();
            emit sigLoading();
        }

        void CBasePage::setupModel()
        {
            fModel->setNameFilters( dirModelFilter() );
            fModel->setRootPath( fDirName );
        }

        QString CBasePage::secondaryProgressLabel() const
        {
            return QString();
        }

        void CBasePage::postNonQueuedRun( bool /*finalStep*/ )
        {

        }

        void CBasePage::run()
        {
            emit sigStartStayAwake();

            auto actionName = actionTitleName();
            auto cancelName = actionCancelName();

            connect( fModel.get(), &NCore::CDirModel::sigProcessesFinished, [ this ]( bool status, bool canceled, bool reloadModel )
            {
                clearProgressDlg();
                if ( !status )
                {
                    fModel->showProcessResults( actionErrorName(), tr( "Issues:" ), QMessageBox::Critical, QDialogButtonBox::Ok, this );
                }
                if ( !canceled && reloadModel )
                    load();
                emit sigStopStayAwake();
            } );

            if ( fModel )
            {
                fModel->process(
                    [ actionName, cancelName, this ]( int count )
                {
                    setupProgressDlg( actionName, cancelName, count );
                },
                    [ this ]( bool finalStep )
                { 
                    postNonQueuedRun( finalStep );
                }, this );
            }
        }

        void CBasePage::showResults()
        {
            auto sizes = fImpl->vsplitter->sizes();
            if ( sizes.back() == 0 )
            {
                sizes.front() -= 30;
                sizes.back() = 30;

                fImpl->vsplitter->setSizes( sizes );
            }
        }

        void CBasePage::doubleClicked( const QModelIndex & /*idx*/ )
        {}

        QMenu * CBasePage::contextMenu( const QModelIndex & /*idx*/ )
        {
            return nullptr;
        }

        void CBasePage::slotDoubleClicked( const QModelIndex & idx )
        {
            doubleClicked( idx );
        }

        void CBasePage::slotContextMenu( const QPoint & pt )
        {
            auto idx = fImpl->filesView->indexAt( pt );
            if ( idx.isValid() )
            {
                auto menu = contextMenu( idx );
                if ( !menu )
                    return;
                menu->exec( fImpl->filesView->viewport()->mapToGlobal( pt ) );
            }
        }

        void CBasePage::appendToLog( const QString & msg, std::pair<QString, bool> & previousText, bool /*stdOut*/ )
        {
            showResults();

            QString realMessage = msg;
            if ( !realMessage.endsWith( "\n" ) )
                realMessage += "\n";

            NSABUtils::appendToLog( fImpl->log, realMessage, previousText );
            postProcessLog( realMessage );
        }

        void CBasePage::appendToLog( const QString & msg, bool stdOut )
        {
            return appendToLog( msg, stdOut ? fModel->stdOutRemaining() : fModel->stdErrRemaining(), stdOut );
        }

        void CBasePage::postProcessLog( const QString & /*string*/ )
        {
        }
    }
}

