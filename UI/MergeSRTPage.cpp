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

#include "MergeSRTPage.h"
#include "ui_MergeSRTPage.h"

#include "Core/DirModel.h"
#include <QSettings>

namespace NMediaManager
{
    namespace NUi
    {
        CMergeSRTPage::CMergeSRTPage( QWidget *parent )
            : QWidget( parent ),
            fImpl( new Ui::CMergeSRTPage )
        {
            fImpl->setupUi( this );

            fImpl->files->setExpandsOnDoubleClick( false );

            loadSettings();
        }


        CMergeSRTPage::~CMergeSRTPage()
        {
            saveSettings();
        }

        void CMergeSRTPage::loadSettings()
        {
            QSettings settings;
            settings.beginGroup("Merge SRT");
            if ( settings.contains( "Splitter" ) )
                fImpl->splitter->restoreState( settings.value( "Splitter" ).toByteArray() );
            else
                fImpl->splitter->setSizes( QList< int >() << 100 << 0 );
        }

        void CMergeSRTPage::saveSettings()
        {
            QSettings settings;
            settings.beginGroup("Merge SRT");
            settings.setValue("Splitter", fImpl->splitter->saveState());
        }

        void CMergeSRTPage::setSetupProgressDlgFunc( std::function< QProgressDialog *( const QString &title, const QString &cancelButtonText, int max ) > setupFunc, std::function< void() > clearFunc )
        {
            fSetupProgressFunc = setupFunc;
            fClearProgressFunc = clearFunc;
        }

        void CMergeSRTPage::clearProgressDlg()
        {
            fProgressDlg = nullptr;
            if ( fClearProgressFunc )
                fClearProgressFunc();
        }

        void CMergeSRTPage::setupProgressDlg( const QString &title, const QString &cancelButtonText, int max )
        {
            if ( fSetupProgressFunc )
                fProgressDlg = fSetupProgressFunc( title, cancelButtonText, max );
        }

        bool CMergeSRTPage::canRun() const
        {
            return fModel && fModel->rowCount() != 0;
        }

        void CMergeSRTPage::slotLoadFinished( bool canceled )
        {
            if ( canceled )
            {
                emit sigLoadFinished( canceled );
                return;
            }

            emit sigLoadFinished( canceled );
        }

        void CMergeSRTPage::load( const QString &dirName )
        {
            fDirName = dirName;
            load();
        }

        void CMergeSRTPage::load()
        {
            fModel.reset( new NCore::CDirModel( NCore::CDirModel::eMergeSRT ) );
            fImpl->files->setModel( fModel.get() );
            connect( fModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CMergeSRTPage::slotLoadFinished );
            fModel->setNameFilters( QStringList() << "*.mkv", fImpl->files );
            setupProgressDlg( tr( "Finding Files" ), tr( "Cancel" ), 1 );
            fModel->setRootPath( fDirName, fImpl->files, fImpl->results, fProgressDlg );

            emit sigLoading();
        }

        void CMergeSRTPage::run()
        {
            NCore::CDirModel *model = nullptr;
            auto sizes = fImpl->splitter->sizes();
            if ( sizes.back() == 0 )
            {
                sizes.front() -= 30;
                sizes.back() = 30;

                fImpl->splitter->setSizes( sizes );
            }

            auto actionName = tr( "Merging SRT Files into MKV..." );
            auto cancelName = tr( "Abort Merge" );
            model = fModel.get();
            if ( model && model->process(
                [actionName, cancelName, this]( int count ) { setupProgressDlg( actionName, cancelName, count ); return fProgressDlg; },
                [this]( QProgressDialog *dlg ) { (void)dlg; clearProgressDlg(); },
                this ) )
            {
                load();
            }
        }
    }
}

