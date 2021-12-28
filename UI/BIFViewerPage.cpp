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

#include "BIFViewerPage.h"

#include "ui_BIFViewerPage.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "Core/SearchResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/utils.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/BIFFile.h"
#include "SABUtils/BIFModel.h"
#include "SABUtils/BIFScrollBar.h"
#include "SABUtils/DelayLineEdit.h"

#include <QSettings>
#include <QFileInfo>
#include <QFileDialog>
#include <QCompleter>
#include <QMessageBox>
#include <QFileSystemModel>
#include <QTimer>
#include <QPixmap>
#include <QLabel>


namespace NMediaManager
{
    namespace NUi
    {
        CBIFViewerPage::CBIFViewerPage( QWidget *parent )
            : QWidget( parent ),
            fImpl( new Ui::CBIFViewerPage )
        {
            fImpl->setupUi( this );
            fBIFScrollBar = new NSABUtils::NBIF::CScrollBar(Qt::Vertical);
            fImpl->bifImages->setVerticalScrollBar( fBIFScrollBar );
            fBIFScrollBar->setRange(0, 0);
            QObject::connect(fBIFScrollBar, SIGNAL(actionTriggered(int)), fImpl->bifImages, SLOT(verticalScrollbarAction(int)));
            QObject::connect(fBIFScrollBar, SIGNAL(valueChanged(int)), fImpl->bifImages, SLOT(verticalScrollbarValueChanged(int)));

            clear();

            connect( fImpl->bifWidget, &NSABUtils::NBIF::CWidget::sigPlayingStarted, this, &CBIFViewerPage::slotPlayingStarted );

            loadSettings( true );

            fResizeTimer = new QTimer( this );
            fResizeTimer->setSingleShot( true );
            fResizeTimer->setInterval( 250 );
            connect( fResizeTimer, &QTimer::timeout, this, &CBIFViewerPage::slotResize );

            fImpl->bifImages->installEventFilter( this );
        }


        CBIFViewerPage::~CBIFViewerPage()
        {
            saveSettings();
        }


        void CBIFViewerPage::formatBIFTable()
        {
            fImpl->bifFileValues->resizeColumnToContents( 0 );
            fImpl->bifFileValues->resizeColumnToContents( 1 );
            for ( int ii = 0; ii < fImpl->bifFileValues->topLevelItemCount(); ++ii )
            {
                fImpl->bifFileValues->topLevelItem( ii )->setTextAlignment( 1, Qt::AlignRight | Qt::AlignVCenter );
            }
        }

        void CBIFViewerPage::loadSettings( bool init )
        {
            if ( init )
            {
                QSettings settings;
                if ( settings.contains( "bifViewerVSplitter" ) )
                    fImpl->bifViewerVSplitter->restoreState( settings.value( "bifViewerVSplitter" ).toByteArray() );
                else
                    fImpl->bifViewerVSplitter->setSizes( QList< int >() << 100 << 100 );

                fImpl->bifViewerHSplitter->setSizes( QList< int >() << 100 << 0 );
                setButtonsLayout( static_cast<NSABUtils::NBIF::EButtonsLayout>( settings.value( "bifPlayerButtonLayout", static_cast<int>(NSABUtils::NBIF::EButtonsLayout::eTogglePlayPause ) ).toInt() ) );
            }

            fImpl->bifWidget->setSpeedMultiplier( NCore::CPreferences::instance()->bifPlayerSpeedMultiplier() );
            fImpl->bifWidget->setNumFramesToSkip( NCore::CPreferences::instance()->bifNumFramesToSkip() );
            fImpl->bifWidget->setPlayCount(NCore::CPreferences::instance()->bifLoopCount());
            fImpl->bifWidget->setGIFFlipImage( NCore::CPreferences::instance()->gifFlipImage() );
            fImpl->bifWidget->setGIFDitherImage( NCore::CPreferences::instance()->gifDitherImage() );
            fImpl->bifWidget->setGIFLoopCount( NCore::CPreferences::instance()->gifLoopCount() );
            fImpl->bifWidget->setGIFStartFrame( NCore::CPreferences::instance()->gifStartFrame() );
            fImpl->bifWidget->setGIFEndFrame( NCore::CPreferences::instance()->gifEndFrame() );
            fImpl->bifWidget->setGIFDelay( NCore::CPreferences::instance()->gifDelay() );
        }

