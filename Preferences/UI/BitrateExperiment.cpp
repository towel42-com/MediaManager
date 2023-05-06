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

#include "BitrateExperiment.h"
#include "SABUtils/WidgetChanged.h"
#include "SABUtils/MediaInfo.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/FFMpegFormats.h"

#include "ui_BitrateExperiment.h"

#include <QLocale>
#include <QFileDialog>
#include <QMetaMethod>

#include <unordered_set>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CBitrateExperiment::CBitrateExperiment( QWidget *parent ) :
                QDialog( parent ),
                fImpl( new Ui::CBitrateExperiment )
            {
                fImpl->setupUi( this );
                connect( fImpl->resolutionName, qOverload< int >( &QComboBox::currentIndexChanged ), this, &CBitrateExperiment::slotResolutionChanged );
                NSABUtils::setupWidgetChanged( this, QMetaMethod::fromSignal( &CBitrateExperiment::sigChanged ), { fImpl->resolutionName } );
                connect( this, &CBitrateExperiment::sigChanged, this, &CBitrateExperiment::slotChanged );
                connect( fImpl->openBtn, &QToolButton::clicked, this, &CBitrateExperiment::slotOpenFile );
                connect( fImpl->loadFromFile, &QRadioButton::clicked, this, &CBitrateExperiment::slotSourceChanged );
                connect( fImpl->selectResolution, &QRadioButton::clicked, this, &CBitrateExperiment::slotSourceChanged );

                fImpl->target4kBitrate->setSuffix( "(kbps)" );
                fImpl->targetHDBitrate->setSuffix( "(kbps)" );
                fImpl->targetSubHDBitrate->setSuffix( "(kbps)" );
                fImpl->uncompressedBitrate->setSuffix( "(bps)" );
                fImpl->targetBitrate->setSuffix( "(bps)" );
            }

            CBitrateExperiment::~CBitrateExperiment()
            {
            }

            void CBitrateExperiment::setGreaterThan4kDivisor( int value )
            {
                fImpl->greaterThan4kDivisor->setValue( value );
            }

            int CBitrateExperiment::greaterThan4kDivisor() const
            {
                return fImpl->greaterThan4kDivisor->value();
            }

            void CBitrateExperiment::setTarget4kBitrate( const QString &value )
            {
                fImpl->target4kBitrate->setText( value );
            }

            QString CBitrateExperiment::getTarget4kBitrate() const
            {
                return fImpl->target4kBitrate->text();
            }

            void CBitrateExperiment::setTargetHDBitrate( const QString &value )
            {
                fImpl->targetHDBitrate->setText( value );
            }

            QString CBitrateExperiment::getTargetHDBitrate() const
            {
                return fImpl->targetHDBitrate->text();
            }

            void CBitrateExperiment::setTargetSubHDBitrate( const QString &value )
            {
                fImpl->targetSubHDBitrate->setText( value );
            }

            QString CBitrateExperiment::getTargetSubHDBitrate() const
            {
                return fImpl->targetSubHDBitrate->text();
            }

            void CBitrateExperiment::setResolutionThreshold( int value )
            {
                fImpl->resolutionThreshold->setValue( value );
            }

            int CBitrateExperiment::resolutionThreshold() const
            {
                return fImpl->resolutionThreshold->value();
            }

            void CBitrateExperiment::slotOpenFile()
            {
                auto allExtensions = NPreferences::NCore::CPreferences::instance()->getVideoExtensions( QStringList() << "*.nfo" );

                auto mediaFormats = NPreferences::NCore::CPreferences::instance()->getMediaFormats();
                auto formats = mediaFormats->encoderFormats( true ) << mediaFormats->decoderFormats( true );
                formats.push_front( R"(matroska - Matroska (*.mkv))" );
                formats.removeDuplicates();

                for ( auto ii = 0; ii < formats.count(); ++ii )
                {
                    bool found = false;
                    for ( auto &&jj : allExtensions )
                    {
                        if ( formats[ ii ].indexOf( jj ) != -1 )
                        {
                            found = true;
                            break;
                        }
                    }
                    if ( !found )
                    {
                        formats.removeAt( ii );
                        ii--;
                    }
                }

                formats.push_back( "All Files (*.*)" );
                formats.removeDuplicates();

                auto filter = formats.join( ";;" );
                auto file = QFileDialog::getOpenFileName( this, tr( "Select Video File:" ), fImpl->fileName->text(), filter );
                if ( file.isEmpty() )
                    return;
                fImpl->fileName->setText( file );
                loadFromFile();
            }

            void CBitrateExperiment::loadFromFile()
            {
                auto mediaInfo = NSABUtils::CMediaInfo( fImpl->fileName->text() );
                load( mediaInfo.getResolutionInfo() );
            }

            void CBitrateExperiment::slotResolutionChanged()
            {
                auto curr = fImpl->resolutionName->currentText();
                NSABUtils::SResolutionInfo resDef;

                if ( curr.startsWith( "8k" ) )
                    resDef = NSABUtils::CMediaInfo::k8KResolution;
                else if ( curr.startsWith( "4k" ) )
                    resDef = NSABUtils::CMediaInfo::k4KResolution;
                else if ( curr.startsWith( "1080p" ) )
                    resDef = NSABUtils::CMediaInfo::k1080pResolution;
                else if ( curr.startsWith( "1080i" ) )
                    resDef = NSABUtils::CMediaInfo::k1080iResolution;
                else if ( curr.startsWith( "720p" ) )
                    resDef = NSABUtils::CMediaInfo::k720Resolution;
                else if ( curr.startsWith( "SD" ) )
                    resDef = NSABUtils::CMediaInfo::k480Resolution;

                load( resDef );
            }

            void CBitrateExperiment::load( const NSABUtils::SResolutionInfo &resDef )
            {
                fDisableUpdate = true;
                fImpl->width->setValue( resDef.fResolution.first );
                fImpl->height->setValue( resDef.fResolution.second );
                fImpl->bitsPerColor->setValue( resDef.fBitsPerPixel / 3 );
                fImpl->fps->setValue( resDef.fFPS );
                fImpl->interlaced->setChecked( resDef.fInterlaced );
                fDisableUpdate = false;
                slotChanged();
            }

            std::shared_ptr< NSABUtils::SResolutionInfo > CBitrateExperiment::getResolutionDef() const
            {
                auto retVal = std::make_shared< NSABUtils::SResolutionInfo >();

                retVal->fResolution = { fImpl->width->value(), fImpl->height->value() };
                retVal->fBitsPerPixel = fImpl->bitsPerColor->value() * 3;
                retVal->fFPS = fImpl->fps->value();
                retVal->fInterlaced = fImpl->interlaced->isChecked();

                return retVal;
            }

            void CBitrateExperiment::slotChanged()
            {
                if ( fDisableUpdate )
                    return;

                auto resDef = getResolutionDef();
                auto uncompressed = resDef->idealBitrate();
                auto targetBitrate = NPreferences::NCore::CPreferences::getTargetBitrate( *getResolutionDef(), false, false, fImpl->greaterThan4kDivisor->value(), ( fImpl->resolutionThreshold->value() * 1.0 / 100.0 ), fImpl->target4kBitrate->text().toInt(), fImpl->targetHDBitrate->text().toInt(), fImpl->targetSubHDBitrate->text().toInt(), 1.0 );

                QLocale locale;
                fImpl->uncompressedBitrate->setText( locale.toString( static_cast< qulonglong >( uncompressed ) ) );
                fImpl->targetBitrate->setText( locale.toString( static_cast< qulonglong >( targetBitrate ) ) );
            }

            void CBitrateExperiment::slotSourceChanged()
            {
                fImpl->fileName->setEnabled( fImpl->loadFromFile->isChecked() );
                fImpl->openBtn->setEnabled( fImpl->loadFromFile->isChecked() );

                fImpl->resolutionName->setEnabled( fImpl->selectResolution->isChecked() );

                if ( fImpl->loadFromFile->isChecked() )
                {
                    loadFromFile();
                }
                else
                    slotResolutionChanged();
            }
        }
    }
}