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

#include "TagsPage.h"
#include "SelectTMDB.h"
#include "Preferences/Core/Preferences.h"
#include "Models/TagsModel.h"

#include "SABUtils/QtUtils.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QTimer>
#include <QDir>
#include <QTreeView>
#include <QCoreApplication>
#include <QMenu>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSortFilterProxyModel>
#include <optional>

namespace NMediaManager
{
    namespace NUi
    {
        CTagsPage::CTagsPage( QWidget * parent )
            : CBasePage( "Tags", parent )
        {
            fFilter = new QLineEdit;
            fFilter->setPlaceholderText( tr( "Filter" ) );
            auto mainLayout = this->mainLayout();
            mainLayout->insertWidget( 0, fFilter );
        }

        CTagsPage::~CTagsPage()
        {
            saveSettings();
        }

        void CTagsPage::loadSettings()
        {
            CBasePage::loadSettings();
        }

        NModels::CTagsModel * CTagsPage::tagsModel()
        {
            if ( !fModel )
                return nullptr;

            return dynamic_cast<NModels::CTagsModel *>( fModel.get() );
        }

        void CTagsPage::postNonQueuedRun( bool finalStep, bool canceled )
        {
            emit sigStopStayAwake();
            if ( finalStep && !canceled )
                load( true );
        }

        bool CTagsPage::extendContextMenu( QMenu * /*menu*/, const QModelIndex & idx )
        {
            if ( !idx.isValid() )
                return false;

            return true;
        }
        void CTagsPage::postLoadFinished( bool /*canceled*/ )
        {
        }

        NModels::CDirModel * CTagsPage::createDirModel()
        {
            auto retVal = new NModels::CTagsModel( this );

            fFilterModel = new NModels::CTagsFilterModel( this );
            fFilterModel->setSourceModel( retVal );
            connect( fFilter, &QLineEdit::textChanged, fFilterModel, &NModels::CTagsFilterModel::slotSetFilter );

            return retVal;
        }

        QAbstractItemModel * CTagsPage::getDirModel() const
        {
            return fFilterModel;
        }

        QString CTagsPage::loadTitleName() const
        {
            return tr( "Finding Files" );
        }

        QString CTagsPage::loadCancelName() const
        {
            return tr( "Cancel" );
        }

        QString CTagsPage::actionTitleName() const
        {
            return tr( "Setting Tags..." );
        }

        QString CTagsPage::actionCancelName() const
        {
            return tr( "Abort Tag Setting" );
        }

        QString CTagsPage::actionErrorName() const
        {
            return tr( "Error while Setting Tags:" );
        }


        QStringList CTagsPage::dirModelFilter() const
        {
            auto retVal = NPreferences::NCore::CPreferences::instance()->getMediaExtensions();
            return retVal;
        }

        void CTagsPage::setupModel()
        {
            CBasePage::setupModel();
        }

