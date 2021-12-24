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

#include "ui_MakeMKVPage.h"
#include "MakeMKVPage.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QSettings>

namespace NMediaManager
{
    namespace NUi
    {
        CMakeMKVPage::CMakeMKVPage( QWidget * parent )
            : QWidget( parent ),
            fImpl( new Ui::CMakeMKVPage )
        {
            fImpl->setupUi( this );

            fImpl->files->setExpandsOnDoubleClick( false );

            loadSettings();
        }

        CMakeMKVPage::~CMakeMKVPage()
        {
            saveSettings();
        }

        void CMakeMKVPage::loadSettings()
        {
            fImpl->vsplitter->setSizes( QList< int >() << 100 << 0 );
        }

        void CMakeMKVPage::saveSettings()
        {
            QSettings settings;
            settings.beginGroup( "Make MKV" );
            settings.setValue( "Splitter", fImpl->vsplitter->saveState() );
        }

        void CMakeMKVPage::setSetupProgressDlgFunc( std::function< std::shared_ptr< CDoubleProgressDlg >( const QString & title, const QString & cancelButtonText, int max ) > setupFunc, std::function< void() > clearFunc )
        {
            fSetupProgressFunc = setupFunc;
            fClearProgressFunc = clearFunc;
        }

        void CMakeMKVPage::clearProgressDlg()
        {
            fProgressDlg = nullptr;
            if ( fClearProgressFunc )
                fClearProgressFunc();
        }

        void CMakeMKVPage::setupProgressDlg( const QString & title, const QString & cancelButtonText, int max )
        {
            if ( fSetupProgressFunc )
            {
                fProgressDlg = fSetupProgressFunc( title, cancelButtonText, max );
                fProgressDlg->setSingleProgressBarMode( !canRun() );

                if ( canRun() )
                {
                    fProgressDlg->setSecondaryProgressLabel( tr( "Current Movie:" ) );
                    fProgressDlg->setSecondaryRange( 0, 100 );
                    fProgressDlg->setSecondaryValue( 0 );
                }
            }
        }

        bool CMakeMKVPage::canRun() const
        {
            return fModel && fModel->rowCount() != 0;
        }

        void CMakeMKVPage::slotLoadFinished( bool canceled )
        {
            emit sigLoadFinished( canceled );
            emit sigStopStayAwake();
        }

        void CMakeMKVPage::slotProcessingStarted()
        {
            auto sizes = fImpl->vsplitter->sizes();
            if ( sizes.back() == 0 )
            {
                sizes.front() -= 30;
                sizes.back() = 30;

                fImpl->vsplitter->setSizes( sizes );
            }
        }

        void CMakeMKVPage::load( const QString & dirName )
        {
            fDirName = dirName;
            load();
        }

        void CMakeMKVPage::load()
        {
            fModel.reset( new NCore::CDirModel( NCore::CDirModel::eMakeMKV ) );
            fImpl->files->setModel( fModel.get() );
            connect( fModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMakeMKVPage::slotLoadFinished );
            connect( fModel.get(), &NCore::CDirModel::sigProcessingStarted, this, &CMakeMKVPage::slotProcessingStarted );
            fModel->setNameFilters( NCore::CPreferences::instance()->getNonMKVMediaExtensions(), fImpl->files, fImpl->results );
            setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
            fModel->setRootPath( fDirName, fImpl->files, fImpl->results, fProgressDlg );

            emit sigStartStayAwake();
            emit sigLoading();
        }

        void CMakeMKVPage::run()
        {
            emit sigStartStayAwake();

            auto actionName = tr( "Creating MKV..." );
            auto cancelName = tr( "Abort Creating MKV" );

            connect( fModel.get(), &NCore::CDirModel::sigProcessesFinished, [ this ]( bool status, bool canceled, bool reloadModel )
            {
                clearProgressDlg();
                if ( !status )
                {
                    fModel->showProcessResults( tr( "Error While Creating MKV:" ), tr( "Issues:" ), QMessageBox::Critical, QDialogButtonBox::Ok, this );
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
                    return fProgressDlg;
                },
                    [ this ]( std::shared_ptr< CDoubleProgressDlg > dlg ) { (void)dlg; }, this );
            }
        }
    }
}

