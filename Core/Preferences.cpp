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
                return "<title>{ (<year>)}:<year>/Season <season>";
            else
                return "<title>{ (<year>)}:<year>{ [tmdbid=<tmdbid>]}:<tmdbid>{ - <extra_info>}:<extra_info>";
        }

        QString CPreferences::getDefaultOutFilePattern( bool forTV ) const
        {
            if ( forTV )
                return "<title> - S<season>E<episode>{ - <episode_title>}:<episode_title>{ - <extra_info>}:<extra_info>";
            else
                return "<title>{ - <extra_info>}:<extra_info>";
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

        QColor CPreferences::getColorForStatus(EItemStatus status, bool background ) const
        {
            QSettings settings;
            settings.beginGroup("Colors");
            QColor defaultColor = QColor();
            switch (status)
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
            return settings.value( QString("%1-%2ground").arg( toString( status ) ).arg( background ? "Back" : "Fore" ), defaultColor ).value< QColor >();
        }
            
        void CPreferences::setColorForStatus(EItemStatus status, bool background, const QColor & value)
        {
            QSettings settings;
            settings.beginGroup("Colors");
            settings.setValue(QString("%1-%2ground").arg(toString(status)).arg(background ? "Back" : "Fore"), value );
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
            static auto defaultValues = QStringList( 
                { 
                    "#recycle", 
                    "#recycler", 
                    "extra(s)?",
                    "trailer(s)?",
                    "deleted scene(s)?",
                    "interview(s)?",
                    "featurette(s)?",
                    "sample(s)?"
                } 
            );
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

        bool CPreferences::getVerifyMediaTags() const
        {
            QSettings settings;
            settings.beginGroup("Tag");
            return settings.value("VerifyMediaTags", true).toBool();
        }

        void CPreferences::setVerifyMediaTags(bool value)
        {
            QSettings settings;
            settings.beginGroup("Tag");
            settings.setValue("VerifyMediaTags", value);
        }

        std::list< std::pair< QString, bool > > CPreferences::getTagsToShow() const
        {
            QSettings settings;
            settings.beginGroup( "Tag" );

            std::list< std::pair< QString, bool > > retVal =
            {
                 { "Title", true }
                ,{ "Length", true }
                ,{ "Media Date", true }
                ,{ "Comment", true }
                ,{ "BPM", true }
                ,{ "Artist", true }
                ,{ "Composer", true }
                ,{ "Genre", true }
                ,{ "Track", true }
                ,{ "Album", true }
                ,{ "Album Artist", true }
                ,{ "Discnumber", true }
            };

            if ( !settings.contains( "EnabledTags" ) )
                return retVal;

            auto enabledTags = settings.value( "EnabledTags" ).toStringList();
            for ( auto && jj : retVal )
                jj.second = false;

            for ( auto && ii : enabledTags )
            {
                for ( auto && jj : retVal )
                {
                    if ( jj.first == ii )
                        jj.second = true;
                }
            }
            return retVal;
        }

        QStringList CPreferences::getEnabledTags() const
        {
            auto allTags = getTagsToShow();
            QStringList retVal;
            for ( auto && ii : allTags )
            {
                if ( ii.second )
                    retVal << ii.first;
            }
            return retVal;
        }

        void CPreferences::setEnabledTags( const QStringList & values )
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            settings.setValue( "EnabledTags", values );
        }

        QString replaceFileInfo( const QFileInfo & fi, const QDate & date, const QString & expr )
        {
            QString retVal = expr;

            retVal = retVal.replace( "<EMPTY>", R"(^$)" );
            retVal = retVal.replace( "<filename>", fi.fileName() );
            retVal = retVal.replace( "<basename>", fi.completeBaseName() );
            retVal = retVal.replace( "<extension>", fi.suffix() );

            if ( date.isValid() )
            {
                retVal = retVal.replace( "<year>", date.toString( "(yy|yyyy)" ) );
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
            settings.beginGroup("Tag");
            return settings.value("VerifyMediaTitle", true).toBool();
        }
        
        void CPreferences::setVerifyMediaTitle(bool value)
        {
            QSettings settings;
            settings.beginGroup("Tag");
            settings.setValue("VerifyMediaTitle", value);
        }

        QRegularExpression CPreferences::getVerifyMediaTitleExpr( const QFileInfo & fi, const QDate & date ) const
        {
            auto regExStr = replaceFileInfo( fi, date, getVerifyMediaTitleExpr() );
            return QRegularExpression( regExStr );
        }

        QString CPreferences::getVerifyMediaTitleExpr() const
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            return settings.value( "VerifyMediaTitleExpr", "<basename>" ).toString();
        }

        void CPreferences::setVerifyMediaTitleExpr( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            settings.setValue( "VerifyMediaTitleExpr", value );
        }

        bool CPreferences::getVerifyMediaDate() const
        {
            QSettings settings;
            settings.beginGroup("Tag");
            return settings.value("VerifyMediaDate", true).toBool();
        }
        
        void CPreferences::setVerifyMediaDate(bool value)
        {
            QSettings settings;
            settings.beginGroup("Tag");
            settings.setValue("VerifyMediaDate", value);
        }

        QString CPreferences::getVerifyMediaDateExpr() const
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            return settings.value( "VerifyMediaDateExpr", R"(<year>|<month>[-\/]<year>|<month>[-\/]<day>[-\/]<year>)" ).toString();
        }

        void CPreferences::setVerifyMediaDateExpr( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            settings.setValue( "VerifyMediaDateExpr", value );
        }

        QRegularExpression CPreferences::getVerifyMediaDateExpr( const QFileInfo & fi, const QDate & date ) const
        {
            auto regExStr = replaceFileInfo( fi, date, getVerifyMediaDateExpr() );
            return QRegularExpression( regExStr );
        }

        bool CPreferences::getVerifyMediaComment() const
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            return settings.value( "VerifyMediaComment", true ).toBool();
        }

        void CPreferences::setVerifyMediaComment( bool value )
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            settings.setValue( "VerifyMediaComment", value );
        }

        QString CPreferences::getVerifyMediaCommentExpr() const
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            return settings.value( "VerifyMediaCommentExpr", R"(<EMPTY>)" ).toString();
        }

        void CPreferences::setVerifyMediaCommentExpr( const QString & value )
        {
            QSettings settings;
            settings.beginGroup( "Tag" );
            settings.setValue( "VerifyMediaCommentExpr", value );
        }

        QRegularExpression CPreferences::getVerifyMediaCommentExpr( const QFileInfo & fi, const QDate & date ) const
        {
            auto regExStr = replaceFileInfo( fi, date, getVerifyMediaCommentExpr() );
            return QRegularExpression( regExStr );
        }

        void CPreferences::setDeleteCustom(bool value)
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue("DeleteCustom", value);
        }

        bool CPreferences::deleteCustom() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value("DeleteCustom", true).toBool();
        }

        void CPreferences::setDeleteEXE( bool value )
        {
            QSettings settings;
            settings.beginGroup( "Transform" );
            settings.setValue( "DeleteEXE", value );
        }

        bool CPreferences::deleteEXE() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value("DeleteEXE", true).toBool();
        }

        void CPreferences::setDeleteNFO(bool value)
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue("DeleteNFO", value);
        }

        bool CPreferences::deleteNFO() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value("DeleteNFO", true).toBool();
        }

        void CPreferences::setDeleteBAK(bool value)
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue("DeleteBAK", value);
        }

        bool CPreferences::deleteBAK() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value("DeleteBAK", true).toBool();
        }

        void CPreferences::setDeleteTXT(bool value)
        {
            QSettings settings;
            settings.beginGroup("Transform");
            settings.setValue("DeleteTXT", value);
        }

        bool CPreferences::deleteTXT() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value("DeleteTXT", true).toBool();
        }

        void CPreferences::setCustomPathsToDelete(const QStringList & values)
        {
            QSettings settings;
            settings.beginGroup("Transform");
            const QStringList& realValues = values;
            settings.setValue("CustomToDelete", realValues);
        }

        QStringList CPreferences::getCustomPathsToDelete() const
        {
            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value("CustomToDelete", QStringList()).toStringList();
        }

        QStringList CPreferences::getExtensionsToDelete() const
        {
            QStringList retVal;
            if (deleteEXE())
                retVal << "*.exe";
            if (deleteBAK())
                retVal << "*.bak";
            if (deleteNFO())
                retVal << "*.nfo";
            if (deleteTXT())
                retVal << "*.txt";
            if (deleteCustom())
                retVal << getCustomPathsToDelete();
            return retVal;
        }

        bool CPreferences::isPathToDelete( const QString & path ) const
        {
            auto fn = QFileInfo( path ).fileName();
            auto toDelete = getExtensionsToDelete();
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
            auto knownWords = getKnownStrings();
            for ( auto && ii : value )
            {
                if ( !knownWords.contains( ii ) )
                    knownWords << value;
            }
            setKnownStrings(knownWords);
        }

        void CPreferences::setKnownStrings( const QStringList &value )
        {
            QSettings settings;
            settings.beginGroup("Transform");
            QSet< QString > tmp;
            QStringList realValues;
            for ( auto && ii : value )
            {
                if ( !tmp.contains( ii ) )
                    realValues << ii;
            }

            settings.setValue( "KnownStrings", realValues );
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
                << "RUMOUR"
                << "PECULATE"
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
                << "CM"
                << "NF"
                << "REMASTERED"
                << "PROPER"
                << "DC"
                << "AAC"
                << "DSNP"
                << "10bit"
                << "HDR"
                << "DTS-HD"
                << "MA"
                << "5.1"
                << "SWTYBLZ"
                << "YIFY"
                << "ATVP"
                << "NAISU"
                << "CUPCAKES"
                << "MoviesFD"
                << "SPARKS"
                << "DD5.1"
                << "REMUX"
                << "BRRip"
                << "PLEW"
                << "Japhson"
                << "DTS"
                << "FGT"
                << "CCBB"
                << "IAMABLE"
                << "UHD"
                << "TrueHD"
                << "7.1"
                << "TERMiNAL"
                ;

            QSettings settings;
            settings.beginGroup("Transform");
            return settings.value( "KnownStrings", knownStrings ).toStringList();
        }

        QStringList CPreferences::getKnownStringRegExs() const
        {
            auto strings = getKnownStrings();
            QStringList nonRegExs;
            QStringList retVal;
            for ( auto && ii : strings )
            {
                bool isRegEx =
                    (ii.indexOf( "\\" ) != -1)
                    || (ii.indexOf( "?" ) != -1)
                    || (ii.indexOf( "{" ) != -1)
                    || (ii.indexOf( "}" ) != -1)
                    ;

                if ( isRegEx )
                    retVal << QString( "(?<word>" + ii + ")" );
                else
                    nonRegExs << QRegularExpression::escape( ii );
            }
            auto primRegEx = R"(((?<prefix>\[|\()|\W)(?<word>)" + nonRegExs.join("|") + R"()((?<suffix>\]|\))|\W|$))";
            retVal << primRegEx;

            return retVal;
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
                << "Unrated"
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
            QVariantMap knownAbbreviations( 
                { 
                    { "Dont", "Don't" } 
                    ,{ "NY", "New York" }
                }
            );
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
            if ( !fi.isFile() )
                return false;

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

        QString toString(EItemStatus status)
        {
            switch (status)
            {
            case EItemStatus::eOK: return "OK";
            case EItemStatus::eWarning: return "Warning";
            case EItemStatus::eError: return "Error";
            }
            return {};
        }
   }
}