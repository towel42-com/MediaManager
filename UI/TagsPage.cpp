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
#include <optional>

namespace NMediaManager
{
    namespace NUi
    {
        CTagsPage::CTagsPage( QWidget *parent )
            : CBasePage( "Tags", parent )
        {
        }

        CTagsPage::~CTagsPage()
        {
            saveSettings();
        }

        void CTagsPage::loadSettings()
        {
            CBasePage::loadSettings();
        }

        NModels::CTagsModel * CTagsPage::model()
        {
            if ( !fModel )
                return nullptr;

            return dynamic_cast<NModels::CTagsModel *>(fModel.get());
        }

        void CTagsPage::postNonQueuedRun( bool finalStep, bool canceled )
        {
            emit sigStopStayAwake();
            if ( finalStep && !canceled )
                load();
        }

        bool CTagsPage::extendContextMenu( QMenu * menu, const QModelIndex & idx )
        {
            if ( !idx.isValid() )
                return false;

            if ( fModel->isMediaFile(idx) )
            {
                menu->addSeparator();
                menu->addAction(tr("Set Tags..."),
                    [idx, this]()
                    {
                        editMediaTags(idx);
                    });
                menu->addAction(tr("Auto Set Tags from File Name..."),
                    [idx, this]()
                    {
                        fModel->autoSetMediaTags(idx);
                    });
            }

            return true;
        }
        void CTagsPage::postLoadFinished( bool /*canceled*/ )
        {
        }

        NModels::CDirModel * CTagsPage::createDirModel()
        {
            return new NModels::CTagsModel( this );
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
                connect(fMenu, &QMenu::aboutToShow, this, &CTagsPage::slotMenuAboutToShow);

                fVerifyMediaTitle = new QAction(this);
                fVerifyMediaTitle->setObjectName(QString::fromUtf8("actionVerifyMediaTitle"));
                fVerifyMediaTitle->setCheckable(true);
                fVerifyMediaTitle->setChecked(NPreferences::NCore::CPreferences::instance()->deleteNFO());
                fVerifyMediaTitle->setText(QCoreApplication::translate("NMediaManager::NUi::CMainWindow", "Verify Media Title?", nullptr));
                connect(fVerifyMediaTitle, &QAction::triggered, [this]()
                    {
                        //model()->clearStatusResults();
                        NPreferences::NCore::CPreferences::instance()->setVerifyMediaTitle(fVerifyMediaTitle->isChecked());
                    }
                );

                fVerifyMediaDate = new QAction(this);
                fVerifyMediaDate->setObjectName(QString::fromUtf8("actionVerifyMediaDate"));
                fVerifyMediaDate->setCheckable(true);
                fVerifyMediaDate->setChecked(NPreferences::NCore::CPreferences::instance()->deleteNFO());
                fVerifyMediaDate->setText(QCoreApplication::translate("NMediaManager::NUi::CMainWindow", "Verify Media Date?", nullptr));
                connect(fVerifyMediaDate, &QAction::triggered, [this]()
                    {
                        //model()->clearStatusResults();
                        NPreferences::NCore::CPreferences::instance()->setVerifyMediaDate(fVerifyMediaDate->isChecked());
                    }
                );

                fVerifyMediaTags = new QAction(this);
                fVerifyMediaTags->setObjectName(QString::fromUtf8("actionVerifyMediaTags"));
                fVerifyMediaTags->setCheckable(true);
                fVerifyMediaTags->setChecked(NPreferences::NCore::CPreferences::instance()->deleteNFO());
                fVerifyMediaTags->setText(QCoreApplication::translate("NMediaManager::NUi::CMainWindow", "Verify Media Tags?", nullptr));
                connect(fVerifyMediaTags, &QAction::triggered, [this]()
                    {
                        fVerifyMediaDate->setEnabled(fVerifyMediaTags->isChecked());
                        fVerifyMediaTitle->setEnabled(fVerifyMediaTags->isChecked());
                        //model()->clearStatusResults();
                        NPreferences::NCore::CPreferences::instance()->setVerifyMediaTags(fVerifyMediaTags->isChecked());
                    }
                );

                auto verifyMediaMenu = new QMenu(this);
                verifyMediaMenu->setObjectName("VerifyMediaMenu");
                verifyMediaMenu->setTitle("Verify Media Tags");

                verifyMediaMenu->addAction(fVerifyMediaTags);
                verifyMediaMenu->addAction(fVerifyMediaTitle);
                verifyMediaMenu->addAction(fVerifyMediaDate);

                fMenu->addMenu(verifyMediaMenu);
                setActive( true );
            }
            return fMenu;
        }

        void CTagsPage::slotMenuAboutToShow()
        {
            fVerifyMediaDate->setChecked(NPreferences::NCore::CPreferences::instance()->getVerifyMediaDate());
            fVerifyMediaTitle->setChecked(NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitle());
            fVerifyMediaTags->setChecked(NPreferences::NCore::CPreferences::instance()->getVerifyMediaTags());
            fVerifyMediaDate->setEnabled(fVerifyMediaTags->isChecked());
            fVerifyMediaTitle->setEnabled(fVerifyMediaTags->isChecked());
        }
    }
}


