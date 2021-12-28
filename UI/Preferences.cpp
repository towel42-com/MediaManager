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

            new NSABUtils::CButtonEnabler( fImpl->knownStrings, fImpl->btnDelKnownString );
            new NSABUtils::CButtonEnabler( fImpl->knownExtraStrings, fImpl->btnDelExtraString );
            new NSABUtils::CButtonEnabler( fImpl->knownAbbreviations, fImpl->btnDelAbbreviation );

            loadSettings();

            fftoolToolChanged( fImpl->ffmpegExe );
            fftoolToolChanged( fImpl->ffprobeExe );
            mkvnixToolChanged( fImpl->mkvMergeExe );
            mkvnixToolChanged( fImpl->mkvPropEditExe );

            QSettings settings;
            fImpl->tabWidget->setCurrentIndex( settings.value( "LastPrefPage", 0 ).toInt() );
        }

        CPreferences::~CPreferences()
        {
            QSettings settings;
            settings.setValue( "LastPrefPage", fImpl->tabWidget->currentIndex() );
        }

        void CPreferences::accept()
        {
            saveSettings();
            QDialog::accept();
        }

        void CPreferences::slotAddKnownString()
        {
            auto text = QInputDialog::getText( this, tr( "Add Known String" ), tr( "String:" ) );
            if ( text.isEmpty() )
                return;
            text = text.trimmed();

            auto words = text.split( QRegularExpression( "\\s" ), Qt::SkipEmptyParts );
            auto strings = fKnownStringModel->stringList();
            for ( auto && ii : words )
            {
                ii = ii.trimmed();
                strings.removeAll( ii );
            }
            strings << words;
            fKnownStringModel->setStringList( strings );
            fImpl->knownStrings->scrollTo( fKnownStringModel->index( strings.count() - 1, 0 ) );
        }

        void CPreferences::slotDelKnownString()
        {
            auto model = fImpl->knownStrings->selectionModel();
            if ( !model )
                return;
            auto selected = model->selectedRows();
            if ( selected.isEmpty() )
                return;
            auto strings = fKnownStringModel->stringList();
            for ( auto && ii : selected )
            {
                auto text = ii.data().toString();
                strings.removeAll( text );
            }
            fKnownStringModel->setStringList( strings );
            fImpl->knownStrings->scrollTo( fKnownStringModel->index( selected.front().row(), 0 ) );
        }

        void CPreferences::slotAddExtraString()
        {
            auto text = QInputDialog::getText( this, tr( "Add Known String For Extended Information" ), tr( "String:" ) );
            if ( text.isEmpty() )
                return;
            text = text.trimmed();

            auto words = text.split( QRegularExpression( "\\s" ), Qt::SkipEmptyParts );
            auto strings = fExtraStringModel->stringList();
            for ( auto && ii : words )
            {
                ii = ii.trimmed();
                strings.removeAll( ii );
            }
            strings << words;
            fExtraStringModel->setStringList( strings );
            fImpl->knownExtraStrings->scrollTo( fExtraStringModel->index( strings.count() - 1, 0 ) );
        }

        void CPreferences::slotDelExtraString()
        {
            auto model = fImpl->knownStrings->selectionModel();
            if ( !model )
                return;
            auto selected = model->selectedRows();
            if ( selected.isEmpty() )
                return;
            auto strings = fExtraStringModel->stringList();
            for ( auto && ii : selected )
            {
                auto text = ii.data().toString();
                strings.removeAll( text );
            }
            fExtraStringModel->setStringList( strings );
            fImpl->knownExtraStrings->scrollTo( fKnownStringModel->index( selected.front().row(), 0 ) );
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
    }
}