        void CBIFViewerPage::saveSettings()
        {
            QSettings settings;
            settings.setValue( "bifViewerVSplitter", fImpl->bifViewerVSplitter->saveState() );
            settings.setValue( "bifPlayerButtonLayout", static_cast<int>( fImpl->bifWidget->buttonsLayout() ) );

            NCore::CPreferences::instance()->setBIFPlayerSpeedMultiplier( fImpl->bifWidget->playerSpeedMultiplier() );
            NCore::CPreferences::instance()->setBIFNumFramesToSkip( fImpl->bifWidget->numFramesToSkip() );
            NCore::CPreferences::instance()->setBIFLoopCount(fImpl->bifWidget->playCount());
            NCore::CPreferences::instance()->setGIFFlipImage( fImpl->bifWidget->gifFlipImage() );
            NCore::CPreferences::instance()->setGIFDitherImage( fImpl->bifWidget->gifDitherImage() );
            NCore::CPreferences::instance()->setGIFLoopCount( fImpl->bifWidget->gifLoopCount() );
            NCore::CPreferences::instance()->setGIFStartFrame( fImpl->bifWidget->gifStartFrame() );
            NCore::CPreferences::instance()->setGIFEndFrame( fImpl->bifWidget->gifEndFrame() );
            NCore::CPreferences::instance()->setGIFDelay( fImpl->bifWidget->gifDelay() );
        }

        bool CBIFViewerPage::eventFilter( QObject * obj, QEvent * event )
        {
            if ( ( obj == fImpl->bifImages ) && event->type() == QEvent::Resize )
            {
                fResizeTimer->stop();
                fResizeTimer->start();
            }
            return QWidget::eventFilter( obj, event );
        }

        QToolBar *CBIFViewerPage::toolBar()
        {
            return fImpl->bifWidget->toolBar();
        }

        QMenu *CBIFViewerPage::menu()
        {
            return fImpl->bifWidget->menu();
        }

        void CBIFViewerPage::slotResize()
        {
            auto windowSize = 1.0 * fImpl->bifImages->size().width();
            auto itemSize = 1.0 * fImpl->bifImages->iconSize().width();
            itemSize += 16.0; 

            auto num = std::floor( windowSize / itemSize );

            auto numPerRow = 4;
            if ( num != numPerRow )
            {
                itemSize = std::ceil( windowSize / numPerRow ) - 16;
            }

            fImpl->bifImages->setIconSize( QSize( itemSize, itemSize ) );
        }

        bool CBIFViewerPage::outOfDate() const
        {
            return ( !fBIF || ( fBIF->fileName() != fFileName ) || ( fImpl->bifWidget->fileName() != fFileName ) );
        }

        void CBIFViewerPage::slotPlayingStarted()
        {
            if ( !fImpl->bifViewerHSplitter->sizes().back() )
            {
                auto sizes = fImpl->bifViewerHSplitter->sizes();
                sizes.front() -= ( fImpl->bifWidget->sizeHint().width() + 16 );
                sizes.back() = fImpl->bifWidget->sizeHint().width() + 16;
                fImpl->bifViewerHSplitter->setSizes( sizes );
            }
        }

        void CBIFViewerPage::setActive( bool isActive )
        {
            if ( !isActive )
                fImpl->bifWidget->slotPause();
            else
                fileNameChanged();

            fImpl->bifWidget->setActive( isActive );
        }

        bool CBIFViewerPage::setFileName( const QString &fileName, bool andExecute )
        {
            if ( fileName.isEmpty() || !QFileInfo( fileName ).exists() )
                return false;

            fFileName = fileName;
            fileNameChanged();
            if ( andExecute )
                fImpl->bifWidget->slotPlay();
            return true;
        }

