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

#include "TransformPage.h"
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
        CTransformPage::CTransformPage( QWidget *parent )
            : CBasePage( "Transform", parent )
        {
            fSearchTMDB = new NCore::CSearchTMDB( nullptr, std::optional<QString>(), this );
            fSearchTMDB->setSkipImages( true );
            connect( fSearchTMDB, &NCore::CSearchTMDB::sigAutoSearchFinished, this, &CTransformPage::slotAutoSearchFinished );
        }

        CTransformPage::~CTransformPage()
        {
            saveSettings();
        }

        void CTransformPage::loadSettings()
        {
            CBasePage::loadSettings();
        }

        NMediaManager::NCore::CTransformModel * CTransformPage::model()
        {
            if ( !fModel )
                return nullptr;

            return dynamic_cast<NCore::CTransformModel *>(fModel.get());
        }

        void CTransformPage::postNonQueuedRun( bool finalStep, bool canceled )
        {
            emit sigStopStayAwake();
            if ( finalStep && !canceled )
                load();
        }

        bool CTransformPage::extendContextMenu( QMenu * menu, const QModelIndex & idx )
        {
            if ( !idx.isValid() )
                return false;

            auto nameIdx = model()->index( idx.row(), NCore::EColumns::eFSName, idx.parent() );

            auto nm = nameIdx.data().toString();
            auto action = menu->addAction( tr( "Search for '%1'..." ).arg( nm ),
                [ nameIdx, this ]()
                {
                    manualSearch( nameIdx );
                } );
            menu->setDefaultAction( action );

            if ( model()->canAutoSearch( nameIdx, false ) )
            {
                action = menu->addAction( tr( "Auto-Search for '%1'" ).arg( nm ),
                    [ nameIdx, this ]()
                    {
                        autoSearchForNewNames( nameIdx, false, {} );
                    } );
            }
            menu->addSeparator();

            auto searchResult = model()->getTransformResult( nameIdx, false );
            if ( searchResult )
            {
                menu->addAction( tr( "Clear Search Result" ),
                    [ nameIdx, this ]()
                    {
                        model()->clearSearchResult( nameIdx, false );
                    } );
                if ( model()->rowCount( nameIdx ) )
                {
                    menu->addAction( tr( "Clear Search Result (Including Children)" ),
                        [ nameIdx, this ]()
                        {
                            model()->clearSearchResult( nameIdx, true );
                        } );

                    menu->addAction( tr( "Apply Search Results to Children" ),
                        [ nameIdx, searchResult, this]()
                        {
                            model()->setSearchResult(nameIdx, searchResult, true, true);
                        });
                }
                menu->addSeparator();
                menu->addAction(tr("Transform Item..."),
                    [ nameIdx, this ]()
                    {
                        run( nameIdx );
                    } );
            }
            return true;
        }

        void CTransformPage::slotAutoSearchForNewNames()
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

            model()->setInAutoSearch(true);
            if (filesView())
                filesView()->expandAll();

            auto rootIdx = model()->index( 0, 0 );
            bool somethingToSearchFor = autoSearchForNewNames( rootIdx, true, {} );
            fProgressDlg->setValue( fSearchesCompleted );
            if ( !somethingToSearchFor )
            {
                emit sigLoadFinished( false );
                clearProgressDlg( false );
                return;
            }
        }

        bool CTransformPage::autoSearchForNewNames( const QModelIndex & index, bool searchChildren, std::optional< NCore::EMediaType > mediaType )
        {
            bool retVal = false;

            emit sigStartStayAwake();
            auto parentName = model()->getSearchName( index );

            auto name = model()->getSearchName( index );
            if ( NCore::CPreferences::instance()->isPathToDelete( index.data( NCore::ECustomRoles::eFullPathRole ).toString() ) )
            {
                appendToLog( QString( "Deleting file '%1'" ).arg( index.data( NCore::ECustomRoles::eFullPathRole ).toString() ), true );
                model()->setDeleteItem( index );
            }
            else
            {
                if (model()->canAutoSearch(index, false))
                {
                    auto path = model()->filePath(index);
                    auto titleInfo = model()->getTransformResult( index, false );
                    auto searchInfo = std::make_shared< NCore::SSearchTMDBInfo >( name, titleInfo );
                    searchInfo->setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly() );
                    if ( mediaType.has_value() )
                        searchInfo->setMediaType( mediaType.value() );

                    auto msg = tr( "Adding Background Search for '%1'" ).arg( QDir( fDirName ).relativeFilePath( path ) );
                    appendToLog( msg + QString( "\n\t%1\n" ).arg( searchInfo->toString( false ) ), true );
                    fProgressDlg->setLabelText( msg );
                    fProgressDlg->setValue( fProgressDlg->value() + 1 );
                    qApp->processEvents();

                    fSearchTMDB->addSearch( path, searchInfo );
                    retVal = true;
                }
            }
            auto rowCount = model()->rowCount( index );
            for ( int ii = 0; searchChildren && ( ii < rowCount ); ++ii )
            {
                if (progressCanceled())
                {
                    fSearchTMDB->clearSearchCache();
                    break;
                }
                auto childIndex = model()->index( ii, 0, index );
                retVal = autoSearchForNewNames( childIndex, searchChildren, mediaType ) || retVal;
            }
            return retVal;
        }

        void CTransformPage::slotAutoSearchFinished( const QString &path, NCore::SSearchTMDBInfo * searchInfo, bool searchesRemaining )
        {
            auto results = fSearchTMDB->getResult( path );
            if ( !progressCanceled() && results.empty() && searchInfo && searchInfo->mediaTypeAutoDetermined() )
            {
                auto item = model()->getItemFromPath( path );
                auto index = item ? model()->indexFromItem( item ) : QModelIndex();
                if ( index.isValid() )
                {
                    auto currMediaType = searchInfo->mediaType();
                    auto newMediaType = (currMediaType == NCore::EMediaType::eMovie) ? NCore::EMediaType::eTVShow : NCore::EMediaType::eMovie;
                    autoSearchForNewNames( index, false, newMediaType );
                    searchesRemaining = true;
                    fProgressDlg->setMaximum( fProgressDlg->primaryMax() + 1 );
                }
            }

            searchesRemaining = searchesRemaining && !progressCanceled();

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
                model()->setInAutoSearch(false);
            }

            auto logMsg = QString( "\n\t" );
            if ( results.empty() )
                logMsg += tr( "Found: <No Match>" );
            else
            {
                if ( results.size() > 1 )
                    logMsg += tr( "Found %1 matches. Choosing %2." ).arg( results.size() );
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
                        model()->setSearchResult( item, results.front(), false, false );
                }
            }
            if ( !searchesRemaining )
            {
                emit sigLoadFinished( false );
                emit sigStopStayAwake();
                if (filesView())
                    filesView()->expandAll();
            }
        }

        void CTransformPage::postLoadFinished( bool canceled )
        {
            if ( !canceled )
                QTimer::singleShot( 0, this, &CTransformPage::slotAutoSearchForNewNames );
        }

        NCore::CDirModel * CTransformPage::createDirModel()
        {
            return new NCore::CTransformModel( this );
        }

        QString CTransformPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CTransformPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CTransformPage::actionTitleName() const
        {
            return tr( "Renaming Files..." );
        }

        QString CTransformPage::actionCancelName() const
        {
            return tr( "Abort Rename" );
        }

        QString CTransformPage::actionErrorName() const
        {
            return tr( "Error while Transforming Media:" );
        }


        QStringList CTransformPage::dirModelFilter() const
        {
            auto retVal = NCore::CPreferences::instance()->getMediaExtensions() << NCore::CPreferences::instance()->getSubtitleExtensions();
            retVal << NCore::CPreferences::instance()->getExtensionsToDelete();
            return retVal;
        }

        void CTransformPage::setupModel()
        {
            model()->slotTVOutputFilePatternChanged( NCore::CPreferences::instance()->getTVOutFilePattern() );
            model()->slotTVOutputDirPatternChanged( NCore::CPreferences::instance()->getTVOutDirPattern() );
            model()->slotMovieOutputFilePatternChanged( NCore::CPreferences::instance()->getMovieOutFilePattern() );
            model()->slotMovieOutputDirPatternChanged( NCore::CPreferences::instance()->getMovieOutDirPattern() );

            CBasePage::setupModel();
        }

        void CTransformPage::manualSearch( const QModelIndex & idx )
        {
            auto baseIdx = model()->index( idx.row(), NCore::EColumns::eFSName, idx.parent() );
            auto titleInfo = model()->getTransformResult( idx, true );

            auto isDir = baseIdx.data( NCore::ECustomRoles::eIsDir ).toBool();
            auto fullPath = baseIdx.data( NCore::ECustomRoles::eFullPathRole ).toString();
            bool isTVShow = baseIdx.data( NCore::ECustomRoles::eIsTVShowRole ).toBool();
            auto nm = model()->getSearchName( idx );

            CSelectTMDB dlg( nm, titleInfo, this );
            dlg.setSearchForTVShows( model()->treatAsTVShow( QFileInfo( fullPath ), isTVShow ), true );
            dlg.setExactMatchOnly( NCore::CPreferences::instance()->getExactMatchesOnly(), true );

            if ( dlg.exec() == QDialog::Accepted )
            {
                auto results = dlg.getSearchResult();
                bool setChildren = true;
                if ( results->isTVShow() && results->isSeasonOnly() )
                    setChildren = false;
                model()->setSearchResult( idx, results, setChildren, true );
            }
        }

        QMenu * CTransformPage::menu()
        {
            if ( !fMenu )
            {
                fMenu = new QMenu( this );
                fMenu->setObjectName( "Media Namer Menu " );
                fMenu->setTitle( tr( "Media Namer" ) );
                connect( fMenu, &QMenu::aboutToShow, this, &CTransformPage::slotMenuAboutToShow );

                fExactMatchesOnlyAction = new QAction( this );
                fExactMatchesOnlyAction->setObjectName( QString::fromUtf8( "actionExactMatchesOnly" ) );
                fExactMatchesOnlyAction->setCheckable( true );
                fExactMatchesOnlyAction->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Exact Matches Only?", nullptr ) );
                connect(fExactMatchesOnlyAction, &QAction::triggered, [this]()
                    {
                        NCore::CPreferences::instance()->setExactMatchesOnly(fExactMatchesOnlyAction->isChecked());
                    }
                );

                fTreatAsTVShowByDefaultAction = new QAction( this );
                fTreatAsTVShowByDefaultAction->setObjectName( QString::fromUtf8( "actionTreatAsTVShowByDefault" ) );
                fTreatAsTVShowByDefaultAction->setCheckable( true );
                fTreatAsTVShowByDefaultAction->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Treat as TV Show by Default?", nullptr ) );
                connect(fTreatAsTVShowByDefaultAction, &QAction::triggered, [this]()
                    {
                        NCore::CPreferences::instance()->setTreatAsTVShowByDefault(fTreatAsTVShowByDefaultAction->isChecked());
                    }
                );

                fDeleteEXE = new QAction( this );
                fDeleteEXE->setObjectName( QString::fromUtf8( "actionDeleteKnownPathEXEs" ) );
                fDeleteEXE->setCheckable( true );
                fDeleteEXE->setChecked( NCore::CPreferences::instance()->deleteEXE() );
                fDeleteEXE->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Delete Executables (*.exe)?", nullptr ) );
                connect(fDeleteEXE, &QAction::triggered, [ this ]()
                {
                    NCore::CPreferences::instance()->setDeleteEXE(fDeleteEXE->isChecked() );
                }
                );

                fDeleteTXT = new QAction(this);
                fDeleteTXT->setObjectName(QString::fromUtf8("actionDeleteKnownPathTXTs"));
                fDeleteTXT->setCheckable(true);
                fDeleteTXT->setChecked(NCore::CPreferences::instance()->deleteTXT());
                fDeleteTXT->setText(QCoreApplication::translate("NMediaManager::NUi::CMainWindow", "Delete Text Files (*.txt)?", nullptr));
                connect(fDeleteTXT, &QAction::triggered, [this]()
                    {
                        NCore::CPreferences::instance()->setDeleteTXT(fDeleteTXT->isChecked());
                    }
                );

                fDeleteBAK = new QAction(this);
                fDeleteBAK->setObjectName(QString::fromUtf8("actionDeleteKnownPathBAKs"));
                fDeleteBAK->setCheckable(true);
                fDeleteBAK->setChecked(NCore::CPreferences::instance()->deleteBAK());
                fDeleteBAK->setText(QCoreApplication::translate("NMediaManager::NUi::CMainWindow", "Delete Backup Files (*.bak)?", nullptr));
                connect(fDeleteBAK, &QAction::triggered, [this]()
                    {
                        NCore::CPreferences::instance()->setDeleteBAK(fDeleteBAK->isChecked());
                    }
                );

                fDeleteNFO = new QAction(this);
                fDeleteNFO->setObjectName(QString::fromUtf8("actionDeleteKnownPathNFOs"));
                fDeleteNFO->setCheckable(true);
                fDeleteNFO->setChecked(NCore::CPreferences::instance()->deleteNFO());
                fDeleteNFO->setText(QCoreApplication::translate("NMediaManager::NUi::CMainWindow", "Delete NFO Files (*.nfo)?", nullptr));
                connect(fDeleteNFO, &QAction::triggered, [this]()
                    {
                        NCore::CPreferences::instance()->setDeleteNFO(fDeleteNFO->isChecked());
                    }
                );

                fDeleteCustom = new QAction(this);
                fDeleteCustom->setObjectName(QString::fromUtf8("actionDeleteKnownPathCustoms"));
                fDeleteCustom->setCheckable(true);
                fDeleteCustom->setChecked(NCore::CPreferences::instance()->deleteCustom());
                fDeleteCustom->setText(QCoreApplication::translate("NMediaManager::NUi::CMainWindow", "Delete Custom Known Paths?", nullptr));
                connect(fDeleteCustom, &QAction::triggered, [this]()
                    {
                        NCore::CPreferences::instance()->setDeleteCustom(fDeleteCustom->isChecked());
                    }
                );

                auto deleteKnownMenu = new QMenu(this);
                deleteKnownMenu->setObjectName("DeleteKnownMenu");
                deleteKnownMenu->setTitle("Delete Known Paths");
                deleteKnownMenu->addAction(fDeleteEXE);
                deleteKnownMenu->addAction(fDeleteTXT);
                deleteKnownMenu->addAction(fDeleteBAK);
                deleteKnownMenu->addAction(fDeleteNFO);
                deleteKnownMenu->addAction(fDeleteCustom);

                fMenu->addAction( fExactMatchesOnlyAction );
                fMenu->addAction( fTreatAsTVShowByDefaultAction );
                fMenu->addMenu(deleteKnownMenu);
                setActive( true );
            }
            return fMenu;
        }

        void CTransformPage::slotMenuAboutToShow()
        {
            fTreatAsTVShowByDefaultAction->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fExactMatchesOnlyAction->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );
            fDeleteEXE->setChecked(NCore::CPreferences::instance()->deleteEXE());
            fDeleteTXT->setChecked(NCore::CPreferences::instance()->deleteTXT());
            fDeleteBAK->setChecked(NCore::CPreferences::instance()->deleteBAK());
            fDeleteNFO->setChecked(NCore::CPreferences::instance()->deleteNFO());
            fDeleteCustom->setChecked(NCore::CPreferences::instance()->deleteCustom());
        }
    }
}