        QMenu * CTagsPage::menu()
        {
            if ( !fMenu )
            {
                fMenu = new QMenu( this );
                fMenu->setObjectName( "Tags Menu " );
                fMenu->setTitle( tr( "Tags" ) );
                connect( fMenu, &QMenu::aboutToShow, this, &CTagsPage::slotMenuAboutToShow );

                fIgnoreSkippedPathSettings = new QAction( this );
                fIgnoreSkippedPathSettings->setObjectName( QString::fromUtf8( "actionIgnoreSkippedPathSettings" ) );
                fIgnoreSkippedPathSettings->setCheckable( true );
                fIgnoreSkippedPathSettings->setChecked( NPreferences::NCore::CPreferences::instance()->getIgnorePathNamesToSkip( false ) );
                fIgnoreSkippedPathSettings->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Ignore Skipped Path Settings?", nullptr ) );
                connect( fIgnoreSkippedPathSettings, &QAction::triggered, [this]()
                         {
                             NPreferences::NCore::CPreferences::instance()->setIgnorePathNamesToSkip( false, fIgnoreSkippedPathSettings->isChecked() );
                         }
                );

                fVerifyMediaTitle = new QAction( this );
                fVerifyMediaTitle->setObjectName( QString::fromUtf8( "actionVerifyMediaTitle" ) );
                fVerifyMediaTitle->setCheckable( true );
                fVerifyMediaTitle->setChecked( NPreferences::NCore::CPreferences::instance()->deleteNFO() );
                fVerifyMediaTitle->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Verify Media Title?", nullptr ) );
                connect( fVerifyMediaTitle, &QAction::triggered, [this]()
                         {
                             NPreferences::NCore::CPreferences::instance()->setVerifyMediaTitle( fVerifyMediaTitle->isChecked() );
                         }
                );

                fVerifyMediaDate = new QAction( this );
                fVerifyMediaDate->setObjectName( QString::fromUtf8( "actionVerifyMediaDate" ) );
                fVerifyMediaDate->setCheckable( true );
                fVerifyMediaDate->setChecked( NPreferences::NCore::CPreferences::instance()->deleteNFO() );
                fVerifyMediaDate->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Verify Media Date?", nullptr ) );
                connect( fVerifyMediaDate, &QAction::triggered, [this]()
                         {
                             NPreferences::NCore::CPreferences::instance()->setVerifyMediaDate( fVerifyMediaDate->isChecked() );
                         }
                );

                fVerifyMediaComment = new QAction( this );
                fVerifyMediaComment->setObjectName( QString::fromUtf8( "actionVerifyMediaComment" ) );
                fVerifyMediaComment->setCheckable( true );
                fVerifyMediaComment->setChecked( NPreferences::NCore::CPreferences::instance()->deleteNFO() );
                fVerifyMediaComment->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Verify Media Comment?", nullptr ) );
                connect( fVerifyMediaComment, &QAction::triggered, [this]()
                         {
                             NPreferences::NCore::CPreferences::instance()->setVerifyMediaComment( fVerifyMediaComment->isChecked() );
                         }
                );

                fVerifyMediaTags = new QAction( this );
                fVerifyMediaTags->setObjectName( QString::fromUtf8( "actionVerifyMediaTags" ) );
                fVerifyMediaTags->setCheckable( true );
                fVerifyMediaTags->setChecked( NPreferences::NCore::CPreferences::instance()->deleteNFO() );
                fVerifyMediaTags->setText( QCoreApplication::translate( "NMediaManager::NUi::CMainWindow", "Verify Media Tags?", nullptr ) );
                connect( fVerifyMediaTags, &QAction::triggered, [this]()
                         {
                             fVerifyMediaDate->setEnabled( fVerifyMediaTags->isChecked() );
                             fVerifyMediaTitle->setEnabled( fVerifyMediaTags->isChecked() );
                             fVerifyMediaComment->setEnabled( fVerifyMediaTags->isChecked() );
                             NPreferences::NCore::CPreferences::instance()->setVerifyMediaTags( fVerifyMediaTags->isChecked() );
                         }
                );

                auto verifyMediaMenu = new QMenu( this );
                verifyMediaMenu->setObjectName( "VerifyMediaMenu" );
                verifyMediaMenu->setTitle( "Verify Media Tags" );

                verifyMediaMenu->addAction( fVerifyMediaTags );
                verifyMediaMenu->addSeparator();
                verifyMediaMenu->addAction( fVerifyMediaTitle );
                verifyMediaMenu->addAction( fVerifyMediaDate );
                verifyMediaMenu->addAction( fVerifyMediaComment );

                fMenu->addAction( fIgnoreSkippedPathSettings );
                fMenu->addSeparator();
                fMenu->addMenu( verifyMediaMenu );
                setActive( true );
            }
            return fMenu;
        }

        void CTagsPage::slotMenuAboutToShow()
        {
            fIgnoreSkippedPathSettings->setChecked( NPreferences::NCore::CPreferences::instance()->getIgnorePathNamesToSkip( false ) );

            fVerifyMediaDate->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaDate() );
            fVerifyMediaTitle->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitle() );
            fVerifyMediaComment->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaComment() );
            fVerifyMediaTags->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaTags() );
            fVerifyMediaDate->setEnabled( fVerifyMediaTags->isChecked() );
            fVerifyMediaTitle->setEnabled( fVerifyMediaTags->isChecked() );
            fVerifyMediaComment->setEnabled( fVerifyMediaTags->isChecked() );
        }

        void CTagsPage::slotPreferencesChanged( NPreferences::EPreferenceTypes prefTypes )
        {
            if ( prefTypes & NPreferences::EPreferenceType::eTagPrefs )
            {
                if ( tagsModel() )
                    tagsModel()->resetStatusCaches();
            }
            CBasePage::slotPreferencesChanged( prefTypes );
        }
    }
}


