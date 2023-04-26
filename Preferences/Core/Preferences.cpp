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
#include "TranscodeNeeded.h"

#include "Core/LanguageInfo.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/GPUDetect.h"
#include "SABUtils/ScrollMessageBox.h"
#include "SABUtils/FFMpegFormats.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QMap>
#include <QDir>
#include <QVariant>
#include <QString>
#include <QTimer>
#include <QMessageBox>
#include <QClipboard>
#include <QGuiApplication>
#include <QPushButton>
#include <QLabel>
#include <QImageReader>
#include <QProcess>

#include <optional>
#include <unordered_set>
#include <typeinfo>

namespace NMediaManager
{
    namespace NPreferences
    {
        QString toString( EItemStatus status )
        {
            switch ( status )
            {
                case EItemStatus::eOK:
                    return "OK";
                case EItemStatus::eWarning:
                    return "Warning";
                case EItemStatus::eError:
                    return "Error";
            }
            return {};
        }

        QString toString( EPreferenceType prefType, bool forEnum )
        {
            switch ( prefType )
            {
                case eSystemPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eSystemPrefs" : "System";
                case eLoadPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eLoadPrefs" : "Load";
                case eMediaRenamerPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eMediaRenamerPrefs" : "MediaRenamer";
                case eTagPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eTagPrefs" : "Tags";
                case eExtToolsPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eExtToolsPrefs" : "ExternalTools";
                case eGIFPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eGIFPrefs" : "GIF";
                case eBIFPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eBIFPrefs" : "BIF";
                case eTranscodePrefs:
                    return forEnum ? "NMediaManager::NPreferences::eTranscodePrefs" : "Transcode";
                default:
                    return "";
            }
        }

        namespace NCore
        {
            QString toString( ETranscodePreset preset, bool forEnum )
            {
                switch ( preset )
                {
                    case ETranscodePreset::eUltraFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eUltraFast" : "ultrafast";
                    case ETranscodePreset::eSuperFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eSuperFast" : "superfast";
                    case ETranscodePreset::eVeryFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eVeryFast" : "veryfast";
                    case ETranscodePreset::eFaster:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eFaster" : "faster";
                    case ETranscodePreset::eFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eFast" : "fast";
                    case ETranscodePreset::eMedium:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eMedium" : "medium";
                    case ETranscodePreset::eSlow:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eSlow" : "slow";
                    case ETranscodePreset::eSlower:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eSlower" : "slower";
                    case ETranscodePreset::eVerySlow:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodePreset::eVerySlow" : "veryslow";
                }
                return toString( ETranscodePreset::eMedium, forEnum );
            }

            QString toString( ETranscodeTune preset, bool forEnum )
            {
                switch ( preset )
                {
                    case ETranscodeTune::eFilm:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeTune::eFilm" : "film";
                    case ETranscodeTune::eAnimation:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeTune::eAnimation" : "animation";
                    case ETranscodeTune::eGrain:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeTune::eGrain" : "grain";
                    case ETranscodeTune::eStillImage:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeTune::eStilImage" : "stillimage";
                    case ETranscodeTune::eFastDecode:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeTune::eFastDecode" : "fastdecode";
                    case ETranscodeTune::eZeroLatency:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeTune::eZeroLatency" : "zerolatency";
                }
                return toString( ETranscodeTune::eFilm, forEnum );
            }

            QString toString( ETranscodeProfile profile, bool forEnum )
            {
                switch ( profile )
                {
                    case ETranscodeProfile::eMain:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain" : "main";
                    case ETranscodeProfile::eMainIntra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMainIntra" : "main-intra";
                    case ETranscodeProfile::eMailStillPicture:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMailStillPicture" : "mailstillpicture";
                    case ETranscodeProfile::eMain444_8:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain444_8" : "main444-8";
                    case ETranscodeProfile::eMain444Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain444Intra" : "main444-intra";
                    case ETranscodeProfile::eMain444StillPicture:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain444StillPicture" : "main444-stillpicture";
                    case ETranscodeProfile::eMain10:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain10" : "main10";
                    case ETranscodeProfile::eMain10Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain10Intra" : "main10-intra";
                    case ETranscodeProfile::eMain422_10:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain422_10" : "main422-10";
                    case ETranscodeProfile::eMain422_10Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain422_10Intra" : "main422-10-intra";
                    case ETranscodeProfile::eMain444_10:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain444_10" : "main444-10";
                    case ETranscodeProfile::eMain444_10Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain444_10Intra" : "main444-10-intra";
                    case ETranscodeProfile::eMain12:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain12" : "main12";
                    case ETranscodeProfile::eMain12Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain12Intra" : "main12-intra";
                    case ETranscodeProfile::eMain422_12:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain422_12" : "main422-12";
                    case ETranscodeProfile::eMain422_12Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain422_12Intra" : "main422-12-intra";
                    case ETranscodeProfile::eMain444_12:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain444_12" : "main444-12";
                    case ETranscodeProfile::eMain444_12Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::ETranscodeProfile::eMain444_12Intra" : "main444012-intra";
                }
                return toString( ETranscodeProfile::eMain, forEnum );
            }

            CPreferences *CPreferences::instance()
            {
                static CPreferences retVal;
                return &retVal;
            }

            CPreferences::CPreferences()
            {
                fMediaFormats = std::make_unique< NSABUtils::CFFMpegFormats >( getFFMpegEXE() );
                connect( NSABUtils::CMediaInfoMgr::instance(), &NSABUtils::CMediaInfoMgr::sigMediaLoaded, this, &CPreferences::sigMediaInfoLoaded );
            }

            CPreferences::~CPreferences()
            {
            }

            /// ////////////////////////////////////////////////////////
            /// Color Options
            /// ////////////////////////////////////////////////////////
            QColor CPreferences::getColorForStatus( EItemStatus status, bool background ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eColorsPrefs ) );
                QColor defaultColor = QColor();
                switch ( status )
                {
                    case EItemStatus::eOK:
                        defaultColor = QColor();
                        break;
                    case EItemStatus::eError:
                        defaultColor = background ? Qt::red : Qt::black;
                        break;
                    case EItemStatus::eWarning:
                        defaultColor = background ? Qt::yellow : Qt::black;
                        break;
                };
                return settings.value( QString( "%1-%2ground" ).arg( toString( status ) ).arg( background ? "Back" : "Fore" ), defaultColor ).value< QColor >();
            }

            void CPreferences::setColorForStatus( EItemStatus status, bool background, const QColor &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eColorsPrefs ) );
                settings.setValue( QString( "%1-%2ground" ).arg( toString( status ) ).arg( background ? "Back" : "Fore" ), value );
                emitSigPreferencesChanged( EPreferenceType::eColorsPrefs );
            }

            /// ////////////////////////////////////////////////////////
            /// System Options
            /// ////////////////////////////////////////////////////////

            void CPreferences::setPageVisible( const QString &pageName, bool isVisible )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( QString( "PageVisible-%1" ).arg( pageName ), isVisible );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            bool CPreferences::getPageVisible( const QString &pageName ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( QString( "PageVisible-%1" ).arg( pageName ), true ).toBool();
            }

            int CPreferences::getNumSearchPages() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( "NumSearchPages", -1 ).toInt();
            }

            void CPreferences::setNumSearchPages( int numpages )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "NumSearchPages", numpages );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            QSize CPreferences::getThumbnailSize( const QFileInfo &fi ) const
            {
                auto mediaInfo = NSABUtils::CMediaInfo( fi.absoluteFilePath() );
                auto tags = mediaInfo.getMediaTags( { NSABUtils::EMediaTags::eWidth, NSABUtils::EMediaTags::eHeight, NSABUtils::EMediaTags::eAspectRatio } );

                auto width = tags[ NSABUtils::EMediaTags::eWidth ].toInt();
                auto height = tags[ NSABUtils::EMediaTags::eHeight ].toInt();

                auto aspectRatio = tags[ NSABUtils::EMediaTags::eAspectRatio ].toDouble();   // w/h
                if ( aspectRatio == 0.0 )
                    aspectRatio = ( 1.0 * width ) / ( 1.0 * height );

                if ( byImageWidth() )
                {
                    width = imageWidth();
                    height = width / aspectRatio;
                }
                else if ( byImageHeight() )
                {
                    height = imageHeight();
                    width = height * aspectRatio;
                }

                return QSize( width, height );
            }

            QString CPreferences::getImageFileName( const QFileInfo &fi, const QSize &sz, const QString &ext ) const
            {
                return fi.absoluteDir().absoluteFilePath( fi.completeBaseName() + QString( "-%1x%2-%3.%4" ).arg( sz.width() ).arg( sz.height() ).arg( imageInterval() ).arg( ext ) );
            }

            QString CPreferences::getImageFileName( const QFileInfo &fi, const QString &ext ) const
            {
                auto sz = getThumbnailSize( fi );
                return getImageFileName( fi, sz, ext );
            }

            QStringList CPreferences::cleanUpPaths( const QStringList &paths, bool /*areDirs*/ ) const
            {
                QStringList retVal;
                for ( auto &&ii : paths )
                {
                    QString curr;
                    if ( NSABUtils::NFileUtils::isIPAddressNetworkPath( ii ) )
                        curr = ii;
                    else
                        curr = NSABUtils::NFileUtils::getCorrectPathCase( ii );
                    if ( curr.isEmpty() )
                        continue;
                    retVal << curr;
                }
                retVal.removeDuplicates();
                return retVal;
            }

            void CPreferences::setDirectories( const QStringList &dir )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "Directories", dir );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            QStringList CPreferences::getDirectories() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                auto dirs = cleanUpPaths( settings.value( "Directories", QStringList() ).toStringList(), true );
                return dirs;
            }

            void CPreferences::setFileNames( const QStringList &dir )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "FileNames", dir );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            QStringList CPreferences::getFileNames() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                auto retVal = settings.value( "FileNames", QStringList() ).toStringList();
                for ( auto &&ii : retVal )
                {
                    ii = QDir::toNativeSeparators( ii );
                }
                retVal.removeDuplicates();
                return retVal;
            }

            QStringList CPreferences::getVideoExtensions( const QStringList &exclude /*= {} */ ) const
            {
                return getMediaFormats()->getVideoExtensions( exclude );
            }

            QStringList CPreferences::getAudioExtensions( const QStringList &exclude /*= {} */ ) const
            {
                return getMediaFormats()->getAudioExtensions( exclude );
            }

            QStringList CPreferences::getSubtitleExtensions( const QStringList &exclude /*= {} */ ) const
            {
                return getMediaFormats()->getSubtitleExtensions( exclude );
            }

            QStringList CPreferences::getVideoEncoderExtensions( const QStringList &exclude ) const
            {
                return getMediaFormats()->getVideoEncoderExtensions( exclude );
            }

            QStringList CPreferences::getAudioEncoderExtensions( const QStringList &exclude ) const
            {
                return getMediaFormats()->getAudioEncoderExtensions( exclude );
            }

            QStringList CPreferences::getSubtitleEncoderExtensions( const QStringList &exclude ) const
            {
                return getMediaFormats()->getSubtitleEncoderExtensions( exclude );
            }

            QStringList CPreferences::getVideoDecoderExtensions( const QStringList &exclude ) const
            {
                return getMediaFormats()->getVideoDecoderExtensions( exclude );
            }

            QStringList CPreferences::getAudioDecoderExtensions( const QStringList &exclude ) const
            {
                return getMediaFormats()->getAudioDecoderExtensions( exclude );
            }

            QStringList CPreferences::getSubtitleDecoderExtensions( const QStringList &exclude ) const
            {
                return getMediaFormats()->getSubtitleDecoderExtensions( exclude );
            }

            void CPreferences::setLoadMediaInfo( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "LoadMediaInfo", value );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            bool CPreferences::getLoadMediaInfo() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( "LoadMediaInfo", true ).toBool();
            }

            void CPreferences::setBackgroundLoadMediaInfo( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "BackgroundLoadMediaInfo", value );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            bool CPreferences::getBackgroundLoadMediaInfo() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( "BackgroundLoadMediaInfo", true ).toBool();
            }

            /// ////////////////////////////////////////////////////////
            /// transform Options
            /// ////////////////////////////////////////////////////////
            void CPreferences::setTreatAsTVShowByDefault( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "TreatAsTVShowByDefault", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::getTreatAsTVShowByDefault() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "TreatAsTVShowByDefault", false ).toBool();
            }

            bool CPreferences::isEncoderFormat( const QString &fileName, const QString &formatName ) const
            {
                return isEncoderFormat( QFileInfo( fileName ), formatName );
            }

            bool CPreferences::isEncoderFormat( const QFileInfo &fi, const QString &formatName ) const
            {
                return fMediaFormats->isEncoderFormat( fi.suffix(), formatName );
            }

            bool CPreferences::isEncoderFormat( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &formatName ) const
            {
                return isEncoderFormat( QFileInfo( mediaInfo->fileName() ), formatName );
            }

            bool CPreferences::isDecoderFormat( const QString &fileName, const QString &formatName ) const
            {
                return isDecoderFormat( QFileInfo( fileName ), formatName );
            }

            bool CPreferences::isDecoderFormat( const QFileInfo &fi, const QString &formatName ) const
            {
                return fMediaFormats->isDecoderFormat( fi.suffix(), formatName );
            }

            bool CPreferences::isDecoderFormat( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &formatName ) const
            {
                return isDecoderFormat( QFileInfo( mediaInfo->fileName() ), formatName );
            }

            void CPreferences::setExactMatchesOnly( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "ExactMatchesOnly", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::getExactMatchesOnly() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "ExactMatchesOnly", false ).toBool();
            }

            void CPreferences::setOnlyTransformDirectories( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "OnlyLoadDirectories", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::getOnlyTransformDirectories() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "OnlyLoadDirectories", false ).toBool();
            }

            void CPreferences::setTVOutFilePattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "OutFilePattern", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QString CPreferences::getTVOutFilePattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForTV" );

                return settings.value( "OutFilePattern", getDefaultOutFilePattern( true ) ).toString();
            }

            void CPreferences::setSeasonOutDirPattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "SeasonDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QString CPreferences::getSeasonOutDirPattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForTV" );

                return settings.value( "SeasonDirPattern", getDefaultSeasonDirPattern() ).toString();
            }

            void CPreferences::setTVOutDirPattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "OutDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QString CPreferences::getTVOutDirPattern( bool expandSeasonDir ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForTV" );

                auto retVal = settings.value( "OutDirPattern", getDefaultOutDirPattern( true ) ).toString();
                if ( expandSeasonDir )
                    retVal = retVal.replace( "<season_dir>", getSeasonOutDirPattern() );
                return retVal;
            }

            void CPreferences::setMovieOutFilePattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForMovies" );
                settings.setValue( "OutFilePattern", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QString CPreferences::getMovieOutFilePattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForMovies" );

                return settings.value( "OutFilePattern", getDefaultOutFilePattern( false ) ).toString();
            }

            void CPreferences::setMovieOutDirPattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForMovies" );
                settings.setValue( "OutDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QString CPreferences::getMovieOutDirPattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.beginGroup( "ForMovies" );

                return settings.value( "OutDirPattern", getDefaultOutDirPattern( false ) ).toString();
            }

            void CPreferences::setDeleteCustom( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "DeleteCustom", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::deleteCustom() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "DeleteCustom", true ).toBool();
            }

            void CPreferences::setDeleteEXE( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "DeleteEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::deleteEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "DeleteEXE", true ).toBool();
            }

            void CPreferences::setDeleteNFO( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "DeleteNFO", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::deleteNFO() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "DeleteNFO", true ).toBool();
            }

            void CPreferences::setDeleteBAK( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "DeleteBAK", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::deleteBAK() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "DeleteBAK", true ).toBool();
            }

            void CPreferences::setDeleteImages( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "DeleteImages", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::deleteImages() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "DeleteImages", true ).toBool();
            }

            QStringList CPreferences::imageExtensions() const
            {
                auto imageFormats = QImageReader::supportedImageFormats();

                QStringList retVal;
                for ( auto &&ii : imageFormats )
                    retVal << "*." + ii;

                return retVal;
            }

            void CPreferences::setDeleteTXT( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "DeleteTXT", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            bool CPreferences::deleteTXT() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "DeleteTXT", true ).toBool();
            }

            void CPreferences::setCustomPathsToDelete( const QStringList &values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                const QStringList &realValues = values;
                settings.setValue( "CustomToDelete", realValues );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QStringList CPreferences::getCustomPathsToDelete() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "CustomToDelete", getDefaultCustomPathsToDelete() ).toStringList();
            }

            void CPreferences::setRippedWithMakeMKVRegEX( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "RippedWithMKVRegEX", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QString CPreferences::getRippedWithMakeMKVRegEX() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                auto retVal = settings.value( "RippedWithMKVRegEX", getDefaultRippedWithMakeMKVRegEX() ).toString();
#ifdef _DEBUG
                Q_ASSERT_X( retVal.indexOf( "?<num>" ) >= 0, "getRippedWithMKVRegEX", "Invalid setting must contain the capture of num" );
#endif
                return retVal;
            }

            QStringList CPreferences::getExtensionsToDelete() const
            {
                QStringList retVal;
                if ( deleteEXE() )
                    retVal << "*.exe";
                if ( deleteBAK() )
                    retVal << "*.bak";
                if ( deleteNFO() )
                    retVal << "*.nfo";
                if ( deleteTXT() )
                    retVal << "*.txt";
                if ( deleteImages() )
                    retVal << imageExtensions();
                if ( deleteCustom() )
                    retVal << getCustomPathsToDelete();
                return retVal;
            }

            bool CPreferences::isPathToDelete( const QString &path ) const
            {
                auto fn = QFileInfo( path ).fileName();
                auto toDelete = getExtensionsToDelete();
                for ( auto &&ii : toDelete )
                {
                    auto regExStr = QRegularExpression::wildcardToRegularExpression( ii );
                    QRegularExpression::PatternOptions options = QRegularExpression::PatternOption::NoPatternOption;
#ifdef Q_OS_WINDOWS
                    options |= QRegularExpression::PatternOption::CaseInsensitiveOption;
#endif;
                    auto regExp = QRegularExpression( regExStr, options );

                    if ( regExp.match( fn ).hasMatch() )
                        return true;
                }
                return false;
            }

            void CPreferences::addKnownStrings( const QStringList &value )
            {
                auto knownWords = getKnownStrings();
                for ( auto &&ii : value )
                {
                    if ( !knownWords.contains( ii ) )
                        knownWords << value;
                }
                setKnownStrings( knownWords );
            }

            void CPreferences::setKnownStrings( const QStringList &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                QSet< QString > tmp;
                QStringList realValues;
                for ( auto &&ii : value )
                {
                    if ( !tmp.contains( ii ) )
                        realValues << ii;
                }

                settings.setValue( "KnownStrings", realValues );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            struct SCmp
            {
                bool operator()( const QString &lhs, const QString &rhs ) const
                {
                    if ( lhs.compare( rhs, Qt::CaseInsensitive ) == 0 )
                    {
                        return false;
                    }
                    if ( lhs.startsWith( rhs, Qt::CaseInsensitive ) )
                        return true;
                    if ( rhs.startsWith( lhs, Qt::CaseInsensitive ) )
                        return false;
                    return lhs.compare( rhs, Qt::CaseInsensitive ) < 0;
                }
            };

            QStringList CPreferences::getKnownStrings() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                auto tmp = settings.value( "KnownStrings", getDefaultKnownStrings() ).toStringList();
                auto tmp2 = std::set< QString, SCmp >( { tmp.begin(), tmp.end() } );
                QStringList retVal;
                for ( auto &&ii : tmp2 )
                    retVal.push_back( ii );

                return retVal;
            }

            QStringList CPreferences::getKnownStringRegExs() const
            {
                if ( fKnownStringRegExsCache.isEmpty() )
                {
                    auto strings = getKnownStrings();
                    QStringList nonRegExs;
                    for ( auto &&ii : strings )
                    {
                        bool isRegEx = ( ii.indexOf( "\\" ) != -1 ) || ( ii.indexOf( "?" ) != -1 ) || ( ii.indexOf( "{" ) != -1 ) || ( ii.indexOf( "}" ) != -1 );

                        if ( isRegEx )
                            fKnownStringRegExsCache << QString( "(?<word>" + ii + ")" );
                        else
                            nonRegExs << QRegularExpression::escape( ii );
                    }
                    auto primRegEx = R"(((?<prefix>\[|\()|\W)(?<word>)" + nonRegExs.join( "|" ) + R"()((?<suffix>\]|\))|\W|$))";
                    fKnownStringRegExsCache << primRegEx;
                }
                return fKnownStringRegExsCache;
            }

            void CPreferences::setKnownExtendedStrings( const QStringList &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "KnownExtendedStrings", value );
                fKnownStringRegExsCache.clear();
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QStringList CPreferences::getKnownExtendedStrings() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "KnownExtendedStrings", getDefaultKnownExtendedStrings() ).toStringList();
            }

            void CPreferences::setKnownAbbreviations( const QList< QPair< QString, QString > > &value )
            {
                QVariantMap map;
                for ( auto &&ii : value )
                {
                    map[ ii.first ] = ii.second;
                }
                return setKnownAbbreviations( map );
            }

            void CPreferences::setKnownAbbreviations( const QVariantMap &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "KnownAbbreviations", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QVariantMap CPreferences::getKnownAbbreviations() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "KnownAbbreviations", getDefaultKnownAbbreviations() ).toMap();
            };

            void CPreferences::setKnownHyphenated( const QStringList &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                settings.setValue( "KnownHyphenated", value );
                emitSigPreferencesChanged( EPreferenceType::eMediaRenamerPrefs );
            }

            QStringList CPreferences::getKnownHyphenated() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMediaRenamerPrefs ) );
                return settings.value( "KnownHyphenated", getDefaultKnownHyphenated() ).toStringList();
            };

            std::list< std::pair< QString, int > > CPreferences::getKnownHyphenatedData() const
            {
                std::list< std::pair< QString, int > > retVal;
                auto data = getKnownHyphenated();
                for ( auto &&ii : data )
                {
                    retVal.push_back( { ii, ii.indexOf( '-' ) } );
                }
                return retVal;
            }

            /// ////////////////////////////////////////////////////////
            /// Load Options
            /// ////////////////////////////////////////////////////////
            bool CPreferences::containsValue( const QString &value, const QStringList &values ) const
            {
                for ( auto &&ii : values )
                {
                    QRegularExpression::PatternOption option = QRegularExpression::NoPatternOption;
#ifdef Q_OS_WINDOWS
                    option = QRegularExpression::CaseInsensitiveOption;
#endif
                    auto regExp = QRegularExpression( "^" + ii + "$", option );
                    if ( regExp.match( value ).hasMatch() )
                        return true;
                }
                return false;
            }

            bool CPreferences::pathMatches( QString pathName, const QStringList &values ) const
            {
#ifdef Q_OS_WINDOWS
                pathName = pathName.toLower();
#endif
                return containsValue( pathName, values );
            }

            bool CPreferences::pathMatches( const QFileInfo &fileInfo, const QStringList &values ) const
            {
                return pathMatches( fileInfo.fileName(), values );
            }

            bool CPreferences::pathMatches( const QDir &dir, const QStringList &values ) const
            {
                return pathMatches( dir.absolutePath(), values );
            }

            bool CPreferences::isSkippedPath( bool forMediaNaming, const QFileInfo &fileInfo ) const
            {
                return pathMatches( fileInfo, getSkippedPaths( forMediaNaming ) );
            }

            bool CPreferences::isSkippedPath( bool forMediaNaming, const QDir &dir ) const
            {
                return pathMatches( dir, getSkippedPaths( forMediaNaming ) );
            }

            void CPreferences::setSkippedPaths( bool forMediaNaming, const QStringList &values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                QStringList realValues = values;
#ifdef Q_OS_WINDOWS
                for ( auto &&ii : realValues )
                    ii = ii.toLower();
#endif
                settings.setValue( QString( "SkippedDirs (%1)" ).arg( forMediaNaming ? "Media Transform" : "Media Tagging" ), realValues );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            void CPreferences::setIgnorePathNamesToSkip( bool forMediaNaming, bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                settings.setValue( QString( "IgnoreSkipFileNames (%1)" ).arg( forMediaNaming ? "Media Transform" : "Media Tagging" ), value );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            bool CPreferences::getIgnorePathNamesToSkip( bool forMediaNaming ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( QString( "IgnoreSkipFileNames (%1)" ).arg( forMediaNaming ? "Media Transform" : "Media Tagging" ), false ).toBool();
            }

            QStringList CPreferences::getSkippedPaths( bool forMediaNaming ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( QString( "SkippedDirs (%1)" ).arg( forMediaNaming ? "Media Transform" : "Media Tagging" ), getDefaultSkippedPaths( forMediaNaming ) ).toStringList();
            }

            bool CPreferences::isIgnoredPath( const QFileInfo &fileInfo ) const
            {
                return pathMatches( fileInfo, getIgnoredPaths() );
            }

            bool CPreferences::isIgnoredPath( const QDir &dir ) const
            {
                return pathMatches( dir, getIgnoredPaths() );
            }

            void CPreferences::setIgnoredPaths( const QStringList &values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                QStringList realValues = values;
#ifdef Q_OS_WINDOWS
                for ( auto &&ii : realValues )
                    ii = ii.toLower();
#endif
                settings.setValue( "IgnoredFileNames", realValues );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            bool CPreferences::getIgnorePathNamesToIgnore() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( "IgnoreIgnoredFileNames", false ).toBool();
            }

            void CPreferences::setIgnorePathNamesToIgnore( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                settings.setValue( "IgnoreIgnoredFileNames", value );
                emitSigPreferencesChanged( EPreferenceType::eLoadPrefs );
            }

            QStringList CPreferences::getIgnoredPaths() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eLoadPrefs ) );
                return settings.value( "IgnoredFileNames", getDefaultIgnoredPaths() ).toStringList();
            }

            /// ////////////////////////////////////////////////////////
            /// Tag Options
            /// ////////////////////////////////////////////////////////

            bool CPreferences::getVerifyMediaTags() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaTags", true ).toBool();
            }

            void CPreferences::setVerifyMediaTags( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaTags", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            std::list< std::pair< NSABUtils::EMediaTags, bool > > CPreferences::getAllMediaTags() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );

                std::list< std::pair< NSABUtils::EMediaTags, bool > > retVal = {
                    { NSABUtils::EMediaTags::eTitle, true }, { NSABUtils::EMediaTags::eLength, true }, { NSABUtils::EMediaTags::eDate, true }, { NSABUtils::EMediaTags::eComment, true }, { NSABUtils::EMediaTags::eBPM, true }, { NSABUtils::EMediaTags::eArtist, true }, { NSABUtils::EMediaTags::eComposer, true }, { NSABUtils::EMediaTags::eGenre, true }, { NSABUtils::EMediaTags::eTrack, true }, { NSABUtils::EMediaTags::eAlbum, false }, { NSABUtils::EMediaTags::eAlbumArtist, false }, { NSABUtils::EMediaTags::eDiscnumber, false }, { NSABUtils::EMediaTags::eAspectRatio, false }, { NSABUtils::EMediaTags::eWidth, false }, { NSABUtils::EMediaTags::eHeight, false }, { NSABUtils::EMediaTags::eResolution, false }, { NSABUtils::EMediaTags::eAllVideoCodecs, false }, { NSABUtils::EMediaTags::eAllAudioCodecsDisp, false }, { NSABUtils::EMediaTags::eVideoBitrateString, false }, { NSABUtils::EMediaTags::eOverAllBitrateString, false }, { NSABUtils::EMediaTags::eNumChannels, false }, { NSABUtils::EMediaTags::eAudioSampleRateString, false }, { NSABUtils::EMediaTags::eAllSubtitleLanguages, false }, { NSABUtils::EMediaTags::eAllSubtitleCodecs, false },
                };

                if ( !settings.contains( "EnabledTags" ) )
                    return retVal;

                auto enabledTags = settings.value( "EnabledTags" ).toList();
                for ( auto &&jj : retVal )
                    jj.second = false;

                for ( auto &&ii : enabledTags )
                {
                    for ( auto &&jj : retVal )
                    {
                        if ( jj.first == static_cast< NSABUtils::EMediaTags >( ii.toInt() ) )
                            jj.second = true;
                    }
                }
                return retVal;
            }

            std::list< NSABUtils::EMediaTags > CPreferences::getEnabledTags() const
            {
                auto allTags = getAllMediaTags();
                std::list< NSABUtils::EMediaTags > retVal;
                for ( auto &&ii : allTags )
                {
                    if ( ii.second )
                        retVal.emplace_back( ii.first );
                }
                return retVal;
            }

            QStringList CPreferences::getEnabledTagsForDisplay() const
            {
                auto tags = getEnabledTags();
                QStringList retVal;
                for ( auto &&ii : tags )
                    retVal << NSABUtils::displayName( ii );
                return retVal;
            }

            void CPreferences::setEnabledTags( const std::list< NSABUtils::EMediaTags > &values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                QVariantList tmp;
                for ( auto &&ii : values )
                    tmp << static_cast< int >( ii );
                settings.setValue( "EnabledTags", tmp );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QString replaceFileInfo( const QFileInfo &fi, const QDate &date, const QString &expr )
            {
                QString retVal = expr;

                retVal = retVal.replace( "<EMPTY>", R"(^$)" );
                retVal = retVal.replace( "<filename>", QRegularExpression::escape( fi.fileName() ) );
                retVal = retVal.replace( "<basename>", QRegularExpression::escape( fi.completeBaseName() ) );
                retVal = retVal.replace( "<extension>", QRegularExpression::escape( fi.suffix() ) );

                if ( date.isValid() )
                {
                    retVal = retVal.replace( "<year>", date.toString( "(yyyy)" ) );
                    retVal = retVal.replace( "<month>", date.toString( "(M|MM|MMM|MMMM)" ) );
                    retVal = retVal.replace( "<day>", date.toString( "(d|dd|ddd|dddd)" ) );

                    auto dateFormat = "(" + NSABUtils::getDateFormats( { true, false } ).join( "|" ) + ")";
                    retVal = retVal.replace( "<date>", date.toString( dateFormat ) );
                }
                return retVal;
            }

            bool CPreferences::getVerifyMediaTitle() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaTitle", true ).toBool();
            }

            void CPreferences::setVerifyMediaTitle( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaTitle", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QRegularExpression CPreferences::getVerifyMediaTitleExpr( const QFileInfo &fi, const QDate &date ) const
            {
                auto regExStr = replaceFileInfo( fi, date, getVerifyMediaTitleExpr() );
                return QRegularExpression( regExStr );
            }

            QString CPreferences::getVerifyMediaTitleExpr() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaTitleExpr", "<basename>" ).toString();
            }

            void CPreferences::setVerifyMediaTitleExpr( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaTitleExpr", value );
            }

            bool CPreferences::getVerifyMediaDate() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaDate", true ).toBool();
            }

            void CPreferences::setVerifyMediaDate( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaDate", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QString CPreferences::getVerifyMediaDateExpr() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaDateExpr", R"(<year>|<month>[-\/]<year>|<month>[-\/]<day>[-\/]<year>)" ).toString();
            }

            void CPreferences::setVerifyMediaDateExpr( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaDateExpr", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QRegularExpression CPreferences::getVerifyMediaDateExpr( const QFileInfo &fi, const QDate &date ) const
            {
                auto regExStr = replaceFileInfo( fi, date, getVerifyMediaDateExpr() );
                return QRegularExpression( regExStr );
            }

            bool CPreferences::getVerifyMediaComment() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaComment", true ).toBool();
            }

            void CPreferences::setVerifyMediaComment( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaComment", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QString CPreferences::getVerifyMediaCommentExpr() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                return settings.value( "VerifyMediaCommentExpr", R"(<EMPTY>)" ).toString();
            }

            void CPreferences::setVerifyMediaCommentExpr( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTagPrefs ) );
                settings.setValue( "VerifyMediaCommentExpr", value );
                emitSigPreferencesChanged( EPreferenceType::eTagPrefs );
            }

            QRegularExpression CPreferences::getVerifyMediaCommentExpr( const QFileInfo &fi, const QDate &date ) const
            {
                auto regExStr = replaceFileInfo( fi, date, getVerifyMediaCommentExpr() );
                return QRegularExpression( regExStr );
            }

            /// ////////////////////////////////////////////////////////
            /// External Tools Options
            /// ////////////////////////////////////////////////////////

            QString CPreferences::getExternalToolPath( const QString &settingsLoc, const QString &exe, const QString &defaultPath ) const
            {
                QDir defDir( qApp->applicationDirPath() );
                if ( getUseCustomExternalTools() )
                    defDir = QDir( defaultPath );

                auto retVal = defDir.absoluteFilePath( exe );

                if ( !settingsLoc.isEmpty() )
                {
                    QSettings settings;
                    settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                    retVal = settings.value( settingsLoc, retVal ).toString();
                }

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
            }

            void CPreferences::setUseCustomExternalTools( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "UseCustomExternalTools", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            bool CPreferences::getUseCustomExternalTools() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                return settings.value( "UseCustomExternalTools", false ).toBool();
            }

            QString CPreferences::getMKVValidatorEXE() const
            {
                return getExternalToolPath( "", "mkvalidator.exe", qApp->applicationDirPath() );
            }

            void CPreferences::setMKVPropEditEXE( const QString &value )
            {
                if ( value == getMKVPropEditEXE() )
                    return;

                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "MKVPropEditEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getMKVPropEditEXE() const
            {
                return getExternalToolPath( "MKVPropEditEXE", "mkvpropedit.exe", "C:/Program Files/MKVToolNix" );
            }

            void CPreferences::setFFMpegEXE( const QString &value )
            {
                if ( value == getFFMpegEXE() )
                    return;

                fMediaFormats->setFFMpegExecutable( value );

                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "FFMpegEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getFFMpegEXE() const
            {
                return getExternalToolPath( "FFMpegEXE", "ffmpeg.exe", QString() );
            }

            void CPreferences::setFFMpegEmbyEXE( const QString &value )
            {
                if ( value == getFFMpegEmbyEXE() )
                    return;

                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "FFMpegEmbyEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getFFMpegEmbyEXE() const
            {
                return getExternalToolPath( "FFMpegEmbyEXE", "ffmpeg-emby.exe", QString() );
            }

            void CPreferences::setFFProbeEXE( const QString &value )
            {
                if ( value == getFFProbeEXE() )
                    return;

                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "FFProbeEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );

                NSABUtils::CMediaInfo::setFFProbeEXE( value );
            }

            QString CPreferences::getFFProbeEXE() const
            {
                return getExternalToolPath( "FFProbeEXE", "ffprobe.exe", QString() );
            }

            /// ////////////////////////////////////////////////////////
            /// BIF Options
            /// ////////////////////////////////////////////////////////

            void CPreferences::setBIFPlayerSpeedMultiplier( int interval )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "PlayerSpeedMultiplier", interval );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::bifPlayerSpeedMultiplier() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "PlayerSpeedMultiplier", 200 ).toInt();
            }

            void CPreferences::setBIFNumFramesToSkip( int interval )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "NumFramesToSkip", interval );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::bifNumFramesToSkip() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "NumFramesToSkip", 5 ).toInt();
            }

            void CPreferences::setBIFLoopCount( int loopCount )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "LoopCount", loopCount );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::bifLoopCount() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                auto retVal = settings.value( "LoopCount", -1 ).toInt();
                if ( retVal == 0 )
                    retVal = -1;
                return retVal;
            }

            int CPreferences::imageInterval() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "ImageInterval", 10 ).toInt();
            }

            void CPreferences::setImageInterval( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "ImageInterval", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            bool CPreferences::imageOriginalSize() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "ImageOriginalSize", false ).toBool();
            }

            void CPreferences::setImageOriginalSize( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "ImageOriginalSize", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            bool CPreferences::byImageWidth() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "ByImageWidth", true ).toBool();
            }

            void CPreferences::setByImageWidth( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "ByImageWidth", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::imageWidth() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "ImageWidth", 320 ).toInt();
            }

            void CPreferences::setImageWidth( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "ImageWidth", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            bool CPreferences::byImageHeight() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "ByImageHeight", false ).toBool();
            }

            void CPreferences::setByImageHeight( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "ByImageHeight", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            int CPreferences::imageHeight() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "ImageHeight", 133 ).toInt();
            }

            void CPreferences::setImageHeight( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "ImageHeight", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            bool CPreferences::generateGIF() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "GenerateGIF", true ).toBool();
            }

            void CPreferences::setGenerateGIF( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "GenerateGIF", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            bool CPreferences::generateBIF() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "GenerateBIF", true ).toBool();
            }

            void CPreferences::setGenerateBIF( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "GenerateBIF", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            bool CPreferences::keepTempDir() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                return settings.value( "KeepTempDir", false ).toBool();
            }

            void CPreferences::setKeepTempDir( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eBIFPrefs ) );
                settings.setValue( "KeepTempDir", value );
                emitSigPreferencesChanged( EPreferenceType::eBIFPrefs );
            }

            /// ////////////////////////////////////////////////////////
            /// GIF Options
            /// ////////////////////////////////////////////////////////

            void CPreferences::setGIFFlipImage( bool flipImage )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "FlipImage", flipImage );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            bool CPreferences::gifFlipImage() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "FlipImage", false ).toBool();
            }

            void CPreferences::setGIFDitherImage( bool ditherImage )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "DitherImage", ditherImage );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            bool CPreferences::gifDitherImage() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "DitherImage", true ).toBool();
            }

            void CPreferences::setGIFLoopCount( int loopCount )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "LoopCount", loopCount );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifLoopCount() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "LoopCount", -1 ).toInt();
            }

            void CPreferences::setGIFStartFrame( int startFrame )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "StartFrame", startFrame );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifStartFrame() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "StartFrame", 0 ).toInt();
            }

            void CPreferences::setGIFEndFrame( int endFrame )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "EndFrame", endFrame );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifEndFrame() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "EndFrame", true ).toInt();
            }

            void CPreferences::setGIFDelay( int delay )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                settings.setValue( "Delay", delay );
                emitSigPreferencesChanged( EPreferenceType::eGIFPrefs );
            }

            int CPreferences::gifDelay() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eGIFPrefs ) );
                return settings.value( "Delay", 10 ).toInt();
            }

            /// ////////////////////////////////////////////////////////
            /// MakeMKV Options
            /// ////////////////////////////////////////////////////////
            std::shared_ptr< NSABUtils::CMediaInfo > CPreferences::getMediaInfo( const QString &fileName, bool force )
            {
                return getMediaInfo( std::move( QFileInfo( fileName ) ), force );
            }

            std::shared_ptr< NSABUtils::CMediaInfo > CPreferences::getMediaInfo( const QFileInfo &fi, bool force )
            {
                if ( !isMediaFile( fi ) )
                    return {};

                if ( NSABUtils::CMediaInfoMgr::instance()->isMediaCached( fi ) )
                    return NSABUtils::CMediaInfoMgr::instance()->getMediaInfo( fi );

                if ( !force && !getLoadMediaInfo() )
                    return {};

                bool delayLoad = !force && getBackgroundLoadMediaInfo();
                if ( delayLoad )
                {
                    return NSABUtils::CMediaInfoMgr::instance()->getMediaInfo( fi );
                }
                else
                    return std::make_shared< NSABUtils::CMediaInfo >( fi );
            }

            QStringList CPreferences::availableEncoderMediaFormats( bool verbose ) const
            {
                return getMediaFormats()->encoderFormats( verbose );
            }

            NSABUtils::TFormatMap CPreferences::getEncoderFormatExtensionsMap() const
            {
                return getMediaFormats()->mediaEncoderFormatExtensions();
            }

            QStringList CPreferences::availableDecoderMediaFormats( bool verbose ) const
            {
                return getMediaFormats()->decoderFormats( verbose );
            }

            NSABUtils::TFormatMap CPreferences::getDecoderFormatExtensionsMap() const
            {
                return getMediaFormats()->mediaDecoderFormatExtensions();
            }

            QStringList CPreferences::availableVideoDecodingCodecs( bool verbose ) const
            {
                return getMediaFormats()->videoDecodingCodecs( verbose );
            }

            QStringList CPreferences::availableVideoEncodingCodecs( bool verbose ) const
            {
                return getMediaFormats()->videoEncodingCodecs( verbose );
            }

            QStringList CPreferences::availableAudioEncodingCodecs( bool verbose ) const
            {
                return getMediaFormats()->audioEncodingCodecs( verbose );
            }

            QStringList CPreferences::availableAudioDecodingCodecs( bool verbose ) const
            {
                return getMediaFormats()->audioDecodingCodecs( verbose );
            }

            QStringList CPreferences::availableSubtitleEncodingCodecs( bool verbose ) const
            {
                return getMediaFormats()->subtitleEncodingCodecs( verbose );
            }

            QStringList CPreferences::availableSubtitleDecodingCodecs( bool verbose ) const
            {
                return getMediaFormats()->subtitleDecodingCodecs( verbose );
            }

            QStringList CPreferences::availableVideoDecoders( bool verbose ) const
            {
                return getMediaFormats()->videoDecoders( verbose );
            }

            QStringList CPreferences::availableVideoEncoders( bool verbose ) const
            {
                return getMediaFormats()->videoEncoders( verbose );
            }

            QStringList CPreferences::availableAudioEncoders( bool verbose ) const
            {
                return getMediaFormats()->audioEncoders( verbose );
            }

            QStringList CPreferences::availableAudioDecoders( bool verbose ) const
            {
                return getMediaFormats()->audioDecoders( verbose );
            }

            QStringList CPreferences::availableSubtitleEncoders( bool verbose ) const
            {
                return getMediaFormats()->subtitleEncoders( verbose );
            }

            QStringList CPreferences::availableSubtitleDecoders( bool verbose ) const
            {
                return getMediaFormats()->subtitleDecoders( verbose );
            }

            NSABUtils::TCodecToEncoderDecoderMap CPreferences::getCodecToEncoderMap() const
            {
                return getMediaFormats()->codecToEncoderMap();
            }

            NSABUtils::TCodecToEncoderDecoderMap CPreferences::getCodecToDecoderMap() const
            {
                return getMediaFormats()->codecToDecoderMap();
            }

            QStringList CPreferences::availableHWAccels( bool verbose ) const
            {
                return getMediaFormats()->hwAccels( verbose );
            }

            void CPreferences::recomputeSupportedFormats( QProgressDialog *dlg )
            {
                loadMediaFormats( true, dlg );
            }

            void CPreferences::loadMediaFormats( bool forceFromFFMpeg, QProgressDialog *dlg ) const
            {
                auto ffmpeg = getFFMpegEXE();
                if ( ffmpeg.isEmpty() )
                    return;

                if ( forceFromFFMpeg )
                {
                    fMediaFormats->setFFMpegExecutable( ffmpeg );
                    fMediaFormats->recompute( dlg );
                }
                else if ( !fMediaFormats->loaded() )
                {
                    fMediaFormats = std::move( std::make_unique< NSABUtils::CFFMpegFormats >() );
                    fMediaFormats->setFFMpegExecutable( ffmpeg );

                    fMediaFormats->initEncoderFormatsFromDefaults( availableMediaEncoderFormatsStatic( false ), availableMediaEncoderFormatsStatic( true ), getEncoderFormatExtensionsMapStatic() );
                    fMediaFormats->initDecoderFormatsFromDefaults( availableMediaDecoderFormatsStatic( false ), availableMediaDecoderFormatsStatic( true ), getDecoderFormatExtensionsMapStatic() );

                    fMediaFormats->initVideoEncodingCodecsFromDefaults( availableVideoEncodingCodecsStatic( false ), availableVideoEncodingCodecsStatic( true ) );
                    fMediaFormats->initVideoDecodingCodecsFromDefaults( availableVideoDecodingCodecsStatic( false ), availableVideoDecodingCodecsStatic( true ) );

                    fMediaFormats->initAudioEncodingCodecsFromDefaults( availableAudioEncodingCodecsStatic( false ), availableAudioEncodingCodecsStatic( true ) );
                    fMediaFormats->initAudioDecodingCodecsFromDefaults( availableAudioDecodingCodecsStatic( false ), availableAudioDecodingCodecsStatic( true ) );

                    fMediaFormats->initSubtitleEncodingCodecsFromDefaults( availableSubtitleEncodingCodecsStatic( false ), availableSubtitleEncodingCodecsStatic( true ) );
                    fMediaFormats->initSubtitleDecodingCodecsFromDefaults( availableSubtitleDecodingCodecsStatic( false ), availableSubtitleDecodingCodecsStatic( true ) );

                    fMediaFormats->initVideoEncodersFromDefaults( availableVideoEncodersStatic( false ), availableVideoEncodersStatic( true ) );
                    fMediaFormats->initVideoDecodersFromDefaults( availableVideoDecodersStatic( false ), availableVideoDecodersStatic( true ) );

                    fMediaFormats->initAudioEncodersFromDefaults( availableAudioEncodersStatic( false ), availableAudioEncodersStatic( true ) );
                    fMediaFormats->initAudioDecodersFromDefaults( availableAudioDecodersStatic( false ), availableAudioDecodersStatic( true ) );

                    fMediaFormats->initSubtitleEncodersFromDefaults( availableSubtitleEncodersStatic( false ), availableSubtitleEncodersStatic( true ) );
                    fMediaFormats->initSubtitleDecodersFromDefaults( availableSubtitleDecodersStatic( false ), availableSubtitleDecodersStatic( true ) );

                    fMediaFormats->initCodecToEncoderMapDefaults( getCodecToEncoderMapStatic() );
                    fMediaFormats->initCodecToDecoderMapDefaults( getCodecToDecoderMapStatic() );

                    fMediaFormats->initHWAccelsFromDefaults( availableHWAccelsStatic( false ), availableHWAccelsStatic( true ) );
                }
            }

            NSABUtils::CFFMpegFormats *CPreferences::getMediaFormats() const
            {
                loadMediaFormats( false );
                return fMediaFormats.get();
            }

            QStringList CPreferences::getEncoderExtensionsForFormat( const QString &formatName, const QStringList &exclude ) const
            {
                return getMediaFormats()->getEncoderExtensionsForFormat( formatName, exclude );
            }

            QStringList CPreferences::getDecoderExtensionsForFormat( const QString &formatName, const QStringList &exclude ) const
            {
                return getMediaFormats()->getDecoderExtensionsForFormat( formatName, exclude );
            }

            void CPreferences::setForceTranscode( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "ForceTranscode", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getForceTranscode() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "ForceTranscode", false ).toBool();
            }

            void CPreferences::setConvertMediaContainer( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "ConvertMediaFormat", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getConvertMediaContainer() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "ConvertMediaFormat", getConvertMediaToContainerDefault() ).toBool();
            }

            void CPreferences::setConvertMediaToContainer( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "ConvertMediaToContainer", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            QString CPreferences::getConvertMediaToContainer() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "ConvertMediaToContainer", getConvertMediaToContainerDefault() ).toString();
            }

            QString CPreferences::getMediaContainerExt() const
            {
                return getMediaFormats()->getPrimaryEncoderExtensionForFormat( getConvertMediaToContainer() );
            }

            void CPreferences::setTranscodeAudio( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "TranscodeAudio", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getTranscodeAudio() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "TranscodeAudio", getTranscodeAudioDefault() ).toBool();
            }

            void CPreferences::setAddAACAudioCodec( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "AddAACAudioCodec", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getAddAACAudioCodec() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "AddAACAudioCodec", getTranscodeAudioDefault() ).toBool();
            }

            void CPreferences::setOnlyTranscodeVideoOnFormatChange( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "OnlyTranscodeVideoOnFormatChange", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getOnlyTranscodeVideoOnFormatChange() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "OnlyTranscodeVideoOnFormatChange", getOnlyTranscodeVideoOnFormatChangeDefault() ).toBool();
            }

            void CPreferences::setTranscodeToAudioCodec( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "AudioCodec", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            QString CPreferences::getTranscodeToAudioCodec() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "AudioCodec", getTranscodeToAudioCodecDefault() ).toString();
            }

            void CPreferences::setTranscodeVideo( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "TranscodeVideo", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getTranscodeVideo() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "TranscodeVideo", getTranscodeVideoDefault() ).toBool();
            }

            void CPreferences::setTranscodeToVideoCodec( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "VideoCodec", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            QString CPreferences::getTranscodeToVideoCodec() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "VideoCodec", getTranscodeToVideoCodecDefault() ).toString();
            }

            QString CPreferences::getTranscodeHWAccel() const
            {
                auto videoCodec = getTranscodeToVideoCodec();
                return getTranscodeHWAccel( videoCodec );
            }

            QString CPreferences::getTranscodeHWAccel( const QString &codec ) const
            {
                return getMediaFormats()->getTranscodeHWAccel( codec );
            }

            QString CPreferences::getCodecForHWAccel( const QString &codec ) const
            {
                return getMediaFormats()->getCodecForHWAccel( codec );
            }

            void CPreferences::setLosslessEncoding( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.setValue( "LosslessEncoding", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getLosslessEncoding() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "LosslessEncoding", true ).toBool();
            }

            void CPreferences::setUseCRF( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "UseCRF", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getUseCRF() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "UseCRF", getUseCRFDefault() ).toBool();
            }

            void CPreferences::setCRF( double value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "CRF", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            double CPreferences::getCRF() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "CRF", getCRFDefault() ).toDouble();
            }

            void CPreferences::setGenerateLowBitrateVideo( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "GenerateLowBitrateVideo", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getGenerateLowBitrateVideo() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "GenerateLowBitrateVideo", getGenerateLowBitrateVideoDefault() ).toBool();
            }

            void CPreferences::setBitrateThresholdPercentage( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "BitrateThreshold", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            int CPreferences::getBitrateThresholdPercentage() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "BitrateThresholdPercentage", getBitrateThresholdPercentageDefault() ).toInt();
            }

            void CPreferences::setGenerateNon4kVideo( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "GenerateNon4kVideo", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getGenerateNon4kVideo() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "GenerateNon4kVideo", getGenerateNon4kVideoDefault() ).toBool();
            }

            void CPreferences::setUseAverageBitrate( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "UseAverageBitrate", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getUseAverageBitrate() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "UseAverageBitrate", getUseAverageBitrateDefault() ).toBool();
            }

            uint64_t CPreferences::getAverageBitrateTarget( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, bool useKBS, bool addThreshold ) const
            {
                if ( !mediaInfo )
                    return 0;

                std::optional< uint64_t > retVal;
                if ( mediaInfo->is4kResolution() )
                    retVal = getAverage4kBitrate();
                else if ( mediaInfo->isHDResolution() )
                    retVal = getAverageHDBitrate();
                else if ( mediaInfo->isSubHDResolution() )
                    retVal = getAverageSubHDBitrate();

                if ( !retVal.has_value() )
                {
                    auto bitRate = mediaInfo->getUncompressedBitRate();
                    auto tmp = 1.0 * bitRate / ( 1.0 * getNonConformingResolutionDivisor() );
                    if ( useKBS )
                        tmp /= 1000.0;

                    retVal = static_cast< uint64_t >( tmp );
                }
                else
                {
                    if ( !useKBS )
                        retVal.value() *= 1000;
                }

                if ( addThreshold )
                    retVal.value() *= 1.0 + ( ( 1.0 * getBitrateThresholdPercentage() ) / 100.0 );
                return retVal.value();
            }

            QString CPreferences::getAverageBitrateTargetDisplayString( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo ) const
            {
                auto bps = NPreferences::NCore::CPreferences::instance()->getAverageBitrateTarget( mediaInfo, false, false );
                return NSABUtils::NFileUtils::byteSizeString( bps, true, false, 1, false, "bps" );
            }

            void CPreferences::setAverage4kBitrate( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "Average4kBitrate", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            int CPreferences::getAverage4kBitrate() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "Average4kBitrate", getAverage4kBitrateDefault() ).toInt();
            }

            void CPreferences::setAverageHDBitrate( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "AverageHDBitrate", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            int CPreferences::getAverageHDBitrate() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "AverageHDBitrate", getAverageHDBitrateDefault() ).toInt();
            }

            void CPreferences::setAverageSubHDBitrate( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "AverageSubHDBitrate", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            int CPreferences::getAverageSubHDBitrate() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "AverageSubHDBitrate", getAverageSubHDBitrateDefault() ).toInt();
            }

            void CPreferences::setNonConformingResolutionDivisor( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "NonConformingResolutionRatio", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            int CPreferences::getNonConformingResolutionDivisor() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "NonConformingResolutionRatio", getNonConformingResolutionDivisorDefault() ).toInt();
            }

            void CPreferences::setUsePreset( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "UsePreset", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getUsePreset() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "UsePreset", getUsePresetDefault() ).toBool();
            }

            void CPreferences::setPreset( ETranscodePreset value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "Preset", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            ETranscodePreset CPreferences::getPreset() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return static_cast< ETranscodePreset >( settings.value( "Preset", getPresetDefault() ).toInt() );
            }

            void CPreferences::setUseTune( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "UseTune", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getUseTune() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "UseTune", getUseTuneDefault() ).toBool();
            }

            void CPreferences::setTune( ETranscodeTune value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "Tune", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            ETranscodeTune CPreferences::getTune() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return static_cast< ETranscodeTune >( settings.value( "Tune", getTuneDefault() ).toInt() );
            }

            void CPreferences::setUseProfile( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "UseProfile", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getUseProfile() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "UseProfile", getUseProfileDefault() ).toBool();
            }

            void CPreferences::setProfile( ETranscodeProfile value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "Profile", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            ETranscodeProfile CPreferences::getProfile() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return static_cast< ETranscodeProfile >( settings.value( "Profile", getProfileDefault() ).toInt() );
            }

            bool CPreferences::hasIntelGPU() const
            {
                if ( !fHasIntelGPU.has_value() )
                {
                    fHasIntelGPU = false;
                    auto gpus = NSABUtils::detectGPUs();
                    for ( auto &&ii : gpus )
                    {
                        if ( ii->isIntelGPU() )
                        {
                            fHasIntelGPU = true;
                            break;
                        }
                    }
                }
                return fHasIntelGPU.value();
            }

            bool CPreferences::hasNVidiaGPU() const
            {
                if ( !fHasNVidiaGPU.has_value() )
                {
                    fHasNVidiaGPU = false;
                    auto gpus = NSABUtils::detectGPUs();
                    for ( auto &&ii : gpus )
                    {
                        if ( ii->isNVidiaGPU() )
                        {
                            fHasNVidiaGPU = true;
                            break;
                        }
                    }
                }
                return fHasNVidiaGPU.value();
            }

            bool CPreferences::hasAMDGPU() const
            {
                if ( !fHasAMDGPU.has_value() )
                {
                    fHasAMDGPU = false;
                    auto gpus = NSABUtils::detectGPUs();
                    for ( auto &&ii : gpus )
                    {
                        if ( ii->isAMDGPU() )
                        {
                            fHasAMDGPU = true;
                            break;
                        }
                    }
                }
                return fHasAMDGPU.value();
            }

            int CPreferences::getGPUCount() const
            {
                int cnt = hasAMDGPU() ? 1 : 0;
                cnt += ( hasIntelGPU() ? 1 : 0 );
                cnt += ( hasNVidiaGPU() ? 1 : 0 );
                return cnt;
            }

            void CPreferences::setOnlyTranscodeAudioOnFormatChange( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                settings.setValue( "OnlyTranscodeAudioOnFormatChange", value );
                emitSigPreferencesChanged( EPreferenceType::eTranscodePrefs );
            }

            bool CPreferences::getOnlyTranscodeAudioOnFormatChange() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTranscodePrefs ) );
                return settings.value( "OnlyTranscodeAudioOnFormatChange", getOnlyTranscodeAudioOnFormatChangeDefault() ).toBool();
            }

            /////////////////////////////////
            /////////////////////////////////
            bool CPreferences::isFileWithExtension( const QFileInfo &fi, std::function< QStringList() > getExtensions, std::unordered_set< QString > &hash, std::unordered_map< QString, bool > &cache ) const
            {
                if ( hash.empty() )
                {
                    auto suffixes = getExtensions();
                    for ( auto &&ii : suffixes )
                    {
                        auto pos = ii.lastIndexOf( '.' );
                        ii = ii.mid( pos + 1 );
                        ii = ii.toLower();
                    }
                    hash = NSABUtils::hashFromList( suffixes );
                }

                auto suffix = fi.suffix().toLower();
                auto pos = cache.find( suffix );
                if ( pos != cache.end() )
                {
                    return ( *pos ).second;
                }
                bool retVal = ( hash.find( suffix ) != hash.end() );
                cache[ suffix ] = retVal;
                return retVal;
            }

            bool CPreferences::isMediaFile( const QFileInfo &fi ) const
            {
                return isFileWithExtension(
                    fi, [ this ]() { return getMediaFormats()->getVideoExtensions(); }, fMediaExtensionsHash, fIsMediaExtension );
            }

            // only return true for X_Lang.srt files or subs directories
            bool CPreferences::isSubtitleFile( const QFileInfo &fi, bool *isLangFileFormat ) const
            {
                if ( isLangFileFormat )
                {
                    *isLangFileFormat = NMediaManager::NCore::SLanguageInfo::isLangFileFormat( fi );
                }

                return isFileWithExtension(
                    fi, [ this ]() { return getMediaFormats()->getSubtitleExtensions(); }, fSubtitleExtensionsHash, fIsSubtitleExtension );
            }

            void CPreferences::emitSigPreferencesChanged( EPreferenceTypes preferenceTypes )
            {
                fPending |= preferenceTypes;
                if ( !fPrefChangeTimer )
                {
                    fPrefChangeTimer = new QTimer( this );
                    fPrefChangeTimer->setSingleShot( true );
                    fPrefChangeTimer->setInterval( 50 );
                    connect(
                        fPrefChangeTimer, &QTimer::timeout,
                        [ this ]()
                        {
                            if ( ( fPending & eSystemPrefs ) != 0 )
                            {
                                fMediaExtensionsHash.clear();
                                fIsMediaExtension.clear();
                                fSubtitleExtensionsHash.clear();
                                fIsSubtitleExtension.clear();
                            }
                            if ( ( fPending & eMediaRenamerPrefs ) != 0 )
                            {
                                fKnownStringRegExsCache.clear();
                            }
                            emit sigPreferencesChanged( fPending );
                            fPending = EPreferenceTypes();
                        } );
                }
                fPrefChangeTimer->stop();
                fPrefChangeTimer->start();
            }
        }
    }
}
