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

        QString toString( EPreferenceType prefType )
        {
            switch ( prefType )
            {
                case eSystemPrefs:
                    return "System";
                case eLoadPrefs:
                    return "Load";
                case eTransformPrefs:
                    return "Transform";
                case eTagPrefs:
                    return "Tags";
                case eExtToolsPrefs:
                    return "ExternalTools";
                case eGIFPrefs:
                    return "GIF";
                case eBIFPrefs:
                    return "BIF";
                case eMakeMKVPrefs:
                    return "MakeMKV";
                default:
                    return "";
            }
        }

        namespace NCore
        {
            QString toString( EMakeMKVPreset preset )
            {
                switch ( preset )
                {
                    case EMakeMKVPreset::eUltraFast:
                        return "ultrafast";
                    case EMakeMKVPreset::eSuperFast:
                        return "superfast";
                    case EMakeMKVPreset::eVeryFast:
                        return "veryfast";
                    case EMakeMKVPreset::eFaster:
                        return "faster";
                    case EMakeMKVPreset::eFast:
                        return "fast";
                    case EMakeMKVPreset::eMedium:
                        return "medium";
                    case EMakeMKVPreset::eSlow:
                        return "slow";
                    case EMakeMKVPreset::eSlower:
                        return "slower";
                    case EMakeMKVPreset::eVerySlow:
                        return "verislow";
                }
                return "medium";
            }

            QString toString( EMakeMKVTune preset )
            {
                switch ( preset )
                {
                    case EMakeMKVTune::eFilm:
                        return "film";
                    case EMakeMKVTune::eAnimation:
                        return "animation";
                    case EMakeMKVTune::eGrain:
                        return "grain";
                    case EMakeMKVTune::eStillImage:
                        return "stillimage";
                    case EMakeMKVTune::eFastDecode:
                        return "fastdecode";
                    case EMakeMKVTune::eZeroLatency:
                        return "zerolatency";
                }
                return "film";
            }

            QString toString( EMakeMKVProfile profile )
            {
                switch ( profile )
                {
                    case EMakeMKVProfile::eMain:
                        return "main";
                    case EMakeMKVProfile::eMainIntra:
                        return "main-intra";
                    case EMakeMKVProfile::eMailStillPicture:
                        return "mailstillpicture";
                    case EMakeMKVProfile::eMain444_8:
                        return "main444-8";
                    case EMakeMKVProfile::eMain444Intra:
                        return "main444-intra";
                    case EMakeMKVProfile::eMain444StillPicture:
                        return "main444-stillpicture";
                    case EMakeMKVProfile::eMain10:
                        return "main10";
                    case EMakeMKVProfile::eMain10Intra:
                        return "main10-intra";
                    case EMakeMKVProfile::eMain422_10:
                        return "main422-10";
                    case EMakeMKVProfile::eMain422_10Intra:
                        return "main422-10-intra";
                    case EMakeMKVProfile::eMain444_10:
                        return "main444-10";
                    case EMakeMKVProfile::eMain444_10Intra:
                        return "main444-10-intra";
                    case EMakeMKVProfile::eMain12:
                        return "main12";
                    case EMakeMKVProfile::eMain12Intra:
                        return "main12-intra";
                    case EMakeMKVProfile::eMain422_12:
                        return "main422-12";
                    case EMakeMKVProfile::eMain422_12Intra:
                        return "main422-12-intra";
                    case EMakeMKVProfile::eMain444_12:
                        return "main444-12";
                    case EMakeMKVProfile::eMain444_12Intra:
                        return "main444012-intra";
                }
                return "main";
            }

            CPreferences *CPreferences::instance()
            {
                static CPreferences retVal;
                return &retVal;
            }

            CPreferences::CPreferences()
            {
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

            void CPreferences::setMediaExtensions( const QStringList &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "MediaExtensions", value );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            void CPreferences::setMediaExtensions( const QString &value )
            {
                setMediaExtensions( value.toLower().split( ";" ) );
            }

            QStringList CPreferences::getVideoExtensions() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                auto retVal = settings.value( "MediaExtensions", defaultVideoExtensions( false ) ).toStringList();
                for ( auto &&ii : retVal )
                    ii = ii.toLower();
                return retVal;
            }

            QStringList CPreferences::defaultVideoExtensions( bool forceReset ) const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );

                QStringList videoExtensions;
                if ( forceReset || !settings.contains( "MediaExtensions" ) )
                {
                    loadFFmpegFormats( forceReset );

                    for ( auto &&ii : fMediaFormatExtensions )
                        videoExtensions << ii.second;

                    videoExtensions.removeDuplicates();
                    settings.setValue( "MediaExtensions", videoExtensions );
                }
                else
                    videoExtensions = settings.value( "MediaExtensions" ).toStringList();
                return videoExtensions;
            }

            void CPreferences::setSubtitleExtensions( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                settings.setValue( "SubtitleExtensions", value );
                emitSigPreferencesChanged( EPreferenceType::eSystemPrefs );
            }

            void CPreferences::setSubtitleExtensions( const QStringList &value )
            {
                setSubtitleExtensions( value.join( ";" ) );
            }

            QStringList CPreferences::getSubtitleExtensions() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                return settings.value( "SubtitleExtensions", QString( "*.idx;*.sub;*.srt" ) ).toString().split( ";" );
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

            QString CPreferences::getMKVValidatorEXE() const
            {
                auto exe = QDir( qApp->applicationDirPath() ).absoluteFilePath( "mkvalidator.exe" );
                auto fi = QFileInfo( exe );
                bool aOK = !exe.isEmpty() && fi.isExecutable();
                return aOK ? exe : QString();
            }

            void CPreferences::setMKVMergeEXE( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "MKVMergeEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getMKVMergeEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                auto retVal = settings.value( "MKVMergeEXE", QString( "C:/Program Files/MKVToolNix/mkvmerge.exe" ) ).toString();

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
            }

            void CPreferences::setMKVPropEditEXE( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "MKVPropEditEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getMKVPropEditEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                auto retVal = settings.value( "MKVPropEditEXE", QString( "C:/Program Files/MKVToolNix/mkvpropedit.exe" ) ).toString();

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
            }

            void CPreferences::setFFMpegEXE( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "FFMpegEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getFFMpegEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                auto retVal = settings.value( "FFMpegEXE", QString() ).toString();

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
            }

            void CPreferences::setFFProbeEXE( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                settings.setValue( "FFProbeEXE", value );
                emitSigPreferencesChanged( EPreferenceType::eExtToolsPrefs );
            }

            QString CPreferences::getFFProbeEXE() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eExtToolsPrefs ) );
                auto retVal = settings.value( "FFProbeEXE", QString() ).toString();

                auto fi = QFileInfo( retVal );
                bool aOK = !retVal.isEmpty() && fi.isExecutable();
                return aOK ? retVal : QString();
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
            QStringList CPreferences::getTranscodeArgs( const QString &srcName, const QString &destName ) const
            {
                auto mediaInfo = std::make_shared< NSABUtils::CMediaInfo >( srcName );
                return getTranscodeArgs( mediaInfo, srcName, destName );
            }

            STranscodeNeeded::STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo )
            {
                fFormat = NPreferences::NCore::CPreferences::instance()->getForceMediaFormat() && mediaInfo && !mediaInfo->isFormat( NPreferences::NCore::CPreferences::instance()->getForceMediaFormatName() );

                fVideo = NPreferences::NCore::CPreferences::instance()->getTranscodeToH265() && mediaInfo && !mediaInfo->isHEVCVideo();
                fAudio = NPreferences::NCore::CPreferences::instance()->getTranscodeAudio() && mediaInfo && !mediaInfo->isAudioCodec( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );

                if ( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeVideoOnFormatChange() )
                {
                    fVideo &= fFormat;
                }
                
                if ( NPreferences::NCore::CPreferences::instance()->getOnlyTranscodeAudioOnFormatChange() )
                {
                    fAudio &= fFormat;
                }
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
                
                retVal << "-f" << getForceMediaFormatName()   //
                       << destName;

                return retVal;
            }

            QStringList CPreferences::availableAudioEncoders( bool verbose ) const
            {
                loadCodecs();
                if ( verbose )
                    return fAudioCodecsVerbose;
                else
                    return fAudioCodecsTerse;
            }

            QStringList CPreferences::availableVideoEncoders( bool verbose ) const
            {
                loadCodecs();
                if ( verbose )
                    return fVideoCodecsVerbose;
                else
                    return fVideoCodecsTerse;
            }

            QStringList CPreferences::availableSubtitleEncoders( bool verbose ) const
            {
                loadCodecs();
                if ( verbose )
                    return fSubtitleCodecsVerbose;
                else
                    return fSubtitleCodecsTerse;
            }

            QStringList CPreferences::availableMediaFormats( bool verbose ) const
            {
                loadFFmpegFormats( false );
                if ( verbose )
                    return fMediaFormatsVerbose;
                else
                    return fMediaFormatsTerse;
            }

            std::tuple< QStringList, QStringList, std::unordered_map< QString, QStringList >, std::unordered_map< QString, QString > > CPreferences::getAllFFmpegFormats( const QString &ffmpeg )
            {
                QStringList mediaFormatsTerse;
                QStringList mediaFormatsVerbose;
                std::unordered_map< QString, QStringList > mediaFormatExtensions;
                std::unordered_map< QString, QString > reverseMediaFormatExtensions;

                QProcess process;
                process.start(
                    ffmpeg, QStringList() << "-hide_banner"
                                          << "-formats" );
                process.waitForFinished();
                auto formats = process.readAllStandardOutput();

                auto pos = formats.indexOf( "--" );
                if ( pos == -1 )
                    return { mediaFormatsTerse, mediaFormatsVerbose, mediaFormatExtensions, reverseMediaFormatExtensions };

                formats = formats.mid( pos + 2 );
                /*
                        * Encoders:
                           D. = Demuxing supported
                           .E = Muxing supported
                        
                           D  3dostr          3DO STR
                            E 3g2             3GP2 (3GPP2 file format)
                        */

                auto tmp = QImageReader::supportedImageFormats();
                std::unordered_set< QString > imageFormats;
                for ( auto &&ii : tmp )
                    imageFormats.insert( "*." + ii );

                auto regEx = QRegularExpression( R"((?<type>[DE]{1,2})\s+(?<name>\S+)\s+(?<desc>.*))" );
                auto ii = regEx.globalMatch( formats );
                while ( ii.hasNext() )
                {
                    auto match = ii.next();
                    auto type = match.captured( "type" );
                    if ( ( type.length() == 1 ) && ( type[ 0 ] != 'E' ) )
                        continue;
                    if ( ( type.length() == 2 ) && ( type[ 1 ] != 'E' ) )
                        continue;
                    auto names = match.captured( "name" ).trimmed().split( "," );
                    auto desc = match.captured( "desc" ).trimmed();

                    for ( auto &&name : names )
                    {
                        auto exts = getExtensionsForMediaFormat( name, ffmpeg, mediaFormatExtensions, reverseMediaFormatExtensions, imageFormats );
                        if ( exts.empty() )
                            continue;

                        mediaFormatsTerse.push_back( name );
                        mediaFormatsVerbose.push_back( name + " - " + desc + " (" + exts.join( ";" ) + ")" );
                    }
                }
                Q_ASSERT( mediaFormatsTerse.count() == mediaFormatsVerbose.count() );
                return { mediaFormatsTerse, mediaFormatsVerbose, mediaFormatExtensions, reverseMediaFormatExtensions };
            }

            void CPreferences::loadFFmpegFormats( bool forceLoad ) const
            {
                if ( !fMediaFormatsLoaded.has_value() )
                {
                    auto ffmpeg = getFFMpegEXE();
                    if ( ffmpeg.isEmpty() )
                        return;

                    QSettings settings;
                    settings.beginGroup( toString( EPreferenceType::eSystemPrefs ) );
                    settings.beginGroup( "MediaFormats" );
                    auto hasFormats = !settings.childGroups().isEmpty();
                    settings.endGroup();

                    if ( forceLoad || !hasFormats )
                    {
                        std::tie( fMediaFormatsTerse, fMediaFormatsVerbose, fMediaFormatExtensions, fReverseMediaFormatExtensions ) = getAllFFmpegFormats( ffmpeg );

                        settings.beginWriteArray( "MediaFormats", fMediaFormatsTerse.size() );
                        for ( int ii = 0; ii < fMediaFormatsTerse.size(); ++ii )
                        {
                            settings.setArrayIndex( ii );
                            settings.setValue( "terse", fMediaFormatsTerse[ ii ] );
                            settings.setValue( "verbose", fMediaFormatsVerbose[ ii ] );
                            auto pos = fMediaFormatExtensions.find( fMediaFormatsTerse[ ii ] );
                            if ( pos != fMediaFormatExtensions.end() )
                            {
                                settings.setValue( "extensions", ( *pos ).second );
                            }
                        }
                        settings.endArray();
                    }
                    else
                    {
                        fMediaFormatExtensions.clear();
                        fMediaFormatsTerse.clear();
                        fMediaFormatsVerbose.clear();
                        fReverseMediaFormatExtensions.clear();

                        auto arraySize = settings.beginReadArray( "MediaFormats" );
                        for ( int ii = 0; ii < arraySize; ++ii )
                        {
                            settings.setArrayIndex( ii );
                            auto terse = settings.value( "terse" ).toString();
                            auto verbose = settings.value( "verbose" ).toString();
                            auto exts = settings.value( "extensions" ).toStringList();

                            fMediaFormatsTerse.push_back( terse );
                            fMediaFormatsVerbose.push_back( verbose );
                            fMediaFormatExtensions[ terse ] = exts;
                            for ( auto &&ii : exts )
                                fReverseMediaFormatExtensions[ ii ] = terse;
                        }
                        settings.endArray();
                    }
                    fMediaFormatsLoaded = true;
                }
                Q_ASSERT( fMediaFormatsTerse.count() == fMediaFormatsVerbose.count() );
            }

            QStringList CPreferences::getExtensionsForMediaFormat(
                const QString &formatName, const QString &ffmpegExe, std::unordered_map< QString, QStringList > &forwardMap, std::unordered_map< QString, QString > &reverseMap, const std::unordered_set< QString > &imageFormats )
            {
                if ( ffmpegExe.isEmpty() )
                    return {};

                auto pos = forwardMap.find( formatName );
                if ( pos == forwardMap.end() )
                {
                    QProcess process;
                    process.start(
                        ffmpegExe, QStringList() << "-hide_banner"
                                                 << "-h"
                                                 << "muxer=" + formatName );
                    process.waitForFinished();
                    auto formatHelp = process.readAllStandardOutput();

                    // Common extensions: 3g2.
                    auto regEx = QRegularExpression( R"(Common extensions\:\s(?<exts>.*)\.)" );
                    auto match = regEx.match( formatHelp );
                    QStringList exts;
                    if ( match.hasMatch() )
                    {
                        auto tmp = match.captured( "exts" ).trimmed();
                        exts = match.captured( "exts" ).trimmed().split( "," );
                        for ( auto &&ii : exts )
                            ii = "*." + ii;
                    }

                    bool isImageFormat = false;
                    for ( auto &&ii : exts )
                    {
                        if ( imageFormats.find( ii ) != imageFormats.end() )
                        {
                            isImageFormat = true;
                            break;
                        }
                    }
                    if ( isImageFormat )
                        return {};

                    for ( auto &&ii : exts )
                        reverseMap[ ii ] = formatName;

                    pos = forwardMap.insert( { formatName, exts } ).first;
                }
                return ( *pos ).second;
            }

            QStringList CPreferences::getExtensionsForMediaFormat( const QString &formatName ) const
            {
                auto exts = imageExtensions();
                std::unordered_set< QString > imageExts;
                for ( auto &&ii : exts )
                    imageExts.insert( ii );
                return getExtensionsForMediaFormat( formatName, getFFMpegEXE(), fMediaFormatExtensions, fReverseMediaFormatExtensions, imageExts );
            }

            void CPreferences::loadCodecs() const
            {
                if ( !fCodecsLoaded.has_value() )
                {
                    auto ffmpeg = getFFMpegEXE();
                    if ( ffmpeg.isEmpty() )
                        return;

                    QProcess process;
                    process.start(
                        ffmpeg, QStringList() << "-hide_banner"
                                              << "-encoders" );
                    process.waitForFinished();
                    auto codecs = process.readAllStandardOutput();

                    auto pos = codecs.indexOf( "------" );
                    if ( pos == -1 )
                        return;

                    codecs = codecs.mid( pos + 6 );
                    /*
                        * Encoders:
                           V..... = Video
                           A..... = Audio
                           S..... = Subtitle
                        .   F.... = Frame-level multithreading
                            .S... = Slice-level multithreading
                         ...  X.. = Codec is experimental
                        ....   B. = Supports draw_horiz_band
                        .....   D = Supports direct rendering method 1
                        
                        
                           V....D a64multi             Multicolor charset for Commodore 64 (codec a64_multi)
                        */
                    auto regEx = QRegularExpression( R"((?<type>[VAS][FSXBD\.]{5})\s+(?<name>\S+)\s+(?<desc>.*))" );
                    auto ii = regEx.globalMatch( codecs );
                    while ( ii.hasNext() )
                    {
                        auto match = ii.next();
                        auto type = match.captured( "type" );
                        if ( type.length() != 6 )
                            continue;
                        if ( type[ 3 ] == 'X' )
                            continue;
                        if ( ( type[ 0 ] != 'V' ) && ( type[ 0 ] != 'A' ) && ( type[ 0 ] != 'S' ) )
                            continue;
                        auto name = match.captured( "name" );
                        auto desc = match.captured( "desc" );
                        switch ( type[ 0 ].toLatin1() )
                        {
                            case 'A':
                                fAudioCodecsTerse.push_back( name );
                                fAudioCodecsVerbose.push_back( name + " - " + desc );
                                break;
                            case 'V':
                                fVideoCodecsTerse.push_back( name );
                                fVideoCodecsVerbose.push_back( name + " - " + desc );
                                break;
                            case 'S':
                                fSubtitleCodecsTerse.push_back( name );
                                fSubtitleCodecsVerbose.push_back( name + " - " + desc );
                                break;
                            default:
                                break;
                        }
                    }
                    fCodecsLoaded = true;
                }
            }

            void CPreferences::setForceMediaFormat( bool value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "ForceMediaFormat", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            bool CPreferences::getForceMediaFormat() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "ForceMediaFormat", true ).toBool();
            }

            void CPreferences::setForceMediaFormatName( const QString &value )
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                settings.setValue( "ForceMediaFormatName", value );
                emitSigPreferencesChanged( EPreferenceType::eMakeMKVPrefs );
            }

            QString CPreferences::getForceMediaFormatName() const
            {
                QSettings settings;
                settings.beginGroup( toString( EPreferenceType::eMakeMKVPrefs ) );
                return settings.value( "ForceMediaFormatName", "matroska" ).toString();
            }

            QString CPreferences::getForceMediaFormatExt() const
            {
                auto format = getForceMediaFormatName();
                if ( format.toLower() == "matroska" )
                    return "mkv";
                else
                    return format;
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
                return settings.value( "TranscodeAudio", false ).toBool();
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
                return settings.value( "OnlyTranscodeVideoOnFormatChange", false ).toBool();
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
                return settings.value( "AudioCodec", "aac" ).toString();
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
                return settings.value( "ConvertToH265", true ).toBool();
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
                return settings.value( "LosslessTranscoding", true ).toBool();
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
                return settings.value( "UseCRF", false ).toBool();
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
                return settings.value( "UseConstantRateFactor", false ).toBool();
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
                return settings.value( "ExplicitCRF", 28 ).toInt();
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
                return settings.value( "UsePreset", false ).toBool();
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
                return static_cast< EMakeMKVPreset >( settings.value( "Preset", EMakeMKVPreset::eMedium ).toInt() );
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
                return settings.value( "UseTune", false ).toBool();
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
                return static_cast< EMakeMKVTune >( settings.value( "Tune", EMakeMKVTune::eFilm ).toInt() );
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
                return settings.value( "UseProfile", false ).toBool();
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
                return static_cast< EMakeMKVProfile >( settings.value( "Profile", EMakeMKVProfile::eMain422_12 ).toInt() );
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
                return settings.value( "OnlyTranscodeAudioOnFormatChange", false ).toBool();
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
                    fi, [ this ]() { return getVideoExtensions(); }, fMediaExtensionsHash, fIsMediaExtension );
            }

            // only return true for X_Lang.srt files or subs directories
            bool CPreferences::isSubtitleFile( const QFileInfo &fi, bool *isLangFileFormat ) const
            {
                if ( isLangFileFormat )
                {
                    *isLangFileFormat = NMediaManager::NCore::SLanguageInfo::isLangFileFormat( fi );
                }

                return isFileWithExtension(
                    fi, [ this ]() { return getSubtitleExtensions(); }, fSubtitleExtensionsHash, fIsSubtitleExtension );
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

            QString getIndent( int indent )
            {
                return QString( 4 * indent, QChar( ' ' ) );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const QString &boolVariable, const QString &trueValue, const QString &falseValue )
            {
                QStringList function;
                function << QString( "QString CPreferences::%1( bool %2 ) const" ).arg( funcName ).arg( boolVariable ) << "{" << getIndent( 1 ) + QString( "if ( %2 )" ).arg( boolVariable )
                         << getIndent( 2 ) + QString( "return R\"(%1)\";" ).arg( trueValue ) << getIndent( 1 ) + QString( "else" ) << getIndent( 2 ) + QString( "return R\"(%1)\";" ).arg( falseValue ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const QString &value )
            {
                QStringList function;
                function << QString( "QString CPreferences::%1() const" ).arg( funcName ) << "{" << getIndent( 1 ) + QString( "return R\"(%1)\";" ).arg( value ) << "}";
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

            template< typename T >
            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, T value )
            {
                static_assert( std::is_same< bool, T >::value, "not bool" );
                QStringList function;
                function << QString( "bool CPreferences::%1() const" ).arg( funcName ) << "{" << getIndent( 1 ) + QString( "return %1;" ).arg( value ? "true" : "false" ) << "}";
                for ( auto &&ii : function )
                {
                    ii = getIndent( 3 ) + ii;
                }
                return replaceText( txt, curr, function );
            }

            QStringList getListDefValue( const QString &retValType, const QStringList &newValues, bool asString, int indent )
            {
                QStringList retVal;
                retVal << QString( "%1static auto defaultValue =" ).arg( getIndent( indent ) );

                if ( newValues.empty() )
                {
                    retVal.back() += QString( " %1();" ).arg( retValType );
                }
                else
                {
                    retVal << QString( "%2%1(" ).arg( retValType ).arg( getIndent( indent + 1 ) ) << getIndent( indent + 1 ) + "{";

                    bool first = true;
                    for ( auto &&ii : newValues )
                    {
                        auto fmt = asString ? QString( "%3%1R\"(%2)\"" ) : QString( "%3%1%2" );
                        retVal << fmt.arg( first ? " " : "," ).arg( ii ).arg( getIndent( indent + 2 ) );
                        first = false;
                    }
                    retVal << QString( "%1} );" ).arg( ( getIndent( indent + 1 ) ) );
                }

                retVal << QString( "%1return defaultValue;" ).arg( getIndent( indent ) );
                return retVal;
            }

            void replaceText(
                const QString &txt, QStringList &curr, const QString &funcName, const QString &boolVariable, const QStringList &trueValue, const QStringList &falseValue, const QString &retValType = "QStringList", bool asString = true )
            {
                QStringList function;
                function << QString( "%3 CPreferences::%1( bool %2 ) const" ).arg( funcName ).arg( boolVariable ).arg( retValType ) << "{" << getIndent( 1 ) + QString( "if ( %2 )" ).arg( boolVariable ) << getIndent( 1 ) + "{"
                         << getListDefValue( retValType, trueValue, asString, 2 ) << getIndent( 1 ) + "}" << getIndent( 1 ) + "else" << getIndent( 1 ) + "{" << getListDefValue( retValType, falseValue, asString, 2 ) << getIndent( 1 ) + "}"
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

                function << getListDefValue( retValType, newValues, asString, 1 ) << "}";

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
                    varList << QString( "{ R\"(%1)\", R\"(%2)\" }" ).arg( ii.key() ).arg( ii.value().toString() );
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
                                            << R"()"
                                            << R"(namespace NMediaManager)"
                                            << R"({)"
                                            << R"(    namespace NPreferences)"
                                            << R"(    {)"
                                            << R"(        namespace NCore)"
                                            << R"(        {)"
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
                                            << R"(        })"
                                            << R"(    })"
                                            << R"(})";
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
                                           << compareValues( "Known Hyphenated", getDefaultKnownHyphenated(), getKnownHyphenated() );
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

                bool replace = false;
                if ( !diffs.isEmpty() )
                {
                    QMessageBox dlg( parent );
                    dlg.setIcon( QMessageBox::Warning );
                    dlg.setWindowTitle( tr( "Preferences have changed:" ) );
                    dlg.setText( diffs );
                    dlg.setStandardButtons( QMessageBox::StandardButton::Ok | QMessageBox::StandardButton::Apply );
                    dlg.button( QMessageBox::StandardButton::Apply )->setText( tr( "Copy to Clipboard" ) );
                    replace = ( dlg.exec() == QMessageBox::Apply );
                }
                else if ( showNoChange )
                {
                    replace = QMessageBox::information( parent, tr( "Default Preferences are the Same" ), tr( "The defaults are up to date.\nWould you like to copy the defaults to the Clipboard" ), QMessageBox::Yes, QMessageBox::No )
                              == QMessageBox::Yes;
                }

                if ( replace )
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

                    QGuiApplication::clipboard()->setText( newFileText.join( "\n" ) );
                }
            }
        }
    }
}