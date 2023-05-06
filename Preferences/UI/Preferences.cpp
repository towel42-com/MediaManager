// The MIT License( MIT )
//
// Copyright( c ) 2020-2023 Scott Aron Bloom
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

#include "Preferences.h"
#include "Preferences/Core/Preferences.h"

#include "ui_Preferences.h"

#include "ExtendedInfo.h"
#include "GeneralSettings.h"
#include "FFMpegInfo.h"
#include "ExternalTools.h"
#include "IgnoredPaths.h"
#include "KnownAbbreviations.h"
#include "MovieSettings.h"
#include "PathsToDelete.h"
#include "RemoveFromPaths.h"
#include "KnownHyphenated.h"
#include "SkippedPaths.h"
#include "SearchSettings.h"
#include "MediaNamingSettings.h"
#include "TagAnalysisSettings.h"
#include "TVShowSettings.h"
#include "BIFGeneration.h"
#include "TranscodeGeneralSettings.h"
#include "TranscodeAudioSettings.h"
#include "TranscodeVideoSettings.h"
#include "TranscodeVideoCodec.h"
#include "TranscodeBitrate.h"

#include <QSettings>
#include <QPushButton>
#include "SABUtils/QtUtils.h"

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CPreferences::CPreferences( QWidget *parent ) :
                QDialog( parent ),
                fImpl( new Ui::CPreferences )
            {
                fImpl->setupUi( this );

                auto validateBtn = fImpl->buttonBox->addButton( "&Validate", QDialogButtonBox::ActionRole );
                connect( validateBtn, &QPushButton::clicked, this, &CPreferences::slotValidatePreferences );
                connect( fImpl->buttonBox->button( QDialogButtonBox::Apply ), &QPushButton::clicked, this, &CPreferences::slotApply );

                connect( fImpl->pageSelector, &QTreeWidget::currentItemChanged, this, &CPreferences::slotPageSelectorCurrChanged );
                connect( fImpl->pageSelector, &QTreeWidget::itemActivated, this, &CPreferences::slotPageSelectorItemActived );
                connect( fImpl->pageSelector, &QTreeWidget::itemSelectionChanged, this, &CPreferences::slotPageSelectorSelectionChanged );

                loadPages();
                loadSettings();
                fImpl->pageSelector->expandAll();

                QSettings settings;
                if ( settings.contains( "LastPrefPage" ) )
                {
                    auto lastPrefPageKey = settings.value( "LastPrefPage", "" ).toString();
                    auto pos = fItemMap.find( lastPrefPageKey );
                    if ( pos != fItemMap.end() )
                        fImpl->pageSelector->setCurrentItem( ( *pos ).second );
                }
                fImpl->pageSelector->header()->setSectionResizeMode( QHeaderView::ResizeToContents );
                fImpl->pageSelector->header()->setStretchLastSection( false );
                fImpl->pageSelector->header()->setSectionResizeMode( QHeaderView::Stretch );
                fImpl->pageSelector->setMinimumWidth( NSABUtils::autoSize( fImpl->pageSelector ) + 4 );
                fImpl->splitter->setChildrenCollapsible( false );
            }

            CPreferences::~CPreferences()
            {
                QSettings settings;
                auto currItem = keyForItem( fImpl->pageSelector->currentItem() );
                settings.setValue( "LastPrefPage", currItem );
            }

            void CPreferences::accept()
            {
                saveSettings();
                QDialog::accept();
            }

            void CPreferences::loadSettings()
            {
                for ( auto &&ii : fPageMap )
                {
                    ii.second->load();
                }
            }

            void CPreferences::saveSettings()
            {
                for ( auto &&ii : fPageMap )
                {
                    ii.second->save();
                }
            }

            void CPreferences::loadPages()
            {
                addPage( new CGeneralSettings );
                addPage( new CExternalTools );
                addPage( new CFFMpegInfo );

                addPage( new CTranscodeGeneralSettings );
                addPage( new CTranscodeVideoSettings );
                addPage( new CTranscodeVideoCodec );
                addPage( new CTranscodeAudioSettings );
                addPage( new CTranscodeBitrate );

                addPage( new CExtendedInfo );
                addPage( new CKnownAbbreviations );

                addPage( new CMediaNamingSettings );
                addPage( new CMovieSettings );
                addPage( new CTVShowSettings );

                addPage( new CSearchSettings );

                addPage( new CPathsToDelete );
                addPage( new CRemoveFromPaths );
                addPage( new CKnownHyphenated );
                addPage( new CSkippedPaths( true ) );
                addPage( new CSkippedPaths( false ) );
                addPage( new CIgnoredPaths );

                addPage( new CTagAnalysisSettings );
                addPage( new CBIFGeneration );
            }

            void CPreferences::addPage( CBasePrefPage *page )
            {
                fImpl->stackedWidget->addWidget( page );
                auto name = page->pageName();
                //qDebug() << name;
                Q_ASSERT( !name.isEmpty() );
                if ( name.isEmpty() )
                    return;
                QString key;
                QTreeWidgetItem *parentItem = nullptr;
                for ( int ii = 0; ii < name.count(); ++ii )
                {
                    key += "__" + name[ ii ];

                    auto pos = fItemMap.find( key );
                    if ( pos == fItemMap.end() )
                    {
                        QTreeWidgetItem *item = nullptr;
                        if ( parentItem )
                            item = new QTreeWidgetItem( parentItem );
                        else
                            item = new QTreeWidgetItem( fImpl->pageSelector );
                        item->setText( 0, name[ ii ] );

                        fPageMap[ item ] = page;
                        fItemMap[ key ] = item;
                        parentItem = item;
                    }
                    else
                    {
                        auto item = parentItem = ( *pos ).second;
                        if ( ( ii + 1 ) == name.count() )
                        {
                            Q_ASSERT( fPageMap.find( item ) == fPageMap.end() );
                            fPageMap[ item ] = page;
                        }
                    }
                }
            }

            void CPreferences::slotPageSelectorCurrChanged( QTreeWidgetItem * /*current*/, QTreeWidgetItem * /*previous*/ )
            {
                slotPageSelectorSelectionChanged();
            }

            void CPreferences::slotPageSelectorItemActived( QTreeWidgetItem * /*item*/ )
            {
                slotPageSelectorSelectionChanged();
            }

            void CPreferences::slotPageSelectorSelectionChanged()
            {
                auto curr = fImpl->pageSelector->currentItem();
                if ( !curr )
                    return;
                auto ii = fPageMap.find( curr );
                if ( ii == fPageMap.end() )
                    return;
                fImpl->stackedWidget->setCurrentWidget( ( *ii ).second );
            }

            void CPreferences::slotApply()
            {
                saveSettings();
            }

            void CPreferences::slotValidatePreferences()
            {
                saveSettings();
                NPreferences::NCore::CPreferences::instance()->showValidateDefaults( this, true );
            }

            QString CPreferences::keyForItem( QTreeWidgetItem *item )
            {
                if ( !item )
                    return {};

                QString retVal;
                if ( item->parent() )
                    retVal = keyForItem( item->parent() );
                retVal += "__" + item->text( 0 );
                return retVal;
            }
        }
    }
}
