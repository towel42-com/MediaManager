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
        QString getIndent( int indent )
        {
            return QString( 4 * indent, QChar( ' ' ) );
        }

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
                case eTransformPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eTransformPrefs" : "Transform";
                case eTagPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eTagPrefs" : "Tags";
                case eExtToolsPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eExtToolsPrefs" : "ExternalTools";
                case eGIFPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eGIFPrefs" : "GIF";
                case eBIFPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eBIFPrefs" : "BIF";
                case eMakeMKVPrefs:
                    return forEnum ? "NMediaManager::NPreferences::eMakeMKVPrefs" : "MakeMKV";
                default:
                    return "";
            }
        }

        namespace NCore
        {
            QString toString( EMakeMKVPreset preset, bool forEnum )
            {
                switch ( preset )
                {
                    case EMakeMKVPreset::eUltraFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eUltraFast" : "ultrafast";
                    case EMakeMKVPreset::eSuperFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eSuperFast" : "superfast";
                    case EMakeMKVPreset::eVeryFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eVeryFast" : "veryfast";
                    case EMakeMKVPreset::eFaster:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eFaster" : "faster";
                    case EMakeMKVPreset::eFast:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eFast" : "fast";
                    case EMakeMKVPreset::eMedium:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eMedium" : "medium";
                    case EMakeMKVPreset::eSlow:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eSlow" : "slow";
                    case EMakeMKVPreset::eSlower:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eSlower" : "slower";
                    case EMakeMKVPreset::eVerySlow:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVPreset::eVerySlow" : "veryslow";
                }
                return toString( EMakeMKVPreset::eMedium, forEnum );
            }

            QString toString( EMakeMKVTune preset, bool forEnum )
            {
                switch ( preset )
                {
                    case EMakeMKVTune::eFilm:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVTune::eFilm" : "film";
                    case EMakeMKVTune::eAnimation:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVTune::eAnimation" : "animation";
                    case EMakeMKVTune::eGrain:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVTune::eGrain" : "grain";
                    case EMakeMKVTune::eStillImage:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVTune::eStilImage" : "stillimage";
                    case EMakeMKVTune::eFastDecode:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVTune::eFastDecode" : "fastdecode";
                    case EMakeMKVTune::eZeroLatency:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVTune::eZeroLatency" : "zerolatency";
                }
                return toString( EMakeMKVTune::eFilm, forEnum );
            }

            QString toString( EMakeMKVProfile profile, bool forEnum )
            {
                switch ( profile )
                {
                    case EMakeMKVProfile::eMain:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain" : "main";
                    case EMakeMKVProfile::eMainIntra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMainIntra" : "main-intra";
                    case EMakeMKVProfile::eMailStillPicture:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMailStillPicture" : "mailstillpicture";
                    case EMakeMKVProfile::eMain444_8:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain444_8" : "main444-8";
                    case EMakeMKVProfile::eMain444Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain444Intra" : "main444-intra";
                    case EMakeMKVProfile::eMain444StillPicture:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain444StillPicture" : "main444-stillpicture";
                    case EMakeMKVProfile::eMain10:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain10" : "main10";
                    case EMakeMKVProfile::eMain10Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain10Intra" : "main10-intra";
                    case EMakeMKVProfile::eMain422_10:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain422_10" : "main422-10";
                    case EMakeMKVProfile::eMain422_10Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain422_10Intra" : "main422-10-intra";
                    case EMakeMKVProfile::eMain444_10:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain444_10" : "main444-10";
                    case EMakeMKVProfile::eMain444_10Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain444_10Intra" : "main444-10-intra";
                    case EMakeMKVProfile::eMain12:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain12" : "main12";
                    case EMakeMKVProfile::eMain12Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain12Intra" : "main12-intra";
                    case EMakeMKVProfile::eMain422_12:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain422_12" : "main422-12";
                    case EMakeMKVProfile::eMain422_12Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain422_12Intra" : "main422-12-intra";
                    case EMakeMKVProfile::eMain444_12:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain444_12" : "main444-12";
                    case EMakeMKVProfile::eMain444_12Intra:
                        return forEnum ? "NMediaManager::NPreferences::NCore::EMakeMKVProfile::eMain444_12Intra" : "main444012-intra";
                }
                return toString( EMakeMKVProfile::eMain, forEnum );
            }

            CPreferences *CPreferences::instance()
            {
                static CPreferences retVal;
                return &retVal;
            }

            CPreferences::CPreferences()
            {
                fMediaFormats = std::make_unique< NSABUtils::CFFMpegFormats >( getFFMpegEXE() );
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

            QStringList CPreferences::getVideoExtensions() const
            {
                return getMediaFormats()->getVideoExtensions();
            }


            QStringList CPreferences::getSubtitleExtensions() const
            {
                return getMediaFormats()->getSubtitleExtensions();
            }

            /// ////////////////////////////////////////////////////////
            /// transform Options
            /// ////////////////////////////////////////////////////////
            void CPreferences::setTreatAsTVShowByDefault( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "TreatAsTVShowByDefault", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::getTreatAsTVShowByDefault() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "TreatAsTVShowByDefault", false ).toBool();
            }

            void CPreferences::setExactMatchesOnly( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "ExactMatchesOnly", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::getExactMatchesOnly() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "ExactMatchesOnly", false ).toBool();
            }

            void CPreferences::setLoadMediaInfo( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "LoadMediaInfo", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::getLoadMediaInfo() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "LoadMediaInfo", true ).toBool();
            }

            void CPreferences::setOnlyTransformDirectories( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "OnlyLoadDirectories", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::getOnlyTransformDirectories() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "OnlyLoadDirectories", false ).toBool();
            }

            void CPreferences::setTVOutFilePattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "OutFilePattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getTVOutFilePattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );

                return settings.value( "OutFilePattern", getDefaultOutFilePattern( true ) ).toString();
            }

            void CPreferences::setSeasonOutDirPattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "SeasonDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getSeasonOutDirPattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );

                return settings.value( "SeasonDirPattern", getDefaultSeasonDirPattern() ).toString();
            }

            void CPreferences::setTVOutDirPattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );
                settings.setValue( "OutDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getTVOutDirPattern( bool expandSeasonDir ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForTV" );

                auto retVal = settings.value( "OutDirPattern", getDefaultOutDirPattern( true ) ).toString();
                if ( expandSeasonDir )
                    retVal = retVal.replace( "<season_dir>", getSeasonOutDirPattern() );
                return retVal;
            }

            void CPreferences::setMovieOutFilePattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );
                settings.setValue( "OutFilePattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getMovieOutFilePattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );

                return settings.value( "OutFilePattern", getDefaultOutFilePattern( false ) ).toString();
            }

            void CPreferences::setMovieOutDirPattern( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );
                settings.setValue( "OutDirPattern", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getMovieOutDirPattern() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.beginGroup( "ForMovies" );

                return settings.value( "OutDirPattern", getDefaultOutDirPattern( false ) ).toString();
            }

            void CPreferences::setDeleteCustom( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteCustom", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteCustom() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteCustom", true ).toBool();
            }

            void CPreferences::setDeleteEXE( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteEXE", true ).toBool();
            }

            void CPreferences::setDeleteNFO( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteNFO", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteNFO() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteNFO", true ).toBool();
            }

            void CPreferences::setDeleteBAK( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteBAK", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteBAK() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteBAK", true ).toBool();
            }

            void CPreferences::setDeleteImages( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteImages", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteImages() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
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
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "DeleteTXT", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            bool CPreferences::deleteTXT() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "DeleteTXT", true ).toBool();
            }

            void CPreferences::setCustomPathsToDelete( const QStringList &values )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                const QStringList &realValues = values;
                settings.setValue( "CustomToDelete", realValues );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QStringList CPreferences::getCustomPathsToDelete() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "CustomToDelete", getDefaultCustomPathsToDelete() ).toStringList();
            }

            void CPreferences::setRippedWithMakeMKVRegEX( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "RippedWithMKVRegEX", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QString CPreferences::getRippedWithMakeMKVRegEX() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
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
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                QSet< QString > tmp;
                QStringList realValues;
                for ( auto &&ii : value )
                {
                    if ( !tmp.contains( ii ) )
                        realValues << ii;
                }

                settings.setValue( "KnownStrings", realValues );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            struct SCmp
            {
                bool operator()( const QString &lhs, const QString &rhs ) const
                {
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
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
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
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "KnownExtendedStrings", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QStringList CPreferences::getKnownExtendedStrings() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
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
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "KnownAbbreviations", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QVariantMap CPreferences::getKnownAbbreviations() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                return settings.value( "KnownAbbreviations", getDefaultKnownAbbreviations() ).toMap();
            };

            void CPreferences::setKnownHyphenated( const QStringList &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
                settings.setValue( "KnownHyphenated", value );
                emitSigPreferencesChanged( EPreferenceType::eTransformPrefs );
            }

            QStringList CPreferences::getKnownHyphenated() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eTransformPrefs ) );
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

            bool CPreferences::pathMatches( const QFileInfo &fileInfo, const QStringList &values ) const
            {
                auto pathName = fileInfo.fileName();

#ifdef Q_OS_WINDOWS
                pathName = pathName.toLower();
#endif
                return containsValue( pathName, values );
            }

            bool CPreferences::isSkippedPath( bool forMediaNaming, const QFileInfo &fileInfo ) const
            {
                return pathMatches( fileInfo, getSkippedPaths( forMediaNaming ) );
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

                std::list< std::pair< NSABUtils::EMediaTags, bool > > retVal = { { NSABUtils::EMediaTags::eTitle, true },    { NSABUtils::EMediaTags::eLength, true },       { NSABUtils::EMediaTags::eDate, true },
                                                                                 { NSABUtils::EMediaTags::eComment, true },  { NSABUtils::EMediaTags::eBPM, true },          { NSABUtils::EMediaTags::eArtist, true },
                                                                                 { NSABUtils::EMediaTags::eComposer, true }, { NSABUtils::EMediaTags::eGenre, true },        { NSABUtils::EMediaTags::eTrack, true },
                                                                                 { NSABUtils::EMediaTags::eAlbum, false },   { NSABUtils::EMediaTags::eAlbumArtist, false }, { NSABUtils::EMediaTags::eDiscnumber, false } };

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

            void CPreferences::setMKVMergeEXE( const QString &value )
            {
                if ( value == getMKVMergeEXE() )
                    return;

                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "MKVMergeEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getMKVMergeEXE() const
            {
                return getExternalToolPath( "MKVMergeEXE", "mkvmerge.exe", "C:/Program Files/MKVToolNix" );
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

            void CPreferences::setFFProbeEXE( const QString &value )
            {
                if ( value == getFFProbeEXE() )
                    return;

                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "FFProbeEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
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
            STranscodeNeeded::STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo )
            {
                fFormat = NPreferences::NCore::CPreferences::instance()->getForceMediaContainer() && mediaInfo && !mediaInfo->isFormat( NPreferences::NCore::CPreferences::instance()->getForceMediaContainerName() );

                fVideo = NPreferences::NCore::CPreferences::instance()->getTranscodeToH265() && mediaInfo && !mediaInfo->isHEVCVideo();
                fAudio = NPreferences::NCore::CPreferences::instance()->getTranscodeAudio() && mediaInfo
                         && !mediaInfo->isAudioCodec( QStringList() << NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() << NPreferences::NCore::CPreferences::instance()->getAllowedAudioCodecs() );

                if ( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeVideoOnFormatChange() )
                {
                    fVideo &= fFormat;
                }

                if ( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeAudioOnFormatChange() )
                {
                    fAudio &= fFormat;
                }
            }

            QStringList CPreferences::getTranscodeArgs( const QString &srcName, const QString &destName ) const
            {
                auto mediaInfo = std::make_shared< NSABUtils::CMediaInfo >( srcName );
                return getTranscodeArgs( mediaInfo, srcName, destName );
            }

            QStringList CPreferences::getTranscodeArgs( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &srcName, const QString &destName ) const
            {
                auto transcodeNeeded = STranscodeNeeded( mediaInfo );

                if ( !transcodeNeeded.transcodeNeeded() )
                    return {};

                auto retVal = QStringList()   //
                              << "-y"   //
                              << "-fflags"
                              << "+genpts"   //
                    ;

                if ( transcodeNeeded.formatOnly() )
                {
                    // already HVEC but wrong container, just copy
                    retVal << "-i" << srcName   //
                        << "-c:v"
                        << "copy"   //
                        << "-c:a"
                        << "copy"   //
                        << "-c:s"
                        << "copy"   //
                        ;
                } 
                else
                {
                    QString hwAccel;
                    QString videoCodec;
                    if ( !transcodeNeeded.fVideo )
                    {
                        hwAccel.clear();
                        videoCodec = "copy";
                    }
                    else if ( NPreferences::NCore::CPreferences::instance()->getNVidiaGPUTranscode() )
                    {
                        hwAccel = "cuda";
                        videoCodec = "hevc_nvenc";
                    }
                    else if ( NPreferences::NCore::CPreferences::instance()->getIntelGPUTranscode() )
                    {
                        hwAccel = "qsv";
                        videoCodec = "hevc_qsv";
                    }
                    else if ( NPreferences::NCore::CPreferences::instance()->getSoftwareTranscode() )
                    {
                        hwAccel.clear();
                        videoCodec = "libx265";
                    }

                    if ( transcodeNeeded.fVideo && !hwAccel.isEmpty() )
                    {
                        retVal   //
                            << "-hwaccel" << hwAccel   //
                            << "-hwaccel_output_format" << hwAccel   //
                            ;
                    }

                    auto audioCodec = transcodeNeeded.fAudio ? getTranscodeToAudioCodec() : "copy";

                    retVal << "-i" << srcName   //
                           << "-map"
                           << "0:a?"   //
                           << "-map"
                           << "0:v?"   //
                           << "-map"
                           << "0:s?"   //
                           << "-c:a" << audioCodec   //
                           << "-c:v" << videoCodec   //
                        ;

                    if ( transcodeNeeded.fVideo )
                    {
                        if ( getLosslessTranscoding() )
                            retVal << "-x265-params"
                                   << "lossless=1";
                        else
                        {
                            if ( getUseExplicitCRF() )
                                retVal << "-crf" << QString::number( getExplicitCRF() );
                            if ( getUsePreset() )
                                retVal << "-preset" << toString( getPreset() );
                            if ( getUseTune() )
                                retVal << "-tune" << toString( getTune() );
                        }
                        if ( getUseProfile() )
                            retVal << "-profile:v" << toString( getProfile() );
                    }
                }
                
                retVal << "-f" << getForceMediaContainerName()   //
                       << destName;

                return retVal;
            }

            QStringList CPreferences::availableAudioEncoders( bool verbose ) const
            {
                return getMediaFormats()->audioCodecs( verbose );
            }

            QStringList CPreferences::availableVideoEncoders( bool verbose ) const
            {
                return getMediaFormats()->videoCodecs( verbose );
            }

            QStringList CPreferences::availableSubtitleEncoders( bool verbose ) const
            {
                return getMediaFormats()->subtitleCodecs( verbose );
            }

            QStringList CPreferences::availableMediaFormats( bool verbose ) const
            {
                return getMediaFormats()->formats( verbose );
            }

            NSABUtils::TFormatMap CPreferences::getFormatExtensionsMap() const
            {
                return getMediaFormats()->mediaFormatExtensions();
            }

            void CPreferences::recomputeSupportedFormats( QProgressDialog *dlg )
            {
                loadMediaFormats( true, dlg );
            }

            void CPreferences::loadMediaFormats( bool forceFromFFMpeg, QProgressDialog * dlg ) const
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
                    fMediaFormats->initFormatsFromDefaults( availableMediaFormatsDefault( false ), availableMediaFormatsDefault( true ), getVideoExtensionsMapDefault() );
                    fMediaFormats->initVideoCodecsFromDefaults( availableVideoEncodersDefault( false ), availableVideoEncodersDefault( true ) );
                    fMediaFormats->initAudioCodecsFromDefaults( availableAudioEncodersDefault( false ), availableAudioEncodersDefault( true ) );
                    fMediaFormats->initSubtitleCodecsFromDefaults( availableSubtitleEncodersDefault( false ), availableSubtitleEncodersDefault( true ) );
                }
            }

            NSABUtils::CFFMpegFormats * CPreferences::getMediaFormats() const
            {
                loadMediaFormats( false );
                return fMediaFormats.get();
            }

            QStringList CPreferences::getExtensionsForMediaFormat( const QString &formatName ) const
            {
                return getMediaFormats()->getExtensionsForFormat( formatName );
            }

            void CPreferences::setForceMediaContainer( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "ForceMediaFormat", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getForceMediaContainer() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "ForceMediaFormat", getForceMediaContainerDefault() ).toBool();
            }

            void CPreferences::setForceMediaContainerName( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "ForceMediaFormatName", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            QString CPreferences::getForceMediaContainerName() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "ForceMediaFormatName", getForceMediaContainerNameDefault() ).toString();
            }

            QString CPreferences::getForceMediaContainerExt() const
            {
                return getMediaFormats()->getPrimaryExtensionForFormat( getForceMediaContainerName() );
            }

            void CPreferences::setTranscodeAudio( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "TranscodeAudio", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getTranscodeAudio() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "TranscodeAudio", getTranscodeAudioDefault() ).toBool();
            }

            void CPreferences::setOnlyTranscodeVideoOnFormatChange( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "OnlyTranscodeVideoOnFormatChange", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getOnlyTranscodeVideoOnFormatChange() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "OnlyTranscodeVideoOnFormatChange", getOnlyTranscodeVideoOnFormatChangeDefault() ).toBool();
            }

            void CPreferences::setTranscodeToAudioCodec( const QString& value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "AudioCodec", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            QString CPreferences::getTranscodeToAudioCodec() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "AudioCodec", getTranscodeToAudioCodecDefault() ).toString();
            }

            void CPreferences::setAllowedAudioCodecs( const QStringList &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "AllowedAudioCodecs", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            QStringList CPreferences::getAllowedAudioCodecs() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "AllowedAudioCodecs", getAllowedAudioCodecsDefault() ).toStringList();
            }

            void CPreferences::setTranscodeToH265( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "ConvertToH265", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getTranscodeToH265() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "ConvertToH265", getTranscodeToH265Default() ).toBool();
            }

            void CPreferences::setLosslessTranscoding( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "LosslessTranscoding", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getLosslessTranscoding() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "LosslessTranscoding", getLosslessTranscodingDefault() ).toBool();
            }

            void CPreferences::setUseCRF( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "UseCRF", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getUseCRF() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "UseCRF", getUseCRFDefault() ).toBool();
            }

            void CPreferences::setUseExplicitCRF( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "UseConstantRateFactor", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getUseExplicitCRF() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "UseConstantRateFactor", getUseExplicitCRFDefault() ).toBool();
            }

            void CPreferences::setExplicitCRF( int value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "ExplicitCRF", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            int CPreferences::getExplicitCRF() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "ExplicitCRF", getExplicitCRFDefault() ).toInt();
            }

            void CPreferences::setUsePreset( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "UsePreset", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getUsePreset() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "UsePreset", getUsePresetDefault() ).toBool();
            }

            void CPreferences::setPreset( EMakeMKVPreset value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "Preset", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            EMakeMKVPreset CPreferences::getPreset() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return static_cast< EMakeMKVPreset >( settings.value( "Preset", getPresetDefault() ).toInt() );
            }

            void CPreferences::setUseTune( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "UseTune", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getUseTune() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "UseTune", getUseTuneDefault() ).toBool();
            }

            void CPreferences::setTune( EMakeMKVTune value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "Tune", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            EMakeMKVTune CPreferences::getTune() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return static_cast< EMakeMKVTune >( settings.value( "Tune", getTuneDefault() ).toInt() );
            }

            void CPreferences::setUseProfile( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "UseProfile", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getUseProfile() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "UseProfile", getUseProfileDefault() ).toBool();
            }

            void CPreferences::setProfile( EMakeMKVProfile value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "Profile", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            EMakeMKVProfile CPreferences::getProfile() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return static_cast< EMakeMKVProfile >( settings.value( "Profile", getProfileDefault() ).toInt() );
            }

            bool CPreferences::hasIntelGPU() const
            {
                if ( !fHasIntelGPU.has_value() )
                {
                    fHasIntelGPU = false;
                    auto gpus = NSABUtils::detectGPUs();
                    for ( auto&& ii : gpus )
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
                    for ( auto&& ii : gpus )
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

            void CPreferences::setIntelGPUTranscode( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "IntelGPUTranscode", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getIntelGPUTranscodeDefault() const
            {
                return hasIntelGPU() && !hasNVidiaGPU();
            }

            bool CPreferences::getIntelGPUTranscode() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "IntelGPUTranscode", getIntelGPUTranscodeDefault() ).toBool();
            }

            void CPreferences::setNVidiaGPUTranscode( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "nVidiaGPUTranscode", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getNVidiaGPUTranscodeDefault() const
            {
                return hasNVidiaGPU();
            }

            bool CPreferences::getNVidiaGPUTranscode() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "nVidiaGPUTranscode", getNVidiaGPUTranscodeDefault() ).toBool();
            }

            void CPreferences::setSoftwareTranscode( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "SoftwareTranscode", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getSoftwareTranscodeDefault() const
            {
                return !hasIntelGPU() && !hasNVidiaGPU();
            }

            bool CPreferences::getSoftwareTranscode() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "SoftwareTranscode", getSoftwareTranscodeDefault() ).toBool();
            }

            void CPreferences::setOnlyTranscodeAudioOnFormatChange( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "OnlyTranscodeAudioOnFormatChange", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getOnlyTranscodeAudioOnFormatChange() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
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
                            if ( ( fPending & eTransformPrefs ) != 0 )
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

            void replaceText( const QString &txt, QStringList &curr, const QStringList &values )
            {
                auto pos = curr.indexOf( txt );
                Q_ASSERT( pos != -1 );
                if ( pos == -1 )
                    return;

                curr.removeAt( pos );
                for ( auto &&ii : values )
                {
                    if ( ii.lastIndexOf( QRegularExpression( R"(\S)" ) ) == -1 )
                        curr.insert( pos++, QString() );
                    else
                        curr.insert( pos++, ii );
                }
            }

            QString toString( const QString &value )
            {
                return QString( "R\"(%1)\"" ).arg( value );
            }

            QString toString( const QStringList &values )
            {
                QString retVal = "QStringList( {";
                bool first = true;
                for ( auto &&ii : values )
                {
                    if ( !first )
                        retVal += ",";
                    retVal += " " + toString( ii );
                    first = false;
                }
                retVal += " } )";
                return retVal;
            }

            template< typename FIRST, typename SECOND >
            QString toString( const std::pair< FIRST, SECOND > &value )
            {
                return QString( "{ %1, %2 }" ).arg( toString( value.first ) ).arg( toString( value.second ) );
            }

            QStringList toString( const QString &retValType, const QStringList &newValues, bool asString, int indent )
            {
                QStringList retVal;
                retVal << getIndent( indent ) + QString( "static auto defaultValue =" );

                if ( newValues.empty() )
                {
                    retVal.back() += QString( " %1();" ).arg( retValType );
                }
                else
                {
                    retVal << getIndent( indent + 1 ) + QString( "%1(" ).arg( retValType ) << getIndent( indent + 1 ) + "{";

                    bool first = true;
                    for ( auto &&ii : newValues )
                    {
                        auto stringFmt = QString( "%2" );
                        if ( asString )
                            stringFmt = toString( stringFmt );
                        auto fmt = QString( "%3%1" ) + stringFmt + " //";
                        retVal << fmt.arg( first ? " " : "," ).arg( ii ).arg( getIndent( indent + 2 ) );
                        first = false;
                    }
                    retVal << QString( "%1} );" ).arg( ( getIndent( indent + 1 ) ) );
                }

                retVal << QString( "%1return defaultValue;" ).arg( getIndent( indent ) );
                return retVal;
            }
            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const QString &boolVariable, const QString &trueValue, const QString &falseValue )
            {
                QStringList function;
                function << QString( "QString CPreferences::%1( bool %2 ) const" ).arg( funcName ).arg( boolVariable ) << "{" << getIndent( 1 ) + QString( "if ( %2 )" ).arg( boolVariable )
                         << getIndent( 2 ) + QString( "return %1;" ).arg( toString( trueValue ) ) << getIndent( 1 ) + QString( "else" ) << getIndent( 2 ) + QString( "return %1;" ).arg( toString( falseValue ) ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const QString &value )
            {
                QStringList function;
                function << QString( "QString CPreferences::%1() const" ).arg( funcName ) << "{" << getIndent( 1 ) + QString( "return %1;" ).arg( toString( value ) ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const char *value )
            {
                QStringList function;
                function << QString( "bool CPreferences::%1() const" ).arg( funcName ) << "{" << getIndent( 1 ) + QString( "return %1;" ).arg( value ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const NSABUtils::TFormatMap &value )
            {
                QStringList function;
                function //
                    << QString( "NSABUtils::TFormatMap CPreferences::%1() const" ).arg( funcName ) //
                    << "{"
                    ;

                int indent = 1;
                function   //
                    << getIndent( indent++ ) + QString( "static auto defaultValue = NSABUtils::TFormatMap(" )   //
                    << getIndent( indent++ ) + "{";
                    ;

                auto first = true;
                for( auto && ii : value )
                {
                    function << getIndent( indent++ ) + ( first ? " " : "," ) + "{";
                    function << getIndent( indent ) + toString( ii.first ) + ", std::unordered_map< QString, QStringList >";
                    function << getIndent( indent++ ) + " ( {";
                    bool innerFirst = true;
                    for( auto && jj : ii.second )
                    {
                        function << getIndent( indent ) + ( innerFirst ? " " : "," ) + "{ " + toString( jj.first ) + ", " + toString( jj.second ) + " } //"; //
                        innerFirst = false;
                    }
                    function << getIndent( --indent ) + "} )";
                    function << getIndent( --indent ) + "}";
                    first = false;
                }
                //function << toString( value, 1 );

                function << getIndent( --indent ) + "} );";
                function << getIndent( --indent ) + QString( "return defaultValue;" );
                function << getIndent( --indent ) + "}";
                Q_ASSERT( indent == 0 );

                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            template< typename T >
            auto replaceText( const QString &txt, QStringList &curr, const QString &funcName, T value ) -> typename std::enable_if< std::is_same< bool, T >::value, void >::type
            {
                QStringList function;
                function << QString( "%1 CPreferences::%2() const" ).arg( typeid( T ).name() ).arg( funcName ) << "{" << getIndent( 1 ) + QString( "return %1;" ).arg( value ? "true" : "false" ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            template< typename T >
            auto replaceText( const QString &txt, QStringList &curr, const QString &funcName, T value ) ->
                typename std::enable_if< !std::is_same< bool, T >::value && !std::is_enum< T >::value && !std::is_same< QString, T >::value && !std::is_same< char *, T >::value, void >::type
            {
                QStringList function;
                function << QString( "%1 CPreferences::%2() const" ).arg( typeid( T ).name() ).arg( funcName ) << "{" << getIndent( 1 ) + QString( "return %1;" ).arg( value ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            template< typename T >
            auto replaceText( const QString &txt, const QString & returnType, QStringList &curr, const QString &funcName, T value ) ->
                typename std::enable_if< std::is_enum< T >::value, void >::type
            {
                QStringList function;
                function << QString( "%1 CPreferences::%2() const" ).arg( returnType ).arg( funcName ) << "{" << getIndent( 1 ) + QString( "return %1;" ).arg( toString( value, true ) ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const QString &boolVariable, const QStringList &trueValue, const QStringList &falseValue, const QString &retValType = "QStringList", bool asString = true )
            {
                QStringList function;
                function << QString( "%3 CPreferences::%1( bool %2 ) const" ).arg( funcName ).arg( boolVariable ).arg( retValType ) << "{" << getIndent( 1 ) + QString( "if ( %2 )" ).arg( boolVariable ) << getIndent( 1 ) + "{"
                         << toString( retValType, trueValue, asString, 2 ) << getIndent( 1 ) + "}" << getIndent( 1 ) + "else" << getIndent( 1 ) + "{" << toString( retValType, falseValue, asString, 2 ) << getIndent( 1 ) + "}"
                         << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const QStringList &newValues, const QString &retValType = "QStringList", bool asString = true )
            {
                QStringList function;
                function << QString( "%2 CPreferences::%1() const" ).arg( funcName ).arg( retValType ) << "{";

                function << toString( retValType, newValues, asString, 1 ) << "}";

                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const QVariantMap &newValues )
            {
                QStringList varList;
                for ( auto &&ii = newValues.cbegin(); ii != newValues.cend(); ++ii )
                {
                    varList << QString( "{ %1, %2 }" ).arg( toString( ii.key() ) ).arg( toString( ii.value().toString() ) );
                }
                replaceText( txt, curr, funcName, varList, "QVariantMap", false );
            }

            QString CPreferences::compareValues( const QString &title, const QStringList &defaultValues, const QStringList &currValues ) const
            {
                if ( defaultValues == currValues )
                    return {};

                QStringList items;
                int ii = 0;
                for ( ; ( ii < defaultValues.count() ) && ( ii < currValues.count() ); ++ii )
                {
                    if ( defaultValues[ ii ] != currValues[ ii ] )
                    {
                        items << QString( "%1 != %2" ).arg( defaultValues[ ii ] ).arg( currValues[ ii ] );
                    }
                }

                if ( items.count() > 15 )
                    items.clear();

                int origII = ii;
                for ( int ii = origII; ii < defaultValues.count(); ++ii )
                {
                    items << QString( "%1 currently missing" ).arg( defaultValues[ ii ] );
                }

                for ( int ii = origII; ii < currValues.count(); ++ii )
                {
                    items << QString( "%1 not in defaults" ).arg( currValues[ ii ] );
                }

                if ( items.empty() )
                    return {};

                for ( auto &&ii : items )
                {
                    ii = "<li>" + ii.toHtmlEscaped() + "</li>";
                }

                auto retVal = QString( "<li>%1\n<ul>%2</ul>\n</li>\n" ).arg( title ).arg( items.join( "\n" ) );
                ;
                return retVal;
            }

            QStringList CPreferences::variantMapToStringList( const QVariantMap &data ) const
            {
                QStringList retVal;
                for ( auto &&ii = data.cbegin(); ii != data.cend(); ++ii )
                    retVal << QString( "%1=%2" ).arg( ii.key() ).arg( ii.value().toString() );
                return retVal;
            }

            QString CPreferences::compareValues( const QString &title, const QVariantMap &defaultValues, const QVariantMap &currValues ) const
            {
                return compareValues( title, variantMapToStringList( defaultValues ), variantMapToStringList( currValues ) );
            }

            QString CPreferences::compareValues( const QString &title, const QString &defaultValues, const QString &currValues ) const
            {
                return compareValues( title, QStringList() << defaultValues, QStringList() << currValues );
            }

            QString CPreferences::compareValues( const QString &title, bool defaultValue, bool currValue ) const
            {
                if ( defaultValue == currValue )
                    return {};

                auto retVal = QString( "<li>%1\n<ul>Default is %2, current setting is %3</ul>\n</li>\n" ).arg( title ).arg( defaultValue ? "true" : "false" ).arg( currValue ? "true" : "false" );
                return retVal;
            }

            QStringList CPreferences::getDefaultFile() const
            {
                auto retVal = QStringList() << R"(// The MIT License( MIT ))"
                                            << R"(//)"
                                            << R"(// Copyright( c ) 2020-2021 Scott Aron Bloom)"
                                            << R"(//)"
                                            << R"(// Permission is hereby granted, free of charge, to any person obtaining a copy)"
                                            << R"(// of this software and associated documentation files( the "Software" ), to deal)"
                                            << R"(// in the Software without restriction, including without limitation the rights)"
                                            << R"(// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell)"
                                            << R"(// copies of the Software, and to permit persons to whom the Software is)"
                                            << R"(// furnished to do so, subject to the following conditions :)"
                                            << R"(//)"
                                            << R"(// The above copyright notice and this permission notice shall be included in)"
                                            << R"(// all copies or substantial portions of the Software.)"
                                            << R"(//)"
                                            << R"(// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR)"
                                            << R"(// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,)"
                                            << R"(// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE)"
                                            << R"(// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER)"
                                            << R"(// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,)"
                                            << R"(// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE)"
                                            << R"(// SOFTWARE.)"
                                            << R"()"
                                            << R"(#include "Preferences.h")"
                                            << R"(#include "SABUtils/FFMpegFormats.h")"
                                            << R"()"
                                            << R"(namespace NMediaManager)"
                                            << getIndent( 0 ) +R"({)" 
                                            << getIndent( 1 ) + R"(namespace NPreferences)" 
                                            << getIndent( 1 ) + R"({)"
                                            << getIndent( 2 ) + R"(namespace NCore)" 
                                            << getIndent( 2 ) + R"({)"
                                            << "%DEFAULT_SEASON_DIR_PATTERN%"
                                            << R"()"
                                            << "%DEFAULT_OUT_DIR_PATTERN%"
                                            << R"()"
                                            << "%DEFAULT_OUT_FILE_PATTERN%"
                                            << R"()"
                                            << "%DEFAULT_CUSTOM_PATHS_TO_DELETE%"
                                            << R"()"
                                            << "%DEFAULT_DELETE_CUSTOM%"
                                            << R"()"
                                            << "%DEFAULT_RIPPED_WITH_MKV_REGEX%"
                                            << R"()"
                                            << "%DEFAULT_DELETE_EXE%"
                                            << R"()"
                                            << "%DEFAULT_DELETE_NFO%"
                                            << R"()"
                                            << "%DEFAULT_DELETE_BAK%"
                                            << R"()"
                                            << "%DEFAULT_DELETE_IMAGES%"
                                            << R"()"
                                            << "%DEFAULT_KNOWN_STRINGS%"
                                            << R"()"
                                            << "%DEFAULT_KNOWN_EXTENDED_STRINGS%"
                                            << R"()"
                                            << "%DEFAULT_IGNORED_PATHS%"
                                            << R"()"
                                            << "%DEFAULT_KNOWN_ABBREVIATIONS%"
                                            << R"()"
                                            << "%DEFAULT_KNOWN_HYPHENATED%"
                                            << R"()"
                                            << "%DEFAULT_SKIPPED_PATHS%"
                                            << R"()"
                                            << "%AVAILABLE_AUDIO_ENCODERS%"
                                            << R"()"
                                            << "%AVAILABLE_VIDEO_ENCODERS%"
                                            << R"()"
                                            << "%AVAILABLE_SUBTITLE_ENCODERS%"
                                            << R"()"
                                            << "%MEDIA_FORMAT_DEFS%" 
                                            << R"()"
                                            << "%MEDIA_FORMAT_EXTENSION_MAP%"
                                            << R"()"
                                            << "%DEFAULT_FORCE_MEDIA_CONTAINER%"
                                            << R"()"
                                            << "%DEFAULT_MEDIA_CONTAINER_NAME%"
                                            << R"()"
                                            << "%DEFAULT_TRANSCODE_AUDIO%"
                                            << R"()"
                                            << "%DEFAULT_ONLY_TRANSCODE_AUDIO_ON_FORMAT_CHANGE%"
                                            << R"()"
                                            << "%DEFAULT_TRANSCODE_TO_AUDIO_CODEC%"
                                            << R"()"
                                            << "%ALLOWED_AUDIO_CODECS%"
                                            << R"()"
                                            << "%DEFAULT_TRANSCODE_TO_H265%"
                                            << R"()"
                                            << "%DEFAULT_ONLY_TRANSCODE_VIDEO_ON_FORMAT_CHANGE%"
                                            << R"()"
                                            << "%DEFAULT_LOSSLESS_TRANSCODING%"
                                            << R"()"
                                            << "%DEFAULT_USE_CRF%"
                                            << R"()"
                                            << "%DEFAULT_USE_EXPLICIT_CRF%"
                                            << R"()"
                                            << "%DEFAULT_EXPLICIT_CRF%"
                                            << R"()"
                                            << "%DEFAULT_USE_PRESET%"
                                            << R"()"
                                            << "%DEFAULT_PRESET%"
                                            << R"()"
                                            << "%DEFAULT_USE_TUNE%"
                                            << R"()"
                                            << "%DEFAULT_TUNE%"
                                            << R"()"
                                            << "%DEFAULT_USE_PROFILE%"
                                            << R"()"
                                            << "%DEFAULT_PROFILE%" 
                                            << getIndent( 2 ) + R"(})"
                                            << getIndent( 1 ) + R"(})"
                                            << getIndent( 0 ) + R"(})"
                                            ;
                return retVal;
            }

            QString CPreferences::validateDefaults() const
            {
                auto items = QStringList() << compareValues( "Movie Out Dir Pattern", getDefaultOutDirPattern( false ), getMovieOutDirPattern() )   //
                                           << compareValues( "Movie Out File Pattern", getDefaultOutFilePattern( false ), getMovieOutFilePattern() )   //
                                           << compareValues( "Season Out Dir Pattern", getDefaultSeasonDirPattern(), getSeasonOutDirPattern() )   //
                                           << compareValues( "TV Out Dir Pattern", getDefaultOutDirPattern( true ), getTVOutDirPattern( false ) )   //
                                           << compareValues( "TV Out File Pattern", getDefaultOutFilePattern( true ), getTVOutFilePattern() )   //
                                           << compareValues( "Skipped Paths (Media Transform)", getDefaultSkippedPaths( true ), getSkippedPaths( true ) )   //
                                           << compareValues( "Skipped Paths (Media Tagging)", getDefaultSkippedPaths( false ), getSkippedPaths( false ) )   //
                                           << compareValues( "Ignored Paths", getDefaultIgnoredPaths(), getIgnoredPaths() )   //
                                           << compareValues( "Paths to Delete", getDefaultCustomPathsToDelete(), getCustomPathsToDelete() )   //
                                           << compareValues( "Ripped With MKV RegEX", getDefaultRippedWithMakeMKVRegEX(), getRippedWithMakeMKVRegEX() )   //
                                           << compareValues( "Delete Custom", getDefaultDeleteCustom(), deleteCustom() )   //
                                           << compareValues( "Delete Executables", getDefaultDeleteEXE(), deleteEXE() )   //
                                           << compareValues( "Delete NFO Files", getDefaultDeleteNFO(), deleteNFO() )   //
                                           << compareValues( "Delete BAK Files", getDefaultDeleteBAK(), deleteBAK() )   //
                                           << compareValues( "Delete Images", getDefaultDeleteImages(), deleteImages() )   //
                                           << compareValues( "Known Strings", getDefaultKnownStrings(), getKnownStrings() )   //
                                           << compareValues( "Known Extended Strings", getDefaultKnownExtendedStrings(), getKnownExtendedStrings() )   //
                                           << compareValues( "Known Abbreviations", getDefaultKnownAbbreviations(), getKnownAbbreviations() )   //
                                           << compareValues( "Known Hyphenated", getDefaultKnownHyphenated(), getKnownHyphenated() ) //
                                           << compareValues( "Audio Encoders - Terse", availableAudioEncodersDefault( false ), availableAudioEncoders( false ) )   //
                                           << compareValues( "Audio Encoders - Verbose", availableAudioEncodersDefault( true ), availableAudioEncoders( true ) )   //
                                           << compareValues( "Video Encoders - Terse", availableVideoEncodersDefault( false ), availableVideoEncoders( false ) )   //
                                           << compareValues( "Video Encoders - Verbose", availableVideoEncodersDefault( true ), availableVideoEncoders( true ) )   //
                                           << compareValues( "Subtitle Encoders - Terse", availableSubtitleEncodersDefault( false ), availableSubtitleEncoders( false ) )   //
                                           << compareValues( "Subtitle Encoders - Verbose", availableSubtitleEncodersDefault( true ), availableSubtitleEncoders( true ) )   //
                                           << compareValues( "Media Formats - Terse", availableMediaFormatsDefault( false ), availableMediaFormats( false ) )   //
                                           << compareValues( "Media Formats - Verbose", availableMediaFormatsDefault( true ), availableMediaFormats( true ) )   //

                                           << compareValues( "Force Media Format", getForceMediaContainerDefault(), getForceMediaContainer() )   //
                                           << compareValues( "Force Media Format Name", getForceMediaContainerNameDefault(), getForceMediaContainerName() )   //
                            
                                           << compareValues( "Transcode Audio", getTranscodeAudioDefault(), getTranscodeAudio() )   //
                                           << compareValues( "Transcode Audio on Format Change", getOnlyTranscodeAudioOnFormatChangeDefault(), getOnlyTranscodeAudioOnFormatChange() )   //
                                           << compareValues( "Transcode Audio Codec", getTranscodeToAudioCodecDefault(), getTranscodeToAudioCodec() )   //
                                           << compareValues( "Allowed Audio Codecs", getAllowedAudioCodecsDefault(), getAllowedAudioCodecs() )   //

                                           << compareValues( "Transcode To H265", getTranscodeToH265Default(), getTranscodeToH265() )   //
                                           << compareValues( "Transcode Video on Format Change", getOnlyTranscodeVideoOnFormatChangeDefault(), getOnlyTranscodeVideoOnFormatChange() )   //
                                           << compareValues( "Lossless Video Transcoding", getLosslessTranscodingDefault(), getLosslessTranscoding() )   //
                                           << compareValues( "Use CRF", getUseCRFDefault(), getUseCRF() )   //
                                           << compareValues( "Use Explicit CRF", getUseExplicitCRFDefault(), getUseExplicitCRF() )   //
                                           << compareValues( "Explicit CRF", getExplicitCRFDefault(), getExplicitCRF() )   //
                                           << compareValues( "Use Preset", getUsePresetDefault(), getUsePreset() )   //
                                           << compareValues( "Preset", toString( getPresetDefault() ), toString( getPreset() ) )   //
                                           << compareValues( "Use Tune", getUseTuneDefault(), getUseTune() )   //
                                           << compareValues( "Tune", toString( getTuneDefault() ), toString( getTune() ) )   //
                                           << compareValues( "Use Profile", getUseProfileDefault(), getUseProfile() )   //
                                           << compareValues( "Profile", toString( getProfileDefault() ), toString( getProfile() ) )   //
                    ;
                items.removeAll( QString() );

                QString retVal;
                if ( !items.isEmpty() )
                    retVal = QString( "<style>p{ white-space:nowrap }</style>\n<p>Difference in Settings:\n<ul>\n%2\n</ul>\n</p>" ).arg( items.join( "\n" ) );
                return retVal;
            }

            void CPreferences::showValidateDefaults( QWidget *parent, bool showNoChange )
            {
                auto diffs = validateDefaults();
                if ( diffs.isEmpty() && !showNoChange )
                    return;

                bool copyDefaults = false;
                if ( !diffs.isEmpty() )
                {
                    NSABUtils::CScrollMessageBox dlg( parent );
                    dlg.setIconLabel( QMessageBox::Warning );
                    dlg.setLabel( "Warning:" );
                    dlg.setWindowTitle( tr( "Preferences have changed:" ) );
                    dlg.setHtmlText( diffs );
                    dlg.setButtons( QDialogButtonBox::StandardButton::Ok | QDialogButtonBox::StandardButton::Apply );
                    dlg.button( QDialogButtonBox::StandardButton::Apply )->setText( tr( "Copy to Clipboard" ) );
                    dlg.exec();
                    copyDefaults = dlg.buttonClicked() == dlg.button( QDialogButtonBox::StandardButton::Apply );
                }
                else if ( showNoChange )
                {
                    copyDefaults = QMessageBox::information( parent, tr( "Default Preferences are the Same" ), tr( "The defaults are up to date.\nWould you like to copy the defaults to the Clipboard" ), QMessageBox::Yes, QMessageBox::No )
                              == QMessageBox::Yes;
                }

                if ( copyDefaults )
                {
                    auto newFileText = getDefaultFile();
                    replaceText( "%DEFAULT_SEASON_DIR_PATTERN%", newFileText, "getDefaultSeasonDirPattern", getSeasonOutDirPattern() );
                    replaceText( "%DEFAULT_OUT_DIR_PATTERN%", newFileText, "getDefaultOutDirPattern", "forTV", getTVOutDirPattern( false ), getMovieOutDirPattern() );
                    replaceText( "%DEFAULT_OUT_FILE_PATTERN%", newFileText, "getDefaultOutFilePattern", "forTV", getTVOutFilePattern(), getMovieOutFilePattern() );
                    replaceText( "%DEFAULT_CUSTOM_PATHS_TO_DELETE%", newFileText, "getDefaultCustomPathsToDelete", getCustomPathsToDelete() );
                    replaceText( "%DEFAULT_DELETE_CUSTOM%", newFileText, "getDefaultDeleteCustom", "!getDefaultCustomPathsToDelete().isEmpty()" );
                    replaceText( "%DEFAULT_RIPPED_WITH_MKV_REGEX%", newFileText, "getDefaultRippedWithMakeMKVRegEX", getRippedWithMakeMKVRegEX() );
                    replaceText( "%DEFAULT_DELETE_EXE%", newFileText, "getDefaultDeleteEXE", deleteEXE() );
                    replaceText( "%DEFAULT_DELETE_NFO%", newFileText, "getDefaultDeleteNFO", deleteNFO() );
                    replaceText( "%DEFAULT_DELETE_BAK%", newFileText, "getDefaultDeleteBAK", deleteBAK() );
                    replaceText( "%DEFAULT_DELETE_IMAGES%", newFileText, "getDefaultDeleteImages", deleteImages() );
                    replaceText( "%DEFAULT_KNOWN_STRINGS%", newFileText, "getDefaultKnownStrings", getKnownStrings() );
                    replaceText( "%DEFAULT_KNOWN_EXTENDED_STRINGS%", newFileText, "getDefaultKnownExtendedStrings", getKnownExtendedStrings() );
                    replaceText( "%DEFAULT_IGNORED_PATHS%", newFileText, "getDefaultIgnoredPaths", getIgnoredPaths() );
                    replaceText( "%DEFAULT_KNOWN_ABBREVIATIONS%", newFileText, "getDefaultKnownAbbreviations", getKnownAbbreviations() );
                    replaceText( "%DEFAULT_KNOWN_HYPHENATED%", newFileText, "getDefaultKnownHyphenated", getKnownHyphenated() );
                    replaceText( "%DEFAULT_SKIPPED_PATHS%", newFileText, "getDefaultSkippedPaths", "forMediaNaming", getSkippedPaths( true ), getSkippedPaths( false ) );

                    replaceText( "%AVAILABLE_AUDIO_ENCODERS%", newFileText, "availableAudioEncodersDefault", "verbose", availableAudioEncoders( true ), availableAudioEncoders( false ) );
                    replaceText( "%AVAILABLE_VIDEO_ENCODERS%", newFileText, "availableVideoEncodersDefault", "verbose", availableVideoEncoders( true ), availableVideoEncoders( false ) );
                    replaceText( "%AVAILABLE_SUBTITLE_ENCODERS%", newFileText, "availableSubtitleEncodersDefault", "verbose", availableSubtitleEncoders( true ), availableSubtitleEncoders( false ) );

                    replaceText( "%MEDIA_FORMAT_DEFS%", newFileText, "availableMediaFormatsDefault", "verbose", availableMediaFormats( true ), availableMediaFormats( false ) );
                    replaceText( "%MEDIA_FORMAT_EXTENSION_MAP%", newFileText, "getVideoExtensionsMapDefault", getFormatExtensionsMap() );

                    replaceText( "%DEFAULT_FORCE_MEDIA_CONTAINER%", newFileText, "getForceMediaContainerDefault", getForceMediaContainer() );
                    replaceText( "%DEFAULT_MEDIA_CONTAINER_NAME%", newFileText, "getForceMediaContainerNameDefault", getForceMediaContainerName() );
                    replaceText( "%DEFAULT_TRANSCODE_AUDIO%", newFileText, "getTranscodeAudioDefault", getTranscodeAudio() );
                    replaceText( "%DEFAULT_ONLY_TRANSCODE_AUDIO_ON_FORMAT_CHANGE%", newFileText, "getOnlyTranscodeAudioOnFormatChangeDefault", getOnlyTranscodeAudioOnFormatChange() );
                    replaceText( "%DEFAULT_TRANSCODE_TO_AUDIO_CODEC%", newFileText, "getTranscodeToAudioCodecDefault", getTranscodeToAudioCodec() );
                    replaceText( "%ALLOWED_AUDIO_CODECS%", newFileText, "getAllowedAudioCodecsDefault", getAllowedAudioCodecs() );


                    replaceText( "%DEFAULT_TRANSCODE_TO_H265%", newFileText, "getTranscodeToH265Default", getTranscodeToH265() );
                    replaceText( "%DEFAULT_ONLY_TRANSCODE_VIDEO_ON_FORMAT_CHANGE%", newFileText, "getOnlyTranscodeVideoOnFormatChangeDefault", getOnlyTranscodeVideoOnFormatChange() );
                    replaceText( "%DEFAULT_LOSSLESS_TRANSCODING%", newFileText, "getLosslessTranscodingDefault", getLosslessTranscoding() );
                    replaceText( "%DEFAULT_USE_CRF%", newFileText, "getUseCRFDefault", getUseCRF() );
                    replaceText( "%DEFAULT_USE_EXPLICIT_CRF%", newFileText, "getUseExplicitCRFDefault", getUseExplicitCRF() );
                    replaceText( "%DEFAULT_EXPLICIT_CRF%", newFileText, "getExplicitCRFDefault", getExplicitCRF() );
                    replaceText( "%DEFAULT_USE_PRESET%", newFileText, "getUsePresetDefault", getUsePreset() );
                    replaceText( "%DEFAULT_PRESET%", "EMakeMKVPreset", newFileText, "getPresetDefault", getPreset() );
                    replaceText( "%DEFAULT_USE_TUNE%", newFileText, "getUseTuneDefault", getUseTune() );
                    replaceText( "%DEFAULT_TUNE%", "EMakeMKVTune",  newFileText, "getTuneDefault", getTune() );
                    replaceText( "%DEFAULT_USE_PROFILE%", newFileText, "getUseProfileDefault", getUseProfile() );
                    replaceText( "%DEFAULT_PROFILE%", "EMakeMKVProfile", newFileText, "getProfileDefault", getProfile() );

                    QGuiApplication::clipboard()->setText( newFileText.join( "\n" ) );
                }
            }
        }
    }
}
