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

#include "TransformMediaFileNamesPage.h"
#include "SelectTMDB.h"
#include "TransformConfirm.h"

#include "ui_TransformMediaFileNamesPage.h"

#include "Core/Preferences.h"
#include "Core/DirModel.h"
#include "Core/SearchResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"

#include "SABUtils/QtUtils.h"

#include <QProgressDialog>
#include <QTimer>
#include <QDir>

namespace NMediaManager
{
    namespace NUi
    {
        CTransformMediaFileNamesPage::CTransformMediaFileNamesPage( QWidget *parent )
            : QWidget( parent )
            ,fImpl( new Ui::CTransformMediaFileNamesPage )
        {
            fImpl->setupUi( this );

            fImpl->files->setExpandsOnDoubleClick( false );
            connect( fImpl->files, &QTreeView::doubleClicked, this, &CTransformMediaFileNamesPage::slotDoubleClicked );

            fSearchTMDB = new NCore::CSearchTMDB( nullptr, std::optional<QString>(), this );
            fSearchTMDB->setSkipImages( true );
            connect( fSearchTMDB, &NCore::CSearchTMDB::sigAutoSearchFinished, this, &CTransformMediaFileNamesPage::slotAutoSearchFinished );

            loadSettings();
        }

        CTransformMediaFileNamesPage::~CTransformMediaFileNamesPage()
        {
            saveSettings();
        }

        QTreeView*CTransformMediaFileNamesPage::mainView()
        {
            return fImpl->files;
        }

        void CTransformMediaFileNamesPage::loadSettings()
        {
            setTreatAsTVByDefault( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
        }

        void CTransformMediaFileNamesPage::saveSettings()
        {
        }

        void CTransformMediaFileNamesPage::setSetupProgressDlgFunc( std::function< QProgressDialog *( const QString &title, const QString &cancelButtonText, int max ) > setupFunc, std::function< void() > clearFunc )
        {
            fSetupProgressFunc = setupFunc;
            fClearProgressFunc = clearFunc;
        }

        void CTransformMediaFileNamesPage::setTreatAsTVByDefault( bool value )
        {
            if ( fModel )
                fModel->slotTreatAsTVByDefaultChanged( value );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( value );
        }

        void CTransformMediaFileNamesPage::setExactMatchesOnly( bool value )
        {
            NCore::CPreferences::instance()->setExactMatchesOnly( value );
        }

        void CTransformMediaFileNamesPage::slotAutoSearchForNewNames()
        {
            if ( !fModel || !fModel->rowCount() )
            {
                emit sigLoadFinished( false );
                return;
            }

            Q_ASSERT( fImpl->files->model() == fModel.get() );
            fSearchTMDB->resetResults();

            if ( fSetupProgressFunc )
            {
                auto count = NQtUtils::itemCount( fModel.get(), true );
                fProgressDlg = fSetupProgressFunc( tr( "Finding Results" ), tr( "Cancel" ), count );
            }

            auto rootIdx = fModel->index( 0, 0 );
            bool somethingToSearchFor = autoSearchForNewNames( rootIdx );
            fProgressDlg->setValue( fSearchesCompleted );
            if ( !somethingToSearchFor )
                 emit sigLoadFinished( false );
        }

        bool CTransformMediaFileNamesPage::autoSearchForNewNames( QModelIndex parentIdx )
        {
            bool retVal = false;
            auto rowCount = fModel->rowCount( parentIdx );
            for ( int ii = 0; ii < rowCount; ++ii )
            {
                if ( fProgressDlg->wasCanceled() )
                {
                    fSearchTMDB->clearSearchCache();
                    break;
                }

                auto childIndex = fModel->index( ii, 0, parentIdx );
                auto name = fModel->getSearchName( childIndex );
                auto path = fModel->filePath( childIndex );
                auto titleInfo = fModel->getSearchResultInfo( childIndex );
                auto searchInfo = std::make_shared< NCore::SSearchTMDBInfo >( name, titleInfo );
                searchInfo->setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly() );

                if ( fModel->canAutoSearch( childIndex ) )
                {
                    if ( fProgressDlg )
                    {
                        fProgressDlg->setLabelText( tr( "Adding Background Search for '%1'" ).arg( QDir( fDirName ).relativeFilePath( path ) ) );
                        fProgressDlg->setValue( fProgressDlg->value() + 1 );
                        qApp->processEvents();
                    }
                    fSearchTMDB->addSearch( path, searchInfo );
                    retVal = true;
                }

                retVal = autoSearchForNewNames( childIndex ) || retVal;
            }
            return retVal;
        }

