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
#include "LanguageInfo.h"
#include "SABUtils/QtUtils.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileInfo>
#include <QRegularExpression>
#include <QMap>
#include <QDir>
#include <QVariant>
#include <QString>

#include <optional>
#include <unordered_set>

namespace NMediaManager
{
    namespace NCore
    {

        CPreferences * CPreferences::instance()
        {
            static CPreferences retVal;
            return &retVal;
        }

        CPreferences::CPreferences()
        {}

        CPreferences::~CPreferences()
        {}

        //QString CPreferences::getDefaultInPattern( bool forTV ) const
        //{
        //    if ( forTV )
        //        return "(?<title>.+)\\.([Ss](?<season>\\d+))([Ee](?<episode>\\d+))(\\.(?<episode_title>.*))?\\.(1080|720|2160)(p|i)?.*";
        //    else
        //        return "(?<title>.+)\\.(?<year>(\\d{2}){1,2})\\..*";
        //}


        QString CPreferences::getDefaultOutDirPattern( bool forTV ) const
        {
            if ( forTV )
                return "<title>( (<year>)):<year>/Season <season>";
            else
                return "<title>( (<year>)):<year>( [tmdbid=<tmdbid>]):<tmdbid>( - <extra_info>):<extra_info>";
        }

        QString CPreferences::getDefaultOutFilePattern( bool forTV ) const
        {
            if ( forTV )
                return "<title> - S<season>E<episode>( - <episode_title>):<episode_title>( - <extra_info>):<extra_info>";
            else
                return "<title>";
        }

        void CPreferences::setTreatAsTVShowByDefault( bool value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            return settings.setValue( "TreatAsTVShowByDefault", value );
        }

        bool CPreferences::getTreatAsTVShowByDefault() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            return settings.value( "TreatAsTVShowByDefault", false ).toBool();
        }

        void CPreferences::setExactMatchesOnly( bool value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            return settings.setValue( "ExactMatchesOnly", value );
        }

        bool CPreferences::getExactMatchesOnly() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            return settings.value( "ExactMatchesOnly", true ).toBool();
        }

        void CPreferences::setDirectories( const QStringList & dir )
        {
            QSettings settings;
            settings.setValue( "Directories", dir );
        }

        QStringList CPreferences::getDirectories() const
        {
            QSettings settings;
            auto retVal = settings.value( "Directories", QStringList() ).toStringList();
            retVal.removeDuplicates();
            return retVal;
        }

        void CPreferences::setFileNames( const QStringList & dir )
        {
            QSettings settings;
            settings.setValue( "FileNames", dir );
        }

        QStringList CPreferences::getFileNames() const
        {
            QSettings settings;
            auto retVal = settings.value( "FileNames", QStringList() ).toStringList();
            retVal.removeDuplicates();
            return retVal;
        }

        void CPreferences::setTVOutFilePattern( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForTV" );
            settings.setValue( "OutFilePattern", value );
        }

        QString CPreferences::getTVOutFilePattern() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForTV" );

            return settings.value( "OutFilePattern", getDefaultOutFilePattern( true ) ).toString();
        }

        void CPreferences::setTVOutDirPattern( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForTV" );
            settings.setValue( "OutDirPattern", value );
        }

        QString CPreferences::getTVOutDirPattern() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForTV" );

            return settings.value( "OutDirPattern", getDefaultOutDirPattern( true ) ).toString();
        }

        void CPreferences::setMovieOutFilePattern( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForMovies" );
            settings.setValue( "OutFilePattern", value );
        }

        QString CPreferences::getMovieOutFilePattern() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForMovies" );

            return settings.value( "OutFilePattern", getDefaultOutFilePattern( false ) ).toString();
        }

        void CPreferences::setMovieOutDirPattern( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForMovies" );
            settings.setValue( "OutDirPattern", value );
        }

        QString CPreferences::getMovieOutDirPattern() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.beginGroup( "ForMovies" );

            return settings.value( "OutDirPattern", getDefaultOutDirPattern( false ) ).toString();
        }

        bool CPreferences::containsValue( const QString & value, const QStringList & values ) const
        {
            for ( auto && ii : values )
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

        bool CPreferences::pathMatches( const QFileInfo & fileInfo, const QStringList & values ) const
        {
            auto fn = fileInfo.fileName().toLower();
            if ( fn.endsWith( "-ignore" ) )
                return true;

            auto pathName = fileInfo.fileName();

#ifdef Q_OS_WINDOWS
            pathName = pathName.toLower();
#endif

            return containsValue( pathName, values );
        }

        bool CPreferences::isIgnoredPath( const QFileInfo & fileInfo ) const
        {
            return pathMatches( fileInfo, getIgnoredPaths() );
        }

        bool CPreferences::isSkippedPath( const QFileInfo & fileInfo ) const
        {
            return pathMatches( fileInfo, getSkippedPaths() );
        }

        void CPreferences::setSkippedPaths( const QStringList & values )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            QStringList realValues = values;
#ifdef Q_OS_WINDOWS
            for ( auto && ii : realValues )
                ii = ii.toLower();
#endif
            settings.setValue( "SkippedDirs", realValues );
        }

        QStringList CPreferences::getSkippedPaths() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            static auto defaultValues = QStringList( { "#recycle", "#recycler", "extras" } );
            return settings.value( "SkippedDirs", defaultValues ).toStringList();
        }

        void CPreferences::setIgnoredPaths( const QStringList & values )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            QStringList realValues = values;
