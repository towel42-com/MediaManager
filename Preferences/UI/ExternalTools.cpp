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

#include "ExternalTools.h"
#include "Preferences/Core/Preferences.h"

#include "ui_ExternalTools.h"

#include <QFileDialog>
#include <QMessageBox>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CExternalTools::CExternalTools( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CExternalTools )
            {
                fImpl->setupUi( this );

                connect( fImpl->btnSelectMKVPropEditExe, &QToolButton::clicked, this, &CExternalTools::slotSelectMKVPropEditExe );
                fImpl->mkvPropEditExe->setCheckExists( true );
                fImpl->mkvPropEditExe->setCheckIsFile( true );
                fImpl->mkvPropEditExe->setCheckIsExecutable( true );

                connect( fImpl->btnSelectFFMpegExe, &QToolButton::clicked, this, &CExternalTools::slotSelectFFMpegExe );
                fImpl->ffmpegExe->setCheckExists( true );
                fImpl->ffmpegExe->setCheckIsFile( true );
                fImpl->ffmpegExe->setCheckIsExecutable( true );

                connect( fImpl->btnSelectFFMpegEmbyExe, &QToolButton::clicked, this, &CExternalTools::slotSelectFFMpegEmbyExe );
                fImpl->ffmpegExe->setCheckExists( true );
                fImpl->ffmpegExe->setCheckIsFile( true );
                fImpl->ffmpegExe->setCheckIsExecutable( true );

                connect( fImpl->btnSelectFFProbeExe, &QToolButton::clicked, this, &CExternalTools::slotSelectFFProbeExe );
                fImpl->ffprobeExe->setCheckExists( true );
                fImpl->ffprobeExe->setCheckIsFile( true );
                fImpl->ffprobeExe->setCheckIsExecutable( true );

                connect( fImpl->ffmpegExe, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CExternalTools::slotFFToolChanged );
                connect( fImpl->ffprobeExe, &NSABUtils::CDelayLineEdit::sigTextChangedAfterDelay, this, &CExternalTools::slotFFToolChanged );

                fftoolToolChanged( fImpl->ffmpegExe );
                fftoolToolChanged( fImpl->ffprobeExe );
            }

            CExternalTools::~CExternalTools()
            {
            }

            void CExternalTools::slotSelectMKVPropEditExe()
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

            void CExternalTools::slotSelectFFMpegExe()
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

            void CExternalTools::slotSelectFFMpegEmbyExe()
            {
                auto exe = QFileDialog::getOpenFileName( this, tr( "Select ffmpeg (From Emby) Executable:" ), fImpl->ffmpegEmbyExe->text(), "ffmpeg Executable (ffmpeg.exe);;All Executables (*.exe);;All Files (*.*)" );
                if ( !exe.isEmpty() && !QFileInfo( exe ).isExecutable() )
                {
                    QMessageBox::critical( this, "Not an Executable", tr( "The file '%1' is not an executable" ).arg( exe ) );
                    return;
                }

                if ( !exe.isEmpty() )
                    fImpl->ffmpegEmbyExe->setText( exe );
            }

            void CExternalTools::slotSelectFFProbeExe()
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

            void CExternalTools::updateOtherTool( QObject *sender, const std::pair< QLineEdit *, QString > &lhs, const std::pair< QLineEdit *, QString > &rhs )
            {
                auto le = dynamic_cast< QLineEdit * >( sender );
                if ( !le )
                    return;

                if ( le->text().isEmpty() || !QFileInfo::exists( le->text() ) )
                    return;

                QString otherExe;
                QLineEdit *otherLE = nullptr;
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

            void CExternalTools::slotFFToolChanged()
            {
                fftoolToolChanged( dynamic_cast< QLineEdit * >( sender() ) );
            }

            void CExternalTools::fftoolToolChanged( QLineEdit *le )
            {
                updateOtherTool( le, { fImpl->ffprobeExe, "ffprobe" }, { fImpl->ffmpegExe, "ffmpeg" } );
            }

            void CExternalTools::load()
            {
                fImpl->useCustomExternalTools->setChecked( NPreferences::NCore::CPreferences::instance()->getUseCustomExternalTools() );
                fImpl->mkvPropEditExe->setText( NPreferences::NCore::CPreferences::instance()->getMKVPropEditEXE() );
                fImpl->ffmpegExe->setText( NPreferences::NCore::CPreferences::instance()->getFFMpegEXE() );
                fImpl->ffmpegEmbyExe->setText( NPreferences::NCore::CPreferences::instance()->getFFMpegEmbyEXE() );
                fImpl->ffprobeExe->setText( NPreferences::NCore::CPreferences::instance()->getFFProbeEXE() );
            }

            void CExternalTools::save()
            {
                NPreferences::NCore::CPreferences::instance()->setUseCustomExternalTools( fImpl->useCustomExternalTools->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setMKVPropEditEXE( fImpl->mkvPropEditExe->text() );
                NPreferences::NCore::CPreferences::instance()->setFFMpegEXE( fImpl->ffmpegExe->text() );
                NPreferences::NCore::CPreferences::instance()->setFFMpegEmbyEXE( fImpl->ffmpegEmbyExe->text() );
                NPreferences::NCore::CPreferences::instance()->setFFProbeEXE( fImpl->ffprobeExe->text() );
            }
        }
    }
}
