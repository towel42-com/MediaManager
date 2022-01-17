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
#include "SetTags.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/QtUtils.h"

#include <QSettings>
#include <QMenu>
#include <QTimer>
#include <QDesktopServices>

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
            fProgressDlg = new NSABUtils::CDoubleProgressDlg( this );
            connect( fProgressDlg, &NSABUtils::CDoubleProgressDlg::canceled,
                     [ this ]()
            {
                clearProgressDlg( true );
            } );
            fProgressDlg->setMinimumDuration( -1 );

            fImpl->filesView->setExpandsOnDoubleClick( false );
            fImpl->filesView->setContextMenuPolicy( Qt::CustomContextMenu );
            connect( fImpl->filesView, &QTreeView::doubleClicked, this, &CBasePage::slotDoubleClicked );
            connect( fImpl->filesView, &QTreeView::customContextMenuRequested, this, &CBasePage::slotContextMenu );
            QTimer::singleShot( 0, this, &CBasePage::slotPostInit );

            fImpl->log->installEventFilter( this );
        }

        bool CBasePage::eventFilter( QObject * obj, QEvent * event )
        {
            if ( (obj == fImpl->log) && (event->type() == QEvent::MouseButtonPress ) )
            {
                //qDebug() << event;
                auto mouseEvent = dynamic_cast<QMouseEvent *>(event);
                if ( mouseEvent->buttons() & Qt::MouseButton::RightButton )
                {
                    auto menu = fImpl->log->createStandardContextMenu();
                    menu->addSeparator();
                    auto action = menu->addAction( tr( "Clear All" ) );
                    connect( action, &QAction::triggered, [ this ]()
                    {
                        fImpl->log->clear();;
                    } );
                    menu->exec( mouseEvent->globalPos() );
                    delete menu;
                    return true;
                }
            }
            return false;
        }

        void CBasePage::slotPostInit()
        {
            loadSettings();
        }

        CBasePage::~CBasePage()
        {
            saveSettings();
        }

        void CBasePage::loadSettings()
        {
            if ( fImpl )
                fImpl->vsplitter->setSizes( QList< int >() << 100 << 0 );
        }

        void CBasePage::saveSettings()
        {
            if ( !fImpl )
                return;

            QSettings settings;
            settings.beginGroup( getPageName() );
            settings.setValue( "Splitter", fImpl->vsplitter->saveState() );
        }

        QTreeView * CBasePage::filesView() const
        {
            if ( fImpl )
                return fImpl->filesView;
            return nullptr;
        }

        QPlainTextEdit * CBasePage::log() const
        {
            if ( fImpl )
                return fImpl->log;
            return nullptr;
        }

        void CBasePage::clearProgressDlg( bool canceled )
        {
            fProgressDlg->reset( canceled );
            fProgressDlg->hide();
        }

        void CBasePage::setupProgressDlg( const QString & title, const QString & cancelButtonText, int max, int eventsPerPath )
        {
            fProgressDlg->reset( false );

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
            fProgressDlg->setRange( 0, max * eventsPerPath );
            fProgressDlg->setPrimaryEventsPerIncrement( eventsPerPath );
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
            clearProgressDlg( canceled );
        }

        void CBasePage::slotProcessingStarted()
        {
            showResults();
            emit sigStartStayAwake();
        }

        void CBasePage::load( const QString & dirName )
        {
            fDirName = dirName;
            load();
        }

        void CBasePage::load()
        {
            if ( !fImpl )
                return;
            fImpl->log->clear();
            if ( !fModel )
            {
                fModel.reset( createDirModel() );
                connect( fModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CBasePage::slotLoadFinished );
                connect( fModel.get(), &NCore::CDirModel::sigProcessingStarted, this, &CBasePage::slotProcessingStarted );
                connect( fModel.get(), &NCore::CDirModel::sigProcessesFinished, this, &CBasePage::slotProcessesFinished );
            }
            appendSeparatorToLog();
            appendToLog( tr( "Loading Directory: '%1'" ).arg( fDirName ), true );
            appendSeparatorToLog();
            fModel->clear();
            filesView()->setModel( fModel.get() );
            setupModel();
            setupProgressDlg( loadTitleName(), loadCancelName(), 1, 1 );

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

        void CBasePage::slotProcessesFinished( bool status, bool showProcessResults, bool canceled, bool reloadModel )
        {
            clearProgressDlg( false );
            if ( !status && showProcessResults )
            {
                fModel->showProcessResults( actionErrorName(), tr( "Issues:" ), QMessageBox::Critical, QDialogButtonBox::Ok, this );
            }
            if ( !canceled && reloadModel )
                load();
            emit sigStopStayAwake();
        }

        void CBasePage::run( const QModelIndex & idx )
        {
            auto actionName = actionTitleName();
            auto cancelName = actionCancelName();

            if ( fModel )
            {
                fModel->process( idx,
                    [ actionName, cancelName, this ]( int count, int eventsPerPath )
                    {
                        setupProgressDlg( actionName, cancelName, count, eventsPerPath );
                    },
                    [ this ]( bool finalStep )
                    {
                        postNonQueuedRun( finalStep );
                    },
                    this );
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

        bool CBasePage::extendContextMenu( QMenu * /*menu*/, const QModelIndex & /*idx*/ )
        {
            return false;
        }

        void CBasePage::slotDoubleClicked( const QModelIndex & idx )
        {
            auto menu = menuForIndex( idx );
            if ( !menu )
                return;

            QTimer::singleShot( 0, [ menu ]()
            {
                if ( !menu )
                    return;
                auto defaultAction = menu->defaultAction();
                if ( defaultAction )
                    defaultAction->trigger();
                delete menu;
            } );
        }

        QMenu * CBasePage::menuForIndex( const QModelIndex & idx )
        {
            auto retVal = new QMenu( this );
            retVal->setObjectName( "Context Menu" );
            retVal->setTitle( tr( "Context Menu" ) );

            QAction * separator = nullptr;
            QAction * openLocationAction = nullptr;
            if ( idx.isValid() )
            {
                openLocationAction = retVal->addAction( tr( "Open Location..." ),
                                                        [ idx, this ]()
                {
                    openLocation( idx );
                } );
                separator = retVal->addSeparator();
            }

            bool extended = extendContextMenu( retVal, idx );
            if ( !extended && separator )
                delete separator;

            if ( fModel->showMediaItems() )
            {
                retVal->addSeparator();
                retVal->addAction( tr( "Set Tags..." ),
                                 [ idx, this ]()
                {
                    editMediaTags( idx );
                } );
            }

            if ( !retVal->defaultAction() && openLocationAction )
                retVal->setDefaultAction( openLocationAction );

            if ( !retVal->actions().count() )
            {
                delete retVal;
                retVal = nullptr;
            }
            return retVal;
        }

        void CBasePage::openLocation( const QModelIndex & idx )
        {
            auto fi = fModel->fileInfo( idx );
            if ( fi.exists() )
                QDesktopServices::openUrl( QUrl::fromLocalFile( fi.absoluteFilePath() ) );
        }

        void CBasePage::slotContextMenu( const QPoint & pt )
        {
            if ( !fImpl )
                return;

            auto idx = fImpl->filesView->indexAt( pt );
            auto menu = menuForIndex( idx );
            if ( menu )
                menu->exec( fImpl->filesView->viewport()->mapToGlobal( pt ) );
        }

        void CBasePage::appendSeparatorToLog()
        {
            appendToLog( "================================", true );
        }

        void CBasePage::appendToLog( const QString & msg, std::pair<QString, bool> & previousText, bool /*stdOut*/, bool fromProcess )
        {
            showResults();

            QString realMessage = msg;
            if ( !fromProcess && !realMessage.endsWith( "\n" ) )
                realMessage += "\n";

            NSABUtils::appendToLog( fImpl->log, realMessage, previousText );
            postProcessLog( realMessage );
        }

        void CBasePage::appendToLog( const QString & msg, bool stdOut )
        {
            Q_ASSERT( fModel );
            return appendToLog( msg, stdOut ? fModel->stdOutRemaining() : fModel->stdErrRemaining(), stdOut, false );
        }

        void CBasePage::postProcessLog( const QString & /*string*/ )
        {}

        void CBasePage::editMediaTags( const QModelIndex & idx )
        {
            auto fn = fModel->fileInfo( idx ).absoluteFilePath();
            CSetTags dlg( fn, this );
            dlg.exec();
            fModel->reloadMediaTags( idx );
        }
    }
}

