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

#include "Preferences.h"
#include "Core/Preferences.h"

#include "ui_Preferences.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

#include "SABUtils/ButtonEnabler.h"

namespace NMediaManager
{
    namespace NUi
    {
        CPreferences::CPreferences( QWidget *parent )
            : QDialog( parent ),
            fImpl( new Ui::CPreferences )
        {
            fImpl->setupUi( this );

            connect( fImpl->btnAddKnownString, &QToolButton::clicked, this, &CPreferences::slotAddKnownString );
            connect( fImpl->btnDelKnownString, &QToolButton::clicked, this, &CPreferences::slotDelKnownString );
            connect(fImpl->btnAddExtraString, &QToolButton::clicked, this, &CPreferences::slotAddExtraString);
            connect(fImpl->btnDelExtraString, &QToolButton::clicked, this, &CPreferences::slotDelExtraString);

            connect( fImpl->btnSelectMKVMergeExe, &QToolButton::clicked, this, &CPreferences::slotSelectMKVMergeExe );
            fImpl->mkvMergeExe->setIsOKFunction( [ ]( const QString & fileName )
            {
                auto fi = QFileInfo( fileName );
                return fileName.isEmpty() || (fi.exists() && fi.isFile() && fi.isExecutable());
            }, tr( "File '%1' does not Exist or is not an Executable" ) );

            connect(fImpl->btnSelectMKVPropEditExe, &QToolButton::clicked, this, &CPreferences::slotSelectMKVPropEditExe);
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

            fKnownStringModel = new QStringListModel( this );
            fImpl->knownStrings->setModel( fKnownStringModel );

            fExtraStringModel = new QStringListModel(this);
            fImpl->knownExtraStrings->setModel(fExtraStringModel);

            new CButtonEnabler( fImpl->knownStrings, fImpl->btnDelKnownString );
            new CButtonEnabler(fImpl->knownExtraStrings, fImpl->btnDelExtraString);
            loadSettings();
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
            for ( auto &&ii : words )
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
            for ( auto &&ii : selected )
            {
                auto text = ii.data().toString();
                strings.removeAll( text );
            }
            fKnownStringModel->setStringList( strings );
            fImpl->knownStrings->scrollTo( fKnownStringModel->index( selected.front().row(), 0 ) );
        }

        void CPreferences::slotAddExtraString()
        {
            auto text = QInputDialog::getText(this, tr("Add Known String For Extended Information"), tr("String:"));
            if (text.isEmpty())
                return;
            text = text.trimmed();

            auto words = text.split(QRegularExpression("\\s"), Qt::SkipEmptyParts);
            auto strings = fExtraStringModel->stringList();
            for (auto && ii : words)
            {
                ii = ii.trimmed();
                strings.removeAll(ii);
            }
            strings << words;
            fExtraStringModel->setStringList(strings);
            fImpl->knownExtraStrings->scrollTo(fExtraStringModel->index(strings.count() - 1, 0));
        }

        void CPreferences::slotDelExtraString()
        {
            auto model = fImpl->knownStrings->selectionModel();
            if (!model)
                return;
            auto selected = model->selectedRows();
            if (selected.isEmpty())
                return;
            auto strings = fExtraStringModel->stringList();
            for (auto && ii : selected)
            {
                auto text = ii.data().toString();
                strings.removeAll(text);
            }
            fExtraStringModel->setStringList(strings);
            fImpl->knownExtraStrings->scrollTo(fKnownStringModel->index(selected.front().row(), 0));
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

        void CPreferences::slotSelectMKVPropEditExe()
        {
            auto exe = QFileDialog::getOpenFileName(this, tr("Select MKVPropEdit Executable:"), fImpl->mkvPropEditExe->text(), "mkvpropedit Executable (mkvpropedit.exe);;All Executables (*.exe);;All Files (*.*)");
            if (!exe.isEmpty() && !QFileInfo(exe).isExecutable())
            {
                QMessageBox::critical(this, "Not an Executable", tr("The file '%1' is not an executable").arg(exe));
                return;
            }

            if (!exe.isEmpty())
                fImpl->mkvPropEditExe->setText(exe);
        }

        void CPreferences::loadSettings()
        {
            QSettings settings;

            fKnownStringModel->setStringList( NCore::CPreferences::instance()->getKnownStrings() );
            fImpl->mediaExtensions->setText( NCore::CPreferences::instance()->getMediaExtensions().join( ";" ) );
            fImpl->subtitleExtensions->setText( NCore::CPreferences::instance()->getSubtitleExtensions().join( ";" ) );
            fImpl->treatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->exactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );

            fImpl->tvOutFilePattern->setText( NCore::CPreferences::instance()->getTVOutFilePattern() );
            fImpl->tvOutDirPattern->setText( NCore::CPreferences::instance()->getTVOutDirPattern() );

            fImpl->movieOutFilePattern->setText( NCore::CPreferences::instance()->getMovieOutFilePattern() );
            fImpl->movieOutDirPattern->setText( NCore::CPreferences::instance()->getMovieOutDirPattern() );

            fImpl->mkvMergeExe->setText( NCore::CPreferences::instance()->getMKVMergeEXE() );
            fImpl->mkvPropEditExe->setText(NCore::CPreferences::instance()->getMKVPropEditEXE());
            fImpl->ffmpegExe->setText( NCore::CPreferences::instance()->getFFMpegEXE() );
        }

        void CPreferences::saveSettings()
        {
            NCore::CPreferences::instance()->setKnownStrings( fKnownStringModel->stringList() );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->treatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->exactMatchesOnly->isChecked() );
            NCore::CPreferences::instance()->setMediaExtensions( fImpl->mediaExtensions->text() );
            NCore::CPreferences::instance()->setSubtitleExtensions( fImpl->subtitleExtensions->text() );

            NCore::CPreferences::instance()->setTVOutFilePattern( fImpl->tvOutFilePattern->text() );
            NCore::CPreferences::instance()->setTVOutDirPattern( fImpl->tvOutDirPattern->text() );

            NCore::CPreferences::instance()->setMovieOutFilePattern( fImpl->movieOutFilePattern->text() );
            NCore::CPreferences::instance()->setMovieOutDirPattern( fImpl->movieOutDirPattern->text() );
            NCore::CPreferences::instance()->setMKVMergeEXE( fImpl->mkvMergeExe->text() );
            NCore::CPreferences::instance()->setMKVPropEditEXE(fImpl->mkvPropEditExe->text());
            NCore::CPreferences::instance()->setFFMpegEXE( fImpl->ffmpegExe->text() );
        }
    }
}