        void CTransformMediaFileNamesPage::slotAutoSearchFinished( const QString &path, bool searchesRemaining )
        {
            auto result = fSearchTMDB->getResult( path );

            //qDebug().noquote().nospace() << "Search results for path " << path << " Has Result? " << ( ( results.size() == 1 ) ? "Yes" : "No" );
            if ( searchesRemaining )
            {
                if ( fProgressDlg )
                {
                    fProgressDlg->setValue( fProgressDlg->value() + 1 );
                    fSearchesCompleted++;
                    fProgressDlg->setLabelText( tr( "Search Complete for '%1'" ).arg( QDir( fDirName ).relativeFilePath( path ) ) );
                }
            }
            else
            {
                clearProgressDialog();
            }

            if ( fProgressDlg && fProgressDlg->wasCanceled() )
                fSearchTMDB->clearSearchCache();

            if ( !result )
            {
                if ( !searchesRemaining )
                    emit sigLoadFinished( false );
                return;
            }
            //qDebug() << result->toString();

            auto item = fModel->getItemFromPath( path );
            if ( item && result )
            {
                fModel->setSearchResult( item, result, false );
            }
            if ( !searchesRemaining )
                emit sigLoadFinished( false );
        }

        void CTransformMediaFileNamesPage::clearProgressDialog()
        {
            fProgressDlg = nullptr;
            if ( fClearProgressFunc )
                fClearProgressFunc();
        }

        void CTransformMediaFileNamesPage::setupProgressDialog( const QString &title, const QString &cancelButtonText, int max )
        {
            if ( fSetupProgressFunc )
                fProgressDlg = fSetupProgressFunc( title, cancelButtonText, max );
        }

        void CTransformMediaFileNamesPage::slotDoubleClicked( const QModelIndex &idx )
        {
            auto baseIdx = fModel->index( idx.row(), NCore::EColumns::eFSName, idx.parent() );
            auto titleInfo = fModel->getSearchResultInfo( idx );

            auto isDir = baseIdx.data( NCore::ECustomRoles::eIsDir ).toBool();
            auto fullPath = baseIdx.data( NCore::ECustomRoles::eFullPathRole ).toString();
            bool isTVShow = baseIdx.data( NCore::ECustomRoles::eIsTVShowRole ).toBool();
            auto nm = fModel->getSearchName( idx );

            CSelectTMDB dlg( nm, titleInfo, this );
            dlg.setSearchForTVShows( fModel->treatAsTVShow( QFileInfo( fullPath ), isTVShow ), true );
            dlg.setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly(), true );

            if ( dlg.exec() == QDialog::Accepted )
            {
                auto titleInfo = dlg.getSearchResult();
                bool setChildren = true;
                if ( titleInfo->isTVShow() && titleInfo->isSeasonOnly() )
                    setChildren = false;
                fModel->setSearchResult( idx, titleInfo, setChildren );
            }
        }

        void CTransformMediaFileNamesPage::slotLoadFinished( bool canceled )
        {
            if ( canceled )
            {
                emit sigLoadFinished( canceled );
                return;
            }

            QTimer::singleShot( 0, this, &CTransformMediaFileNamesPage::slotAutoSearchForNewNames );
        }

        void CTransformMediaFileNamesPage::load( const QString & dirName )
        {
            fDirName = dirName;
            load();
        }

        void CTransformMediaFileNamesPage::load()
        {
            fModel.reset( new NCore::CDirModel( NCore::CDirModel::eTransform ) );
            fImpl->files->setModel( fModel.get() );
            connect( fModel.get(), &NCore::CDirModel::sigDirReloaded, this, &CTransformMediaFileNamesPage::slotLoadFinished );
            fModel->slotTreatAsTVByDefaultChanged( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fModel->slotTVOutputFilePatternChanged( NCore::CPreferences::instance()->getTVOutFilePattern() );
            fModel->slotTVOutputDirPatternChanged( NCore::CPreferences::instance()->getTVOutDirPattern() );
            fModel->slotMovieOutputFilePatternChanged( NCore::CPreferences::instance()->getMovieOutFilePattern() );
            fModel->slotMovieOutputDirPatternChanged( NCore::CPreferences::instance()->getMovieOutDirPattern() );
            fModel->setNameFilters( NCore::CPreferences::instance()->getMediaExtensions() << NCore::CPreferences::instance()->getSubtitleExtensions(), fImpl->files );
            setupProgressDialog( tr( "Finding Files" ), tr( "Cancel" ), 1 );
            fModel->setRootPath( fDirName, fImpl->files, nullptr, fProgressDlg );
            emit sigLoading();
        }

        void CTransformMediaFileNamesPage::run()
        {
            NCore::CDirModel *model = nullptr;
            auto actionName = tr( "Renaming Files..." );
            auto cancelName = tr( "Abort Rename" );
            model = fModel.get();

            if ( fModel && fModel->process(
                [actionName, cancelName, this]( int count ) { setupProgressDialog( actionName, cancelName, count ); return fProgressDlg; },
                [this]( QProgressDialog *dlg ) { (void)dlg; clearProgressDialog(); },
                this ) )
            {
                load();
            }
        }

        bool CTransformMediaFileNamesPage::canRun() const
        {
            return fModel && fModel->rowCount() != 0;
        }
    }
}

