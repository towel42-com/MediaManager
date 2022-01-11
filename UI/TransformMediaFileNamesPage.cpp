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

#include "TransformMediaFileNamesPage.h"
#include "SelectTMDB.h"
#include "Core/Preferences.h"
#include "Core/TransformModel.h"
#include "Core/TransformResult.h"
#include "Core/SearchTMDBInfo.h"
#include "Core/SearchTMDB.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QTimer>
#include <QDir>
#include <QTreeView>
#include <QCoreApplication>
#include <QMenu>
#include <optional>

namespace NMediaManager
{
    namespace NUi
    {
        CTransformMediaFileNamesPage::CTransformMediaFileNamesPage( QWidget *parent )
            : CBasePage( "Transform", parent )
        {
            fSearchTMDB = new NCore::CSearchTMDB( nullptr, std::optional<QString>(), this );
            fSearchTMDB->setSkipImages( true );
            connect( fSearchTMDB, &NCore::CSearchTMDB::sigAutoSearchFinished, this, &CTransformMediaFileNamesPage::slotAutoSearchFinished );
        }

        CTransformMediaFileNamesPage::~CTransformMediaFileNamesPage()
        {
            saveSettings();
        }

        void CTransformMediaFileNamesPage::loadSettings()
        {
            setTreatAsTVByDefault( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            CBasePage::loadSettings();
        }

        NMediaManager::NCore::CTransformModel * CTransformMediaFileNamesPage::model()
        {
            if ( !fModel )
                return nullptr;

            return dynamic_cast<NCore::CTransformModel *>(fModel.get());
        }

        void CTransformMediaFileNamesPage::postNonQueuedRun( bool finalStep )
        {
            emit sigStopStayAwake();
            if ( finalStep )
                load();
        }

        void CTransformMediaFileNamesPage::setTreatAsTVByDefault( bool value )
        {
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( value );
        }

        void CTransformMediaFileNamesPage::setExactMatchesOnly( bool value )
        {
            NCore::CPreferences::instance()->setExactMatchesOnly( value );
        }

        QMenu * CTransformMediaFileNamesPage::contextMenu( const QModelIndex & idx )
        {
            if ( !idx.isValid() )
                return nullptr;

            auto menu = new QMenu( this );
            menu->setObjectName( "Context Menu" );
            menu->setTitle( tr( "Context Menu" ) );

            auto nm = model()->index( idx.row(), NCore::EColumns::eFSName, idx.parent() ).data().toString();
            auto action = menu->addAction( tr( "Search for '%1'..." ).arg( nm ), 
                             [ idx, this ]()
            {
                doubleClicked( idx );
            } );
            menu->setDefaultAction( action );

            auto searchResult = model()->getSearchResultInfo( idx );
            if ( searchResult )
            {
                menu->addAction( tr( "Clear Search Result" ),
                                 [ idx, this ]()
                {
                    model()->clearSearchResult( idx );
                } );
                menu->addAction( tr( "Transform Item..." ),
                                 [ idx, this ]()
                {
                    run( idx );
                } );
            }
            return menu;
        }

        void CTransformMediaFileNamesPage::doubleClicked( const QModelIndex & idx )
        {
            search( idx );
        }

        void CTransformMediaFileNamesPage::slotAutoSearchForNewNames()
        {
            if ( !model() || !model()->rowCount() )
            {
                emit sigLoadFinished( false );
                return;
            }

            Q_ASSERT( filesView()->model() == model() );
            fSearchTMDB->resetResults();

            auto count = NSABUtils::itemCount( model(), true );
            setupProgressDlg( tr( "Finding Results" ), tr( "Cancel" ), count, 1 );

            model()->computeEpisodesForDiskNumbers();

            auto rootIdx = model()->index( 0, 0 );
            bool somethingToSearchFor = autoSearchForNewNames( rootIdx );
            fProgressDlg->setValue( fSearchesCompleted );
            if ( !somethingToSearchFor )
            {
                emit sigLoadFinished( false );
                return;
            }
        }

        bool CTransformMediaFileNamesPage::autoSearchForNewNames( const QModelIndex & parentIdx )
        {
            bool retVal = false;
            auto rowCount = model()->rowCount( parentIdx );
            for ( int ii = 0; ii < rowCount; ++ii )
            {
                if ( fProgressDlg->wasCanceled() )
                {
                    fSearchTMDB->clearSearchCache();
                    break;
                }

                emit sigStartStayAwake();
                auto parentName = model()->getSearchName( parentIdx );

                auto childIndex = model()->index( ii, 0, parentIdx );
                auto name = model()->getSearchName( childIndex );
                if ( NCore::CPreferences::instance()->isPathToDelete( childIndex.data( NCore::ECustomRoles::eFullPathRole ).toString() ) )
                {
                    appendToLog( QString( "Deleting file '%1'" ).arg( childIndex.data( NCore::ECustomRoles::eFullPathRole ).toString() ), true );
                    model()->setDeleteItem( childIndex );
                }
                else
                {
                    auto path = model()->filePath( childIndex );
                    auto titleInfo = model()->getSearchResultInfo( childIndex );
                    auto searchInfo = std::make_shared< NCore::SSearchTMDBInfo >( name, titleInfo );
                    searchInfo->setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly() );

                    if ( model()->canAutoSearch( childIndex ) )
                    {
                        auto msg = tr( "Adding Background Search for '%1'" ).arg( QDir( fDirName ).relativeFilePath( path ) );
                        appendToLog( msg + QString( "\n\t%1\n" ).arg( searchInfo->toString( false ) ), true );
                        fProgressDlg->setLabelText( msg );
                        fProgressDlg->setValue( fProgressDlg->value() + 1 );
                        qApp->processEvents();

                        fSearchTMDB->addSearch( path, searchInfo );
                        retVal = true;
                    }
                }
                retVal = autoSearchForNewNames( childIndex ) || retVal;
            }
            return retVal;
        }

