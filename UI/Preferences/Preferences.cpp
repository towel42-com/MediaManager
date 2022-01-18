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

#include "Preferences.h"
#include "Core/Preferences.h"

#include "ui_Preferences.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "SABUtils/ButtonEnabler.h"
#include "SABUtils/UtilityModels.h"
#include "SABUtils/QtUtils.h"

namespace NMediaManager
{
    namespace NUi
    {
        CPreferences::CPreferences( QWidget * parent )
            : QDialog( parent ),
            fImpl( new Ui::CPreferences )
        {
            fImpl->setupUi( this );

            connect( fImpl->btnAddKnownString, &QToolButton::clicked, this, &CPreferences::slotAddKnownString );
            connect( fImpl->btnDelKnownString, &QToolButton::clicked, this, &CPreferences::slotDelKnownString );
            connect( fImpl->btnAddExtraString, &QToolButton::clicked, this, &CPreferences::slotAddExtraString );
            connect( fImpl->btnDelExtraString, &QToolButton::clicked, this, &CPreferences::slotDelExtraString );
            connect( fImpl->btnAddAbbreviation, &QToolButton::clicked, this, &CPreferences::slotAddAbbreviation );
            connect( fImpl->btnDelAbbreviation, &QToolButton::clicked, this, &CPreferences::slotDelAbbreviation );
            connect( fImpl->btnAddIgnorePathName, &QToolButton::clicked, this, &CPreferences::slotAddIgnorePathName );
            connect( fImpl->btnDelIgnorePathName, &QToolButton::clicked, this, &CPreferences::slotDelIgnorePathName );
            connect( fImpl->btnAddSkipPathName, &QToolButton::clicked, this, &CPreferences::slotAddSkipPathName );
            connect( fImpl->btnDelSkipPathName, &QToolButton::clicked, this, &CPreferences::slotDelSkipPathName );
            connect( fImpl->btnAddPathToDelete, &QToolButton::clicked, this, &CPreferences::slotAddPathToDelete );
            connect( fImpl->btnDelPathToDelete, &QToolButton::clicked, this, &CPreferences::slotDelPathToDelete );

            connect( fImpl->pageSelector, &QTreeWidget::currentItemChanged,   this, &CPreferences::slotPageSelectorCurrChanged );
            connect( fImpl->pageSelector, &QTreeWidget::itemActivated, this, &CPreferences::slotPageSelectorItemActived );
            connect( fImpl->pageSelector, &QTreeWidget::itemSelectionChanged, this, &CPreferences::slotPageSelectorSelectionChanged );

            connect( fImpl->btnSelectMKVMergeExe, &QToolButton::clicked, this, &CPreferences::slotSelectMKVMergeExe );
            fImpl->mkvMergeExe->setCheckExists( true );
            fImpl->mkvMergeExe->setCheckIsFile( true );
            fImpl->mkvMergeExe->setCheckIsExecutable( true );

            connect( fImpl->btnSelectMKVPropEditExe, &QToolButton::clicked, this, &CPreferences::slotSelectMKVPropEditExe );
            fImpl->mkvPropEditExe->setCheckExists( true );
            fImpl->mkvPropEditExe->setCheckIsFile( true );
            fImpl->mkvPropEditExe->setCheckIsExecutable( true );

            connect( fImpl->btnSelectFFMpegExe, &QToolButton::clicked, this, &CPreferences::slotSelectFFMpegExe );
            fImpl->ffmpegExe->setCheckExists( true );
            fImpl->ffmpegExe->setCheckIsFile( true );
            fImpl->ffmpegExe->setCheckIsExecutable( true );

            connect( fImpl->btnSelectFFProbeExe, &QToolButton::clicked, this, &CPreferences::slotSelectFFProbeExe );
            fImpl->ffprobeExe->setCheckExists( true );
            fImpl->ffprobeExe->setCheckIsFile( true );
            fImpl->ffprobeExe->setCheckIsExecutable( true );

            connect( fImpl->ffmpegExe, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CPreferences::slotFFToolChanged );
            connect( fImpl->ffprobeExe, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CPreferences::slotFFToolChanged );
            connect( fImpl->mkvMergeExe, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CPreferences::slotMKVNixToolChanged );
            connect( fImpl->mkvPropEditExe, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CPreferences::slotMKVNixToolChanged );

            fKnownStringModel = new QStringListModel( this );
            fImpl->knownStrings->setModel( fKnownStringModel );

            fExtraStringModel = new QStringListModel( this );
            fImpl->knownExtraStrings->setModel( fExtraStringModel );

            fAbbreviationsModel = new NSABUtils::CKeyValuePairModel( this );
            fImpl->knownAbbreviations->setModel( fAbbreviationsModel );

            fSkipPathNamesModel = new QStringListModel( this );
            fImpl->pathNamesToSkip->setModel( fSkipPathNamesModel );

            fIgnorePathNamesModel = new QStringListModel( this );
            fImpl->pathNamesToIgnore->setModel( fIgnorePathNamesModel );

            fPathsToDeleteModel = new QStringListModel( this );
            fImpl->pathsToDelete->setModel( fPathsToDeleteModel );

            new NSABUtils::CButtonEnabler( fImpl->knownStrings, fImpl->btnDelKnownString );
            new NSABUtils::CButtonEnabler( fImpl->knownExtraStrings, fImpl->btnDelExtraString );
            new NSABUtils::CButtonEnabler( fImpl->knownAbbreviations, fImpl->btnDelAbbreviation );
            new NSABUtils::CButtonEnabler( fImpl->pathNamesToSkip, fImpl->btnDelSkipPathName );
            new NSABUtils::CButtonEnabler( fImpl->pathNamesToIgnore, fImpl->btnDelIgnorePathName );
            new NSABUtils::CButtonEnabler( fImpl->pathsToDelete, fImpl->btnDelPathToDelete );

            loadSettings();

            fftoolToolChanged( fImpl->ffmpegExe );
            fftoolToolChanged( fImpl->ffprobeExe );
            mkvnixToolChanged( fImpl->mkvMergeExe );
            mkvnixToolChanged( fImpl->mkvPropEditExe );

            setupPageSelector();

            fImpl->pageSelector->expandAll();

            QSettings settings;
            auto lastPrefPageKey = settings.value( "LastPrefPage", "Extensions" ).toString();
            auto pos = fItemMap.find( lastPrefPageKey );
            if ( pos != fItemMap.end() )
                fImpl->pageSelector->setCurrentItem( (*pos).second );

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

        void CPreferences::addString( const QString & title, const QString & label, QStringListModel * model, QListView * listView, bool splitWords )
        {
            auto text = QInputDialog::getText( this, title, label ).trimmed();
            if ( text.isEmpty() )
                return;

            QStringList words;
            if ( splitWords )
                words = text.split( QRegularExpression( "\\s" ), Qt::SkipEmptyParts );
            else
                words.push_back( text );

            auto strings = model->stringList();

            for ( auto && ii : words )
            {
                ii = ii.trimmed();
                strings.removeAll( ii );
            }

            strings << words;
            model->setStringList( strings );
            listView->scrollTo( model->index( strings.count() - 1, 0 ) );
        }

        void CPreferences::delString( QStringListModel * listModel, QListView * listView )
        {
            auto model = listView->selectionModel();
            if ( !model )
                return;

            auto selected = model->selectedRows();
            if ( selected.isEmpty() )
                return;

            auto strings = listModel->stringList();
            for ( auto && ii : selected )
            {
                auto text = ii.data().toString();
                strings.removeAll( text );
            }

            listModel->setStringList( strings );
            listView->scrollTo( listModel->index( selected.front().row(), 0 ) );
        }

        void CPreferences::slotAddKnownString()
        {
            addString( tr( "Add Known String" ), tr( "String:" ), fKnownStringModel, fImpl->knownStrings, true );
        }

        void CPreferences::slotDelKnownString()
        {
            delString( fKnownStringModel, fImpl->knownStrings );
        }

        void CPreferences::slotAddExtraString()
        {
            addString( tr( "Add Known String For Extended Information" ), tr( "String:" ), fExtraStringModel, fImpl->knownExtraStrings, true );
        }

        void CPreferences::slotDelExtraString()
        {
            delString( fExtraStringModel, fImpl->knownExtraStrings );
        }

        void CPreferences::slotAddAbbreviation()
        {
            auto text = QInputDialog::getText( this, tr( "Add Abbreviation in the form Abbreviation=FullText" ), tr( "String:" ) );
            if ( text.isEmpty() )
                return;
            text = text.trimmed();

            fAbbreviationsModel->addRow( text );
            fImpl->knownAbbreviations->scrollTo( fAbbreviationsModel->index( fAbbreviationsModel->rowCount() - 1 , 0 ) );
        }

        void CPreferences::slotDelAbbreviation()
        {
            auto model = fImpl->knownAbbreviations->selectionModel();
            if ( !model )
                return;
            auto selected = model->selectedRows();
            if ( selected.isEmpty() )
                return;
            fAbbreviationsModel->removeRow( selected.front().row() );
            fImpl->knownAbbreviations->scrollTo( fKnownStringModel->index( selected.front().row(), 0 ) );
        }

        void CPreferences::slotAddSkipPathName()
        {
            addString( tr( "Add Path Name to Skip" ), tr( "Path Name:" ), fSkipPathNamesModel, fImpl->pathNamesToSkip, false );
        }

        void CPreferences::slotDelSkipPathName()
        {
            delString( fSkipPathNamesModel, fImpl->pathNamesToSkip );
        }

        void CPreferences::slotAddIgnorePathName()
        {
            addString( tr( "Add Path Name to Ignore" ), tr( "Path Name:" ), fIgnorePathNamesModel, fImpl->pathNamesToIgnore, false );
        }

        void CPreferences::slotDelIgnorePathName()
        {
            delString( fIgnorePathNamesModel, fImpl->pathNamesToIgnore );
        }

        void CPreferences::slotAddPathToDelete()
        {
            addString( tr( "Add Path (Regular Expressions Allowed) Name to Delete" ), tr( "Name:" ), fPathsToDeleteModel, fImpl->pathsToDelete, false );
        }

        void CPreferences::slotDelPathToDelete()
        {
            delString( fPathsToDeleteModel, fImpl->pathsToDelete );
        }

        void CPreferences::slotSelectMKVMergeExe()
        {
            auto exe = QFileDialog::getOpenFileName( this, tr( "Select MKVMerge Executable:" ), fImpl->mkvMergeExe->text(), "mkvmerge Executable (mkvmerge.exe);;All Executables (*.exe);;All Files (*.*)" );
            if ( !exe.isEmpty() && !QFileInfo( exe ).isExecutable() )
            {
                QMessageBox::critical( this, "Not an Executable", tr( "The file '%1' is not an executable" ).arg( exe ) );
                return;
            }

            if ( !exe.isEmpty() )
                fImpl->mkvMergeExe->setText( exe );
        }

        void CPreferences::slotSelectMKVPropEditExe()
        {
            auto exe = QFileDialog::getOpenFileName( this, tr( "Select MKVPropEdit Executable:" ), fImpl->mkvPropEditExe->text(), "mkvpropedit Executable (mkvpropedit.exe);;All Executables (*.exe);;All Files (*.*)" );
            if ( !exe.isEmpty() && !QFileInfo( exe ).isExecutable() )
            {
                QMessageBox::critical( this, "Not an Executable", tr( "The file '%1' is not an executable" ).arg( exe ) );
                return;
            }

            if ( !exe.isEmpty() )
                fImpl->mkvPropEditExe->setText( exe );
        }

        void CPreferences::slotSelectFFMpegExe()
        {
            auto exe = QFileDialog::getOpenFileName( this, tr( "Select ffmpeg Executable:" ), fImpl->ffmpegExe->text(), "ffmpeg Executable (ffmpeg.exe);;All Executables (*.exe);;All Files (*.*)" );
            if ( !exe.isEmpty() && !QFileInfo( exe ).isExecutable() )
            {
                QMessageBox::critical( this, "Not an Executable", tr( "The file '%1' is not an executable" ).arg( exe ) );
                return;
            }

            if ( !exe.isEmpty() )
                fImpl->ffmpegExe->setText( exe );
        }

        void CPreferences::slotSelectFFProbeExe()
        {
            auto exe = QFileDialog::getOpenFileName( this, tr( "Select ffprobe Executable:" ), fImpl->ffprobeExe->text(), "ffprobe Executable (ffprobe.exe);;All Executables (*.exe);;All Files (*.*)" );
            if ( !exe.isEmpty() && !QFileInfo( exe ).isExecutable() )
            {
                QMessageBox::critical( this, "Not an Executable", tr( "The file '%1' is not an executable" ).arg( exe ) );
                return;
            }

            if ( !exe.isEmpty() )
                fImpl->ffprobeExe->setText( exe );
        }

        void CPreferences::updateOtherTool( QObject * sender, const std::pair< QLineEdit *, QString > & lhs, const std::pair< QLineEdit *, QString > & rhs )
        {
            auto le = dynamic_cast<QLineEdit *>(sender);
            if ( !le )
                return;

            if ( le->text().isEmpty() || !QFileInfo::exists( le->text() ) )
                return;

            QString otherExe;
            QLineEdit * otherLE = nullptr;
            if ( le == lhs.first )
            {
                otherLE = rhs.first;
                otherExe = rhs.second;
            }
            else if ( le == rhs.first )
            {
                otherLE = lhs.first;
                otherExe = lhs.second;
            }
            else
                return;

            if ( !otherLE->text().isEmpty() && QFileInfo( otherLE->text() ).exists() && QFileInfo( otherLE->text() ).isExecutable() )
                return;

            auto dir = QFileInfo( le->text() ).absoluteDir();
            auto otherEXE = dir.absoluteFilePath( otherExe );
            if ( QFileInfo::exists( otherEXE ) && QFileInfo( otherEXE ).isExecutable() )
                otherLE->setText( otherEXE );
            otherEXE += ".exe";
            if ( QFileInfo::exists( otherEXE ) && QFileInfo( otherEXE ).isExecutable() )
                otherLE->setText( otherEXE );
        }

        void CPreferences::slotMKVNixToolChanged()
        {
            mkvnixToolChanged( dynamic_cast< QLineEdit * >( sender() ) );
        }

        void CPreferences::mkvnixToolChanged( QLineEdit * le )
        {
            updateOtherTool( le, { fImpl->mkvPropEditExe, "mkvpropedit" }, { fImpl->mkvMergeExe, "mkvmerge" } );
        }

        void CPreferences::slotFFToolChanged()
        {
            fftoolToolChanged( dynamic_cast<QLineEdit *>(sender()) );
        }

        void CPreferences::fftoolToolChanged( QLineEdit * le )
        {
            updateOtherTool( le, { fImpl->ffprobeExe, "ffprobe" }, { fImpl->ffmpegExe, "ffmpeg" } );
        }

        void CPreferences::loadSettings()
        {
            QSettings settings;

            fKnownStringModel->setStringList( NCore::CPreferences::instance()->getKnownStrings() );
            fExtraStringModel->setStringList( NCore::CPreferences::instance()->getKnownExtendedStrings() );
            fAbbreviationsModel->setValues( NCore::CPreferences::instance()->getKnownAbbreviations() );
            fIgnorePathNamesModel->setStringList( NCore::CPreferences::instance()->getIgnoredPaths() );
            fSkipPathNamesModel->setStringList( NCore::CPreferences::instance()->getSkippedPaths() );
            fPathsToDeleteModel->setStringList( NCore::CPreferences::instance()->getPathsToDelete() );

            fImpl->mediaExtensions->setText( NCore::CPreferences::instance()->getMediaExtensions().join( ";" ) );
            fImpl->subtitleExtensions->setText( NCore::CPreferences::instance()->getSubtitleExtensions().join( ";" ) );
            fImpl->treatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->exactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );

            fImpl->tvOutFilePattern->setText( NCore::CPreferences::instance()->getTVOutFilePattern() );
            fImpl->tvOutDirPattern->setText( NCore::CPreferences::instance()->getTVOutDirPattern() );

            fImpl->movieOutFilePattern->setText( NCore::CPreferences::instance()->getMovieOutFilePattern() );
            fImpl->movieOutDirPattern->setText( NCore::CPreferences::instance()->getMovieOutDirPattern() );

            fImpl->mkvMergeExe->setText( NCore::CPreferences::instance()->getMKVMergeEXE() );
            fImpl->mkvPropEditExe->setText( NCore::CPreferences::instance()->getMKVPropEditEXE() );
            fImpl->ffmpegExe->setText( NCore::CPreferences::instance()->getFFMpegEXE() );
            fImpl->ffprobeExe->setText( NCore::CPreferences::instance()->getFFProbeEXE() );
        }

