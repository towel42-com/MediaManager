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
#include <QProgressDialog>
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

            connect( fImpl->btnAddString, &QToolButton::clicked, this, &CPreferences::slotAddString );
            connect( fImpl->btnDelString, &QToolButton::clicked, this, &CPreferences::slotDelString );
            connect( fImpl->btnSelectMKVMergeExe, &QToolButton::clicked, this, &CPreferences::slotSelectMKVMergeExe );

            fStringModel = new QStringListModel( this );
            fImpl->knownStrings->setModel( fStringModel );

            new CButtonEnabler( fImpl->knownStrings, fImpl->btnDelString );
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

        void CPreferences::slotAddString()
        {
            auto text = QInputDialog::getText( this, tr( "Add Known String" ), tr( "String:" ) );
            if ( text.isEmpty() )
                return;
            text = text.trimmed();

            auto words = text.split( QRegularExpression( "\\s" ), Qt::SkipEmptyParts );
            auto strings = fStringModel->stringList();
            for ( auto &&ii : words )
            {
                ii = ii.trimmed();
                strings.removeAll( ii );
            }
            strings << words;
            fStringModel->setStringList( strings );
            fImpl->knownStrings->scrollTo( fStringModel->index( strings.count() - 1, 0 ) );
        }

        void CPreferences::slotDelString()
        {
            auto model = fImpl->knownStrings->selectionModel();
            if ( !model )
                return;
            auto selected = model->selectedRows();
            if ( selected.isEmpty() )
                return;
            auto strings = fStringModel->stringList();
            for ( auto &&ii : selected )
            {
                auto text = ii.data().toString();
                strings.removeAll( text );
            }
            fStringModel->setStringList( strings );
            fImpl->knownStrings->scrollTo( fStringModel->index( selected.front().row(), 0 ) );
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

        void CPreferences::loadSettings()
        {
            QSettings settings;

            fStringModel->setStringList( NCore::CPreferences::instance()->getKnownStrings() );
            fImpl->mediaExtensions->setText( NCore::CPreferences::instance()->getMediaExtensions().join( ";" ) );
            fImpl->subtitleExtensions->setText( NCore::CPreferences::instance()->getSubtitleExtensions().join( ";" ) );
            fImpl->treatAsTVShowByDefault->setChecked( NCore::CPreferences::instance()->getTreatAsTVShowByDefault() );
            fImpl->exactMatchesOnly->setChecked( NCore::CPreferences::instance()->getExactMatchesOnly() );

            fImpl->tvOutFilePattern->setText( NCore::CPreferences::instance()->getTVOutFilePattern() );
            fImpl->tvOutDirPattern->setText( NCore::CPreferences::instance()->getTVOutDirPattern() );

            fImpl->movieOutFilePattern->setText( NCore::CPreferences::instance()->getMovieOutFilePattern() );
            fImpl->movieOutDirPattern->setText( NCore::CPreferences::instance()->getMovieOutDirPattern() );

            fImpl->mkvMergeExe->setText( NCore::CPreferences::instance()->getMKVMergeEXE() );
        }

        void CPreferences::saveSettings()
        {
            NCore::CPreferences::instance()->setKnownStrings( fStringModel->stringList() );
            NCore::CPreferences::instance()->setTreatAsTVShowByDefault( fImpl->treatAsTVShowByDefault->isChecked() );
            NCore::CPreferences::instance()->setExactMatchesOnly( fImpl->exactMatchesOnly->isChecked() );
            NCore::CPreferences::instance()->setMediaExtensions( fImpl->mediaExtensions->text() );
            NCore::CPreferences::instance()->setSubtitleExtensions( fImpl->subtitleExtensions->text() );

            NCore::CPreferences::instance()->setTVOutFilePattern( fImpl->tvOutFilePattern->text() );
            NCore::CPreferences::instance()->setTVOutDirPattern( fImpl->tvOutDirPattern->text() );

            NCore::CPreferences::instance()->setMovieOutFilePattern( fImpl->movieOutFilePattern->text() );
            NCore::CPreferences::instance()->setMovieOutDirPattern( fImpl->movieOutDirPattern->text() );
            NCore::CPreferences::instance()->setMKVMergeEXE( fImpl->mkvMergeExe->text() );
        }

    }
}