#ifdef Q_OS_WINDOWS
            for ( auto && ii : realValues )
                ii = ii.toLower();
#endif
            settings.setValue( "IgnoredFileNames", realValues );
        }

        QStringList CPreferences::getIgnoredPaths() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            static auto defaultValues = QStringList( { "sub", "subs", "season \\d+" } );
            return settings.value( "IgnoredFileNames", defaultValues ).toStringList();
        }

        void CPreferences::setPathsToDelete( const QStringList & values )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            QStringList realValues = values;
            settings.setValue( "PathsToDelete", realValues );
        }

        QStringList CPreferences::getPathsToDelete() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            static auto defaultValues = QStringList( { "*.txt", "*.exe", "*.nfo" } );
            return settings.value( "PathsToDelete", defaultValues ).toStringList();
        }

        void CPreferences::setDeleteKnownPaths( bool value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.setValue( "DeleteKnownPaths", value );
        }

        bool CPreferences::isPathToDelete( const QString & path ) const
        {
            if ( !deleteKnownPaths() )
                return false;

            auto fn = QFileInfo( path ).fileName();
            auto toDelete = getPathsToDelete();
            for ( auto && ii : toDelete )
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

        bool CPreferences::deleteKnownPaths() const
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            return settings.value( "DeleteKnownPaths", true ).toBool();
        }

        void CPreferences::setMediaExtensions( const QString &value )
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue( "MediaExtensions", value );
        }

        void CPreferences::setMediaExtensions( const QStringList &value )
        {
            setMediaExtensions( value.join( ";" ) );
        }

        QStringList  CPreferences::getMediaExtensions() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value( "MediaExtensions", QString( "*.mkv;*.mp4;*.avi;*.mov;*.wmv;*.mpg;*.mpg2" ) ).toString().toLower().split( ";" );
        }

        QStringList  CPreferences::getNonMKVMediaExtensions() const
        {
            auto retVal = getMediaExtensions();
            retVal.removeAll( "*.mkv" );
            return retVal;
        }

        void CPreferences::setSubtitleExtensions( const QString &value )
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue( "SubtitleExtensions", value );
        }

        void CPreferences::setSubtitleExtensions( const QStringList &value )
        {
            setSubtitleExtensions( value.join( ";" ) );
        }

        QStringList  CPreferences::getSubtitleExtensions() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value( "SubtitleExtensions", QString( "*.idx;*.sub;*.srt" ) ).toString().split( ";" );
        }

        void CPreferences::addKnownStrings(const QStringList & value)
        {
            auto knownWords = getKnownStrings() << value;
            setKnownStrings(knownWords);
        }

        void CPreferences::setKnownStrings( const QStringList &value )
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue( "KnownStrings", value );
        }

        QStringList CPreferences::getKnownStrings() const
        {
            auto knownStrings =
                QStringList()
                << "2160p"
                << "1080p"
                << "720p"
                //<< "10bit"
                << "Amazon"
                << "DDP5.1"
                << "DL.DD+"
                << "DL.DD"
                << "DL.AAC2.0.AVC"
                << "AAC2.0.AVC"
                << "AAC2.0"
                << "AVC"
                << "AMZN"
                << "WebRip"
                << "WEB-DL"
                << "WEB"
                << "SDR"
                << "Atmos"
                << "-RUMOUR"
                << "-PECULATE"
                << "2.0.h.264"
                << "2.0.h.265"
                << "h.264"
                << "h.265"
                << "x264"
                << "x265"
                << "h264"
                << "h265"
                << "rarbg"
                << "BluRay"
                << "ion10"
                << "LiMiTED"
                << "DVDRip"
                << "XviDTLF"
                << "TrollHD"
                << "Troll"
                << "nogrp"
                << "-CM"
                << "NF"
                ;

            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value( "KnownStrings", knownStrings ).toStringList();
        }

        void CPreferences::setKnownExtendedStrings(const QStringList & value)
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue("KnownExtendedStrings", value);
        }

        QStringList CPreferences::getKnownExtendedStrings() const
        {
            auto knownStrings =
                QStringList()
                << "Extended"
                << "Directors Cut"
                << "Director's Cut"
                << "Director"
                ;

            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value("KnownExtendedStrings", knownStrings).toStringList();
        }

        void CPreferences::setKnownAbbreviations( const QList<QPair<QString, QString >> & value )
        {
            QVariantMap map;
            for ( auto && ii : value )
            {
                map[ii.first] = ii.second;
            }
            return setKnownAbbreviations( map );
        }

        void CPreferences::setKnownAbbreviations( const QVariantMap & value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.setValue( "KnownAbbreviations", value );
        }

        QVariantMap CPreferences::getKnownAbbreviations() const
        {
            QVariantMap knownAbbreviations( { { "Dont", "Don't" } }  );
            QSettings settings;
            settings.beginGroup( "Transform" );
            return settings.value( "KnownAbbreviations", knownAbbreviations ).toMap();
        };





        void CPreferences::setMKVMergeEXE( const QString &value )
        {
            QSettings settings;
            settings.beginGroup("ExternalTools");
            settings.setValue( "MKVMergeEXE", value );
        }

        QString CPreferences::getMKVMergeEXE() const
        {
            QSettings settings;
            settings.beginGroup("ExternalTools");
            auto retVal = settings.value( "MKVMergeEXE", QString( "C:/Program Files/MKVToolNix/mkvmerge.exe" ) ).toString();

            auto fi = QFileInfo( retVal );
            bool aOK = !retVal.isEmpty() && fi.isExecutable();
            return aOK ? retVal : QString();
        }
 
        void CPreferences::setMKVPropEditEXE(const QString & value)
        {
            QSettings settings;
            settings.beginGroup("ExternalTools");
            settings.setValue("MKVPropEditEXE", value);
        }

        QString CPreferences::getMKVPropEditEXE() const
        {
            QSettings settings;
            settings.beginGroup("ExternalTools");
            auto retVal = settings.value("MKVPropEditEXE", QString("C:/Program Files/MKVToolNix/mkvpropedit.exe")).toString();

            auto fi = QFileInfo(retVal);
            bool aOK = !retVal.isEmpty() && fi.isExecutable();
            return aOK ? retVal : QString();
        }

        void CPreferences::setFFMpegEXE( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "ExternalTools" );
            settings.setValue( "FFMpegEXE", value );
        }

        QString CPreferences::getFFMpegEXE() const
        {
            QSettings settings;
            settings.beginGroup( "ExternalTools" );
            auto retVal = settings.value( "FFMpegEXE", QString() ).toString();

            auto fi = QFileInfo( retVal );
            bool aOK = !retVal.isEmpty() && fi.isExecutable();
            return aOK ? retVal : QString();
        }

        void CPreferences::setFFProbeEXE( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "ExternalTools" );
            settings.setValue( "FFProbeEXE", value );
        }

        QString CPreferences::getFFProbeEXE() const
        {
            QSettings settings;
            settings.beginGroup( "ExternalTools" );
            auto retVal = settings.value( "FFProbeEXE", QString() ).toString();

            auto fi = QFileInfo( retVal );
            bool aOK = !retVal.isEmpty() && fi.isExecutable();
            return aOK ? retVal : QString();
        }
        void CPreferences::setBIFPlayerSpeedMultiplier( int interval )
        {
            QSettings settings;
            settings.beginGroup("BIFViewer");
            settings.setValue( "PlayerSpeedMultiplier", interval );
        }

        int CPreferences::bifPlayerSpeedMultiplier() const
        {
            QSettings settings;
            settings.beginGroup("BIFViewer");
            return settings.value("PlayerSpeedMultiplier", 200).toInt();
        }

        void CPreferences::setBIFNumFramesToSkip( int interval )
        {
            QSettings settings;
            settings.beginGroup("BIFViewer");
            settings.setValue("NumFramesToSkip", interval);
        }

        int CPreferences::bifNumFramesToSkip() const
        {
            QSettings settings;
            settings.beginGroup("BIFViewer");
            return settings.value( "NumFramesToSkip", 5 ).toInt();
        }

        void CPreferences::setBIFLoopCount(int loopCount)
        {
            QSettings settings;
            settings.beginGroup("BIFViewer");
            settings.setValue("LoopCount", loopCount);
        }

        int CPreferences::bifLoopCount() const
        {
            QSettings settings;
            settings.beginGroup("BIFViewer");
            auto retVal = settings.value("LoopCount", -1).toInt();
            if (retVal == 0)
                retVal = -1;
            return retVal;
        }

        void CPreferences::setGIFFlipImage( bool flipImage )
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            settings.setValue( "FlipImage", flipImage );
        }
        
        bool CPreferences::gifFlipImage() const
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            return settings.value( "FlipImage", false ).toBool();
        }

        void CPreferences::setGIFDitherImage( bool ditherImage )
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            settings.setValue( "DitherImage", ditherImage );
        }
        
        bool CPreferences::gifDitherImage() const
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            return settings.value( "DitherImage", true ).toBool();
        }

        void CPreferences::setGIFLoopCount( int loopCount )
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            settings.setValue( "LoopCount", loopCount );
        }

        int CPreferences::gifLoopCount() const
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            return settings.value( "LoopCount", true ).toInt();
        }

        void CPreferences::setGIFStartFrame( int startFrame )
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            settings.setValue( "StartFrame", startFrame );
        }

        int CPreferences::gifStartFrame() const
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            return settings.value( "StartFrame", true ).toInt();
        }

        void CPreferences::setGIFEndFrame( int endFrame )
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            settings.setValue( "EndFrame", endFrame );
        }
        
        int CPreferences::gifEndFrame() const
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            return settings.value( "EndFrame", true ).toInt();
        }

        void CPreferences::setGIFDelay( int delay )
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            settings.setValue( "Delay", delay );
        }

        int CPreferences::gifDelay() const
        {
            QSettings settings;
            settings.beginGroup( "GIFWriter" );
            return settings.value( "Delay", true ).toInt();
        }

        bool CPreferences::isMediaFile( const QFileInfo &fi ) const
        {
            auto suffixes = getMediaExtensions();
            for ( auto && ii : suffixes )
            {
                auto pos = ii.lastIndexOf( '.' );
                ii = ii.mid( pos + 1 );
            }
            auto extensions = NSABUtils::hashFromList( suffixes );

            auto suffix = fi.suffix();
            return (extensions.find( suffix ) != extensions.end());
        }

        // only return true for X_Lang.srt files or subs directories
        bool CPreferences::isSubtitleFile( const QFileInfo & fi, bool * isLangFileFormat ) const
        {
            if ( isLangFileFormat )
                *isLangFileFormat = false;

            auto exts = CPreferences::instance()->getSubtitleExtensions();
            for ( auto && ii : exts )
            {
                auto pos = ii.lastIndexOf( '.' );
                ii = ii.mid( pos + 1 );
            }
            auto extensions = NSABUtils::hashFromList( exts );

            auto suffix = fi.suffix();
            if ( extensions.find( suffix ) == extensions.end() )
                return false;

            if ( isLangFileFormat )
            {
                *isLangFileFormat = SLanguageInfo::isLangFileFormat( fi );
            }
            return true;
        }
   }
}