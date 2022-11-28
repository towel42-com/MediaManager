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

#include "BasePage.h"
#include "ui_BasePage.h"

#include "Preferences/Core/Preferences.h"
#include "Models/DirModel.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/SetMKVTags.h"
#include "SABUtils/QtUtils.h"

#include <QSettings>
#include <QMenu>
#include <QTimer>
#include <QDesktopServices>
#include <QSortFilterProxyModel>

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
                     [this]()
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
            if ( ( obj == fImpl->log ) && ( event->type() == QEvent::MouseButtonPress ) )
            {
                //qDebug() << event;
                auto mouseEvent = dynamic_cast<QMouseEvent *>( event );
                if ( mouseEvent->buttons() & Qt::MouseButton::RightButton )
                {
                    auto menu = fImpl->log->createStandardContextMenu();
                    menu->addSeparator();
                    auto action = menu->addAction( tr( "Clear All" ) );
                    connect( action, &QAction::triggered, [this]()
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

        void CBasePage::slotPreferencesChanged( NPreferences::EPreferenceTypes prefTypes )
        {
            if ( prefTypes & NPreferences::eLoadPrefs )
            {
                if ( fModel )
                    fModel->reloadModel();
            }
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
            qApp->processEvents();
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
            load( false );
        }

        void CBasePage::load( bool postRun )
        {
            if ( !fImpl )
                return;
            if ( !postRun )
                fImpl->log->clear();
            if ( !fModel )
            {
                fModel.reset( createDirModel() );
                connect( fModel.get(), &NModels::CDirModel::sigDirLoadFinished, this, &CBasePage::slotLoadFinished );
                connect( fModel.get(), &NModels::CDirModel::sigProcessingStarted, this, &CBasePage::slotProcessingStarted );
                connect( fModel.get(), &NModels::CDirModel::sigProcessesFinished, this, &CBasePage::slotProcessesFinished );
                connect( fModel.get(), &NModels::CDirModel::sigDialogClosed, this, &CBasePage::sigDialogClosed );
            }
            appendSeparatorToLog();
            appendToLog( tr( "Loading Directory: '%1'" ).arg( fDirName ), true );
            appendSeparatorToLog();
            clearDirModel();

            filesView()->setModel( getDirModel() );
            setupModel();
            setupProgressDlg( loadTitleName(), loadCancelName(), 1, 1 );

            emit sigStartStayAwake();
            emit sigLoading();
        }

        void CBasePage::setupModel()
        {
            fModel->setRootPath( fDirName );
        }

        QString CBasePage::secondaryProgressLabel() const
        {
            return QString();
        }

        void CBasePage::postNonQueuedRun( bool /*finalStep*/, bool /*canceled*/ )
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
                load( true );
            emit sigStopStayAwake();
        }

        void CBasePage::run( const QModelIndex & idx )
        {
            auto actionName = actionTitleName();
            auto cancelName = actionCancelName();

            if ( fModel )
            {
                fModel->process( idx,
                                 [actionName, cancelName, this]( int count, int eventsPerPath )
                                 {
                                     setupProgressDlg( actionName, cancelName, count, eventsPerPath );
                                 },
                                 [this]( bool finalStep, bool canceled )
                                 {
                                     postNonQueuedRun( finalStep, canceled );
                                 },
                                     this );
            }
        }

        void CBasePage::showResults()
        {
            auto sizes = fImpl->vsplitter->sizes();
            if ( sizes.back() == 0 )
            {
                auto totalHeight = fImpl->vsplitter->size().height();
                auto fortyPercent = totalHeight * 40 / 100;
                sizes.front() -= fortyPercent;
                sizes.back() = fortyPercent;

                fImpl->vsplitter->setSizes( sizes );
            }
        }

        bool CBasePage::extendContextMenu( QMenu * /*menu*/, const QModelIndex & /*idx*/ )
        {
            return false;
        }

        bool CBasePage::progressCanceled() const
        {
            return fProgressDlg && fProgressDlg->wasCanceled();
        }

        void CBasePage::slotDoubleClicked( const QModelIndex & idx )
        {
            auto menu = menuForIndex( idx );
            if ( !menu )
                return;

            QTimer::singleShot( 0, [menu]()
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
                                                        [idx, this]()
                                                        {
                                                            openLocation( idx );
                                                        } );

                auto url = fModel->url( idx );
                if ( url.isValid() )
                {
                    openLocationAction = retVal->addAction( tr( "Open Internet Info..." ),
                                                            [url]()
                                                            {
                                                                QDesktopServices::openUrl( url );
                                                            } );
                }
                separator = retVal->addSeparator();
            }

            bool extended = extendContextMenu( retVal, idx );
            if ( !extended && separator )
                delete separator;

            if ( fModel->showMediaItemsContextMenu() && fModel->isMediaFile( idx ) )
            {
                retVal->addSeparator();
                retVal->addAction( tr( "Set Tags..." ),
                                   [idx, this]()
                                   {
                                       editMediaTags( idx );
                                   } );
                if ( !fModel->areMediaTagsSameAsAutoSet( idx ) )
                {
                    retVal->addAction( tr( "Auto Set Tags from File Name..." ),
                                       [idx, this]()
                                       {
                                           fModel->autoSetMediaTags( idx );
                                       } );
                }
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
            auto model = fImpl->filesView->model();
            if ( dynamic_cast<QSortFilterProxyModel *>( model ) )
                idx = dynamic_cast<QSortFilterProxyModel *>( model )->mapToSource( idx );

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
        {
        }

        void CBasePage::editMediaTags( const QModelIndex & idx )
        {
            auto fn = fModel->fileInfo( idx ).absoluteFilePath();
            NSABUtils::CSetMKVTags dlg( fn, NPreferences::NCore::CPreferences::instance()->getMKVPropEditEXE(), this );
            if ( dlg.exec() == QDialog::Accepted )
                fModel->reloadMediaTags( idx );
            emit sigDialogClosed();
        }

        QVBoxLayout * CBasePage::mainLayout() const
        {
            return fImpl->verticalLayout;
        }

        QAbstractItemModel * CBasePage::getDirModel() const
        {
            return fModel.get();
        }

        void CBasePage::clearDirModel()
        {
            if ( fModel )
                fModel->clear();
        }

    }
}