        void CBIFViewerPage::fileNameChanged()
        {
            NSABUtils::CAutoWaitCursor awc;
            if ( !outOfDate() )
            {
                return;
            }

            auto aOK = canLoad();
            clear();
            fImpl->bifWidget->clear();
            fImpl->bifWidget->validatePlayerActions( false );
            fImpl->bifWidget->slotPause();
            if ( !aOK )
                return;

            slotResize();

            fBIF = fImpl->bifWidget->setFileName(fFileName);
            if ( !fImpl->bifWidget->isValid() )
            {
                auto msg = fBIF ? tr( "Could not load BIF File: %1" ).arg( fBIF->errorString() ) : tr( "Could not load BIF File" );
                QMessageBox::warning( this, tr( "Could not Load" ), msg );
                fBIF.reset();
                return;
            }
            load();
        }

        void CBIFViewerPage::load()
        {
            if ( !fBIF )
                return;

            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Magic Number" ) << tr( "00-07" ) << QString() << fBIF->magicNumber() );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Version" ) << tr( "08-11" ) << QString::number( std::get< 2 >( fBIF->version() ) ) << std::get< 1 >( fBIF->version() ) );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Number of BIF Images" ) << tr( "12-15" ) << QString::number( std::get< 2 >( fBIF->numImages() ) ) << std::get< 1 >( fBIF->numImages() ) );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "milliseconds/Frame" ) << tr( "16-19" ) << QString( "%1s (%2ms)" ).arg(NSABUtils::CTimeString( std::get< 2 >( fBIF->tsMultiplier() ) ).toString( "ss.zzz" ) ).arg( std::get< 2 >( fBIF->tsMultiplier() ) ) << std::get< 1 >( fBIF->tsMultiplier() ) );
            new QTreeWidgetItem( fImpl->bifFileValues, QStringList() << tr( "Reserved" ) << tr( "20-64" ) << QString() << fBIF->reserved() );

            formatBIFTable();

            fBIFModel->setBIFFile( fBIF );
            fBIFScrollBar->setBIFFile(fBIF);
        }

        bool CBIFViewerPage::canLoad() const
        {
            NSABUtils::CAutoWaitCursor awc;

            auto bifFile = fFileName;
            auto fi = QFileInfo( bifFile );
            bool aOK = !bifFile.isEmpty() && fi.exists() && fi.isFile() && fi.isReadable();
            return aOK;
        }

        void CBIFViewerPage::clear()
        {
            fImpl->bifFileValues->clear();
            fImpl->bifFileValues->setHeaderLabels( QStringList() << tr( "Name" ) << tr( "Byte #s" ) << tr( "Value" ) );

            fImpl->bifWidget->clear();

            formatBIFTable();
            delete fBIFModel;
            fBIFModel = new NSABUtils::NBIF::CModel( this );
            fImpl->bifImages->setModel( fBIFModel );
        }

        void CBIFViewerPage::setButtonsLayout(NSABUtils::NBIF::EButtonsLayout layout )
        {
            fImpl->bifWidget->setButtonsLayout( layout );
        }

        NSABUtils::NBIF::EButtonsLayout CBIFViewerPage::buttonsLayout() const
        {
            return fImpl->bifWidget->buttonsLayout();
        }

        QAction *CBIFViewerPage::actionSkipBackward()
        {
            return fImpl->bifWidget->actionSkipBackward();
        }
        
        QAction *CBIFViewerPage::actionPrev() 
        { 
            return fImpl->bifWidget->actionPrev(); 
        }
        
        QAction *CBIFViewerPage::actionTogglePlayPause( std::optional< bool > asPlayButton )
        {
            return fImpl->bifWidget->actionTogglePlayPause( asPlayButton );
        }

        QAction *CBIFViewerPage::actionPause() 
        { 
            return fImpl->bifWidget->actionPause(); 
        }
        QAction *CBIFViewerPage::actionPlay()
        {
            return fImpl->bifWidget->actionPlay();
        }

        QAction *CBIFViewerPage::actionNext()
        {
            return fImpl->bifWidget->actionNext();
        }

        QAction *CBIFViewerPage::actionSkipForward()
        {
            return fImpl->bifWidget->actionSkipForward();
        }
    }
}