        void CPreferences::saveSettings()
        {
            NCore::CPreferences::instance()->setKnownStrings( fKnownStringModel->stringList() );
            NCore::CPreferences::instance()->setKnownExtendedStrings( fExtraStringModel->stringList() );
            NCore::CPreferences::instance()->setKnownAbbreviations( fAbbreviationsModel->data() );
            NCore::CPreferences::instance()->setSkippedPaths( fSkipPathNamesModel->stringList() );
            NCore::CPreferences::instance()->setIgnoredPaths( fIgnorePathNamesModel->stringList() );
            NCore::CPreferences::instance()->setPathsToDelete( fPathsToDeleteModel->stringList() );


            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->treatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->exactMatchesOnly->isChecked() );
            NCore::CPreferences::instance()->setMediaExtensions( fImpl->mediaExtensions->text() );
            NCore::CPreferences::instance()->setSubtitleExtensions( fImpl->subtitleExtensions->text() );

            NCore::CPreferences::instance()->setTVOutFilePattern( fImpl->tvOutFilePattern->text() );
            NCore::CPreferences::instance()->setTVOutDirPattern( fImpl->tvOutDirPattern->text() );

            NCore::CPreferences::instance()->setMovieOutFilePattern( fImpl->movieOutFilePattern->text() );
            NCore::CPreferences::instance()->setMovieOutDirPattern( fImpl->movieOutDirPattern->text() );
            NCore::CPreferences::instance()->setMKVMergeEXE( fImpl->mkvMergeExe->text() );
            NCore::CPreferences::instance()->setMKVPropEditEXE( fImpl->mkvPropEditExe->text() );
            NCore::CPreferences::instance()->setFFProbeEXE( fImpl->ffprobeExe->text() );
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
            fImpl->stackedWidget->setCurrentWidget( (*ii).second );
        }

