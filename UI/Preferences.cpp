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
            connect( fImpl->btnAddIgnoreFileName, &QToolButton::clicked, this, &CPreferences::slotAddIgnoreFileName );
            connect( fImpl->btnDelIgnoreFileName, &QToolButton::clicked, this, &CPreferences::slotDelIgnoreFileName );
            connect( fImpl->btnAddIgnoreDir, &QToolButton::clicked, this, &CPreferences::slotAddIgnoreDir );
            connect( fImpl->btnDelIgnoreDir, &QToolButton::clicked, this, &CPreferences::slotDelIgnoreDir );

            connect( fImpl->pageSelector, &QTreeWidget::currentItemChanged,   this, &CPreferences::slotPageSelectorCurrChanged );
            connect( fImpl->pageSelector, &QTreeWidget::itemActivated, this, &CPreferences::slotPageSelectorItemActived );
            connect( fImpl->pageSelector, &QTreeWidget::itemSelectionChanged, this, &CPreferences::slotPageSelectorSelectionChanged );

            connect( fImpl->btnSelectMKVMergeExe, &QToolButton::clicked, this, &CPreferences::slotSelectMKVMergeExe );
            fImpl->mkvMergeExe->setIsOKFunction( [ ]( const QString & fileName )
            {
                auto fi = QFileInfo( fileName );
                return fileName.isEmpty() || (fi.exists() && fi.isFile() && fi.isExecutable());
            }, tr( "File '%1' does not Exist or is not an Executable" ) );

            connect( fImpl->btnSelectMKVPropEditExe, &QToolButton::clicked, this, &CPreferences::slotSelectMKVPropEditExe );
            fImpl->mkvPropEditExe->setIsOKFunction( [ ]( const QString & fileName )
            {
                auto fi = QFileInfo( fileName );
                return fileName.isEmpty() || (fi.exists() && fi.isFile() && fi.isExecutable());
            }, tr( "File '%1' does not Exist or is not an Executable" ) );

            connect( fImpl->btnSelectFFMpegExe, &QToolButton::clicked, this, &CPreferences::slotSelectFFMpegExe );
            fImpl->ffmpegExe->setIsOKFunction( [ ]( const QString & fileName )
            {
                auto fi = QFileInfo( fileName );
                return fileName.isEmpty() || (fi.exists() && fi.isFile() && fi.isExecutable());
            }, tr( "File '%1' does not Exist or is not an Executable" ) );

            connect( fImpl->btnSelectFFProbeExe, &QToolButton::clicked, this, &CPreferences::slotSelectFFProbeExe );
            fImpl->ffprobeExe->setIsOKFunction( [ ]( const QString & fileName )
            {
                auto fi = QFileInfo( fileName );
                return fileName.isEmpty() || (fi.exists() && fi.isFile() && fi.isExecutable());
            }, tr( "File '%1' does not Exist or is not an Executable" ) );

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

            fIgnoreDirNamesModel = new QStringListModel( this );
            fImpl->dirNamesToIgnore->setModel( fIgnoreDirNamesModel );

            fIgnoreFileNamesModel = new QStringListModel( this );
            fImpl->fileNamesToIgnore->setModel( fIgnoreFileNamesModel );

            new NSABUtils::CButtonEnabler( fImpl->knownStrings, fImpl->btnDelKnownString );
            new NSABUtils::CButtonEnabler( fImpl->knownExtraStrings, fImpl->btnDelExtraString );
            new NSABUtils::CButtonEnabler( fImpl->knownAbbreviations, fImpl->btnDelAbbreviation );
            new NSABUtils::CButtonEnabler( fImpl->dirNamesToIgnore, fImpl->btnDelIgnoreDir );
            new NSABUtils::CButtonEnabler( fImpl->fileNamesToIgnore, fImpl->btnDelIgnoreFileName );

            loadSettings();

            fftoolToolChanged( fImpl->ffmpegExe );
            fftoolToolChanged( fImpl->ffprobeExe );
            mkvnixToolChanged( fImpl->mkvMergeExe );
            mkvnixToolChanged( fImpl->mkvPropEditExe );

            fPageMap =
            {
                 { "Extensions", fImpl->extensionsPage }
                ,{ "Known Strings", fImpl->removeFromPathsPage }
                ,{ "Remove from Paths", fImpl->removeFromPathsPage }
                ,{ "Extended/Extra Information", fImpl->extendedInfoPage }
                ,{ "Known Abbreviations", fImpl->abbreviationsPage }
                ,{ "Ignored", fImpl->ignoreDirPage }
                ,{ "Directory Names", fImpl->ignoreDirPage }
                ,{ "File Names", fImpl->ignoreFilesPage }
                ,{ "Transformation Settings", fImpl->transformationPage }
                ,{ "TV Shows", fImpl->tvShowPage }
                ,{ "Movies", fImpl->moviesPage }
                ,{ "External Tools", fImpl->externalToolsPage }
            };

            fImpl->pageSelector->expandAll();

            QSettings settings;
            auto items = fImpl->pageSelector->findItems( settings.value( "LastPrefPage", "Extensions" ).toString(), Qt::MatchExactly );
            if ( !items.empty() )
                fImpl->pageSelector->setCurrentItem( items.front() );
        }

        CPreferences::~CPreferences()
        {
            QSettings settings;
            auto currItem = fImpl->pageSelector->currentItem();
            settings.setValue( "LastPrefPage", currItem ? currItem->text( 0 ) : QString() );
        }

        void CPreferences::accept()
        {
            saveSettings();
            QDialog::accept();
        }

        void CPreferences::addString( const QString & title, const QString & label, QStringListModel * model, QListView * listView )
        {
            auto text = QInputDialog::getText( this, title, label ).trimmed();
            if ( text.isEmpty() )
                return;

            auto words = text.split( QRegularExpression( "\\s" ), Qt::SkipEmptyParts );
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
            addString( tr( "Add Known String" ), tr( "String:" ), fKnownStringModel, fImpl->knownStrings );
        }

        void CPreferences::slotDelKnownString()
        {
            delString( fKnownStringModel, fImpl->knownStrings );
        }

        void CPreferences::slotAddExtraString()
        {
            addString( tr( "Add Known String For Extended Information" ), tr( "String:" ), fExtraStringModel, fImpl->knownExtraStrings );
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

        void CPreferences::slotAddIgnoreFileName()
        {
            addString( tr( "Add File Name to Ignore" ), tr( "File Name:" ), fIgnoreFileNamesModel, fImpl->fileNamesToIgnore );
        }

        void CPreferences::slotDelIgnoreFileName()
        {
            delString( fIgnoreFileNamesModel, fImpl->fileNamesToIgnore );
        }

        void CPreferences::slotAddIgnoreDir()
        {
            addString( tr( "Add Directory Name to Ignore" ), tr( "Directory Name:" ), fIgnoreDirNamesModel, fImpl->dirNamesToIgnore );
        }

        void CPreferences::slotDelIgnoreDir()
        {
            delString( fIgnoreDirNamesModel, fImpl->dirNamesToIgnore );
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
            fIgnoreDirNamesModel->setStringList( NCore::CPreferences::instance()->getIgnoredDirectories() );
            fIgnoreFileNamesModel->setStringList( NCore::CPreferences::instance()->getIgnoredFileNames() );

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
            NCore::CPreferences::instance()->setIgnoredDirectories( fIgnoreDirNamesModel->stringList() );
            NCore::CPreferences::instance()->setIgnoredFileNames( fIgnoreFileNamesModel->stringList() );

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
            auto ii = fPageMap.find( curr->text( 0 ) );
            if ( ii == fPageMap.end() )
                return;
            fImpl->stackedWidget->setCurrentWidget( (*ii).second );
        }
    }
}