        void CTransformMediaFileNamesPage::slotAutoSearchFinished( const QString &path, bool searchesRemaining )
        {
            auto results = fSearchTMDB->getResult( path );

            searchesRemaining = searchesRemaining && !fProgressDlg->wasCanceled();

            auto msg = tr( "Search Complete for '%1'" ).arg( QDir( fDirName ).relativeFilePath( path ) );
            if ( searchesRemaining )
            {
                fProgressDlg->setValue( fProgressDlg->value() + 1 );
                fSearchesCompleted++;
                fProgressDlg->setLabelText( msg );
            }
            else
            {
                clearProgressDlg( fProgressDlg->wasCanceled() );
            }

            auto logMsg = QString( "\n\t" );
            if ( results.empty() )
                logMsg += tr( "Found: <No Match>" );
            else
            {
                if ( results.size() > 1 )
                    logMsg += tr( "Found %1 matches. Choosing %2: " ).arg( results.size() );
                else
                    logMsg += tr( "Found: %1" );
                logMsg = logMsg.arg( results.front()->toString( false ) );
            }

            appendToLog( msg + logMsg, true );

            if ( fProgressDlg->wasCanceled() )
                fSearchTMDB->clearSearchCache();
            else
            {
                if ( !results.empty() )
                {
                    auto item = model()->getItemFromPath( path );
                    if ( item )
                        model()->setSearchResult( item, results.front(), false );
                }
            }
            if ( !searchesRemaining )
            {
                emit sigLoadFinished( false );
                emit sigStopStayAwake();
            }
        }

        void CTransformMediaFileNamesPage::postLoadFinished( bool canceled )
        {
            if ( !canceled )
                QTimer::singleShot( 0, this, &CTransformMediaFileNamesPage::slotAutoSearchForNewNames );
        }

        NCore::CDirModel * CTransformMediaFileNamesPage::createDirModel()
        {
            return new NCore::CTransformModel( this );
        }

        QString CTransformMediaFileNamesPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CTransformMediaFileNamesPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CTransformMediaFileNamesPage::actionTitleName() const
        {
            return tr( "Renaming Files..." );
        }

        QString CTransformMediaFileNamesPage::actionCancelName() const
        {
            return tr( "Abort Rename" );
        }

        QString CTransformMediaFileNamesPage::actionErrorName() const
        {
            return tr( "Error While Creating MKV:" );
        }


        QStringList CTransformMediaFileNamesPage::dirModelFilter() const
        {
            auto retVal = NCore::CPreferences::instance()->getMediaExtensions() << NCore::CPreferences::instance()->getSubtitleExtensions();
            if ( NCore::CPreferences::instance()->deleteKnownPaths() )
                retVal << NCore::CPreferences::instance()->getPathsToDelete();
            return retVal;
        }

        void CTransformMediaFileNamesPage::setupModel()
        {
            model()->slotTVOutputFilePatternChanged( NCore::CPreferences::instance()->getTVOutFilePattern() );
            model()->slotTVOutputDirPatternChanged( NCore::CPreferences::instance()->getTVOutDirPattern() );
            model()->slotMovieOutputFilePatternChanged( NCore::CPreferences::instance()->getMovieOutFilePattern() );
            model()->slotMovieOutputDirPatternChanged( NCore::CPreferences::instance()->getMovieOutDirPattern() );

            CBasePage::setupModel();
        }

        void CTransformMediaFileNamesPage::search( const QModelIndex & idx )
        {
            auto baseIdx = model()->index( idx.row(), NCore::EColumns::eFSName, idx.parent() );
            auto titleInfo = model()->getSearchResultInfo( idx );

            auto isDir = baseIdx.data( NCore::ECustomRoles::eIsDir ).toBool();
            auto fullPath = baseIdx.data( NCore::ECustomRoles::eFullPathRole ).toString();
            bool isTVShow = baseIdx.data( NCore::ECustomRoles::eIsTVShowRole ).toBool();
            auto nm = model()->getSearchName( idx );

            CSelectTMDB dlg( nm, titleInfo, this );
            dlg.setSearchForTVShows( model()->treatAsTVShow( QFileInfo( fullPath ), isTVShow ), true );
            dlg.setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly(), true );

            if ( dlg.exec() == QDialog::Accepted )
            {
                auto titleInfo = dlg.getSearchResult();
                bool setChildren = true;
                if ( titleInfo->isTVShow() && titleInfo->isSeasonOnly() )
                    setChildren = false;
                model()->setSearchResult( idx, titleInfo, setChildren );
            }
        }

        QMenu * CTransformMediaFileNamesPage::menu()
        {
            if ( !fMenu )
            {
                fMenu = new QMenu( this );
                fMenu->setObjectName( "Media Namer Menu " );
                fMenu->setTitle( tr( "Media Namer" ) );
                connect( fMenu, &QMenu::aboutToShow, this, &CTransformMediaFileNamesPage::slotMenuAboutToShow );

                fExactMatchesOnlyAction = new QAction( this );
                fExactMatchesOnlyAction->setObjectName( QString::fromUtf8( "actionExactMatchesOnly" ) );
                fExactMatchesOnlyAction->setCheckable( true );
                fExactMatchesOnlyAction->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Exact Matches Only?", nullptr ) );
                connect( fExactMatchesOnlyAction, &QAction::triggered, this, &CTransformMediaFileNamesPage::slotExactMatchesOnly );

                fTreatAsTVShowByDefaultAction = new QAction( this );
                fTreatAsTVShowByDefaultAction->setObjectName( QString::fromUtf8( "actionTreatAsTVShowByDefault" ) );
                fTreatAsTVShowByDefaultAction->setCheckable( true );
                fTreatAsTVShowByDefaultAction->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Treat as TV Show by Default?", nullptr ) );
                connect( fTreatAsTVShowByDefaultAction, &QAction::triggered, this, &CTransformMediaFileNamesPage::slotTreatAsTVShowByDefault );

                fDeleteKnownPaths = new QAction( this );
                fDeleteKnownPaths->setObjectName( QString::fromUtf8( "actionDeleteKnownPaths" ) );
                fDeleteKnownPaths->setCheckable( true );
                fDeleteKnownPaths->setChecked( NCore::CPreferences::instance()->deleteKnownPaths() );
                fDeleteKnownPaths->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Delete Known Paths?", nullptr ) );
                connect( fDeleteKnownPaths, &QAction::triggered, [ this ]()
                {
                    fDeleteKnownPaths->setChecked( !fDeleteKnownPaths->isChecked() );
                    NCore::CPreferences::instance()->setDeleteKnownPaths( fDeleteKnownPaths->isChecked() );
                }
                );

                fMenu->addAction( fExactMatchesOnlyAction );
                fMenu->addAction( fTreatAsTVShowByDefaultAction );
                fMenu->addAction( fDeleteKnownPaths );
                setActive( true );
            }
            return fMenu;
        }

        void CTransformMediaFileNamesPage::setActive( bool isActive )
        {
            CBasePage::setActive( isActive );
            if ( isActive )
            {
                if ( fTreatAsTVShowByDefaultAction )
                    fTreatAsTVShowByDefaultAction->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
                if ( fExactMatchesOnlyAction )
                    fExactMatchesOnlyAction->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );
            }
            else
            {
                if ( fTreatAsTVShowByDefaultAction )
                    NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fTreatAsTVShowByDefaultAction->isChecked() );
                if ( fExactMatchesOnlyAction )
                    NCore::CPreferences::instance()->setExactMatchesOnly( fExactMatchesOnlyAction->isChecked() );
            }
        }

        void CTransformMediaFileNamesPage::slotMenuAboutToShow()
        {
            fTreatAsTVShowByDefaultAction->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fExactMatchesOnlyAction->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );
        }

        void CTransformMediaFileNamesPage::slotExactMatchesOnly()
        {
            NCore::CPreferences::instance()->setExactMatchesOnly( fExactMatchesOnlyAction->isChecked() );
            setExactMatchesOnly( NCore::CPreferences::instance()->getExactMatchesOnly() );
        }

        void CTransformMediaFileNamesPage::slotTreatAsTVShowByDefault()
        {
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fTreatAsTVShowByDefaultAction->isChecked() );
            setTreatAsTVByDefault( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
        }
    }
}