        QStringList CPreferences::getPageItemNames( const QString & name )
        {
            auto retVal = name.split( "____", Qt::SkipEmptyParts );
            for ( auto && ii : retVal )
            {
                ii.replace( "__", "/" );
                ii.replace( "_", " " );
            }

            return retVal;
        }

        QString CPreferences::keyForItem( QTreeWidgetItem * item )
        {
            if ( !item )
                return QString();

            QString retVal;
            if ( item->parent() )
                retVal = keyForItem( item->parent() );
            retVal += "__" + item->text( 0 );
            return retVal;
        }

        void CPreferences::setupPageSelector()
        {
            int pageCount = fImpl->stackedWidget->count();
            for ( int ii = 0; ii < pageCount; ++ii )
            {
                auto widget = fImpl->stackedWidget->widget( ii );
                auto itemNames = getPageItemNames( widget->objectName() );
                Q_ASSERT( !itemNames.isEmpty() );
                if ( itemNames.isEmpty() )
                    continue;

                QString key;
                QTreeWidgetItem * parentItem = nullptr;
                for ( int ii = 0; ii < itemNames.count(); ++ii )
                {
                    key += "__" + itemNames[ii];
                    auto pos = fItemMap.find( key );
                    QTreeWidgetItem * item = nullptr;
                    if ( pos == fItemMap.end() )
                    {
                        if ( parentItem )
                            item = new QTreeWidgetItem( parentItem );
                        else
                            item = new QTreeWidgetItem( fImpl->pageSelector );
                        item->setText( 0, itemNames[ii] );

                        fPageMap[item] = widget;
                        fItemMap[key] = item;
                        parentItem = item;
                    }
                    else
                    {
                        item = parentItem = (*pos).second;
                    }
                }
            }
        }

    }
}
