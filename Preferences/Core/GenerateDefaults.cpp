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
        namespace NCore
        {
            QString getIndent( int indent )
            {
                return QString( 4 * indent, QChar( ' ' ) );
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
                function << QString( "QString CPreferences::%1( bool %2 ) const" ).arg( funcName ).arg( boolVariable ) << "{" << getIndent( 1 ) + QString( "if ( %2 )" ).arg( boolVariable ) << getIndent( 2 ) + QString( "return %1;" ).arg( toString( trueValue ) ) << getIndent( 1 ) + QString( "else" ) << getIndent( 2 ) + QString( "return %1;" ).arg( toString( falseValue ) ) << "}";
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
                function   //
                    << QString( "NSABUtils::TFormatMap CPreferences::%1() const" ).arg( funcName )   //
                    << "{";

                int indent = 1;
                function   //
                    << getIndent( indent++ ) + QString( "static auto defaultValue = NSABUtils::TFormatMap(" )   //
                    << getIndent( indent++ ) + "{";
                ;

                auto first = true;
                for ( auto &&ii : { NSABUtils::EFormatType::eUnknown, NSABUtils::EFormatType::eAudio, NSABUtils::EFormatType::eImage, NSABUtils::EFormatType::eSubtitle, NSABUtils::EFormatType::eVideo } )
                {
                    auto pos = value.find( ii );
                    if ( pos == value.end() )
                        continue;

                    function << getIndent( indent++ ) + ( first ? " " : "," ) + "{";
                    function << getIndent( indent ) + toString( ( *pos ).first ) + ", std::unordered_map< QString, QStringList >";
                    function << getIndent( indent++ ) + " ( {";

                    auto innerMap = std::map< QString, QStringList >( ( *pos ).second.begin(), ( *pos ).second.end() );
                    bool innerFirst = true;
                    for ( auto &&jj : innerMap )
                    {
                        function << getIndent( indent ) + ( innerFirst ? " " : "," ) + "{ " + toString( jj.first ) + ", " + toString( jj.second ) + " } //";   //
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

            void replaceText( const QString &txt, QStringList &curr, const QString &funcName, const NSABUtils::TCodecToEncoderDecoderMap &value )
            {
                QStringList function;
                function   //
                    << QString( "NSABUtils::TCodecToEncoderDecoderMap CPreferences::%1() const" ).arg( funcName )   //
                    << "{";

                int indent = 1;
                function   //
                    << getIndent( indent++ ) + QString( "static auto defaultValue = NSABUtils::TCodecToEncoderDecoderMap(" )   //
                    << getIndent( indent++ ) + "{";
                ;

                auto first = true;
                for ( auto &&ii : { NSABUtils::EFormatType::eUnknown, NSABUtils::EFormatType::eAudio, NSABUtils::EFormatType::eImage, NSABUtils::EFormatType::eSubtitle, NSABUtils::EFormatType::eVideo } )
                {
                    auto pos = value.find( ii );
                    if ( pos == value.end() )
                        continue;

                    function << getIndent( indent++ ) + ( first ? " " : "," ) + "{";
                    function << getIndent( indent ) + toString( (*pos).first ) + ", std::unordered_multimap< QString, QString >";
                    function << getIndent( indent++ ) + " ( {";

                    auto innerMap = std::multimap< QString, QString >( ( *pos ).second.begin(), ( *pos ).second.end() );
                    bool innerFirst = true;
                    for ( auto &&jj : innerMap )
                    {
                        function << getIndent( indent ) + ( innerFirst ? " " : "," ) + "{ " + toString( jj.first ) + ", " + toString( jj.second ) + " } //";   //
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
            auto replaceText( const QString &txt, QStringList &curr, const QString &funcName, T value ) -> typename std::enable_if< !std::is_same< bool, T >::value && !std::is_enum< T >::value && !std::is_same< QString, T >::value && !std::is_same< char *, T >::value, void >::type
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
            auto replaceText( const QString &txt, const QString &returnType, QStringList &curr, const QString &funcName, T value ) -> typename std::enable_if< std::is_enum< T >::value, void >::type
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
                function << QString( "%3 CPreferences::%1( bool %2 ) const" ).arg( funcName ).arg( boolVariable ).arg( retValType ) << "{" << getIndent( 1 ) + QString( "if ( %2 )" ).arg( boolVariable ) << getIndent( 1 ) + "{" << toString( retValType, trueValue, asString, 2 ) << getIndent( 1 ) + "}" << getIndent( 1 ) + "else" << getIndent( 1 ) + "{" << toString( retValType, falseValue, asString, 2 ) << getIndent( 1 ) + "}"
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

            QString compareValues( const QString &title, const QStringList &defaultValues, const QStringList &currValues )
            {
                if ( defaultValues == currValues )
                    return {};

                //auto defaultSortedUnique = std::set< QString >( { defaultValues.begin(), defaultValues.end() } );
                auto currSortedUnique = std::set< QString >( { currValues.begin(), currValues.end() } );

                QStringList items;
                for ( auto &&curr : defaultValues )
                {
                    if ( currSortedUnique.find( curr ) == currSortedUnique.end() )
                    {
                        items << QString( "%1 currently missing" ).arg( curr );
                    }
                }

                auto defaultSortedUnique = std::set< QString >( { defaultValues.begin(), defaultValues.end() } );
                for ( auto &&curr : currValues )
                {
                    if ( defaultSortedUnique.find( curr ) == defaultSortedUnique.end() )
                    {
                        items << QString( "%1 not in defaults" ).arg( curr );
                    }
                }

                //if ( items.count() > 15 )
                //    items.clear();

                //int origII = ii;
                //for ( int ii = origII; ii < defaultValues.count(); ++ii )
                //{
                //    items << QString( "%1 currently missing" ).arg( defaultValues[ ii ] );
                //}

                //for ( int ii = origII; ii < currValues.count(); ++ii )
                //{
                //    items << QString( "%1 not in defaults" ).arg( currValues[ ii ] );
                //}

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

            QStringList variantMapToStringList( const QVariantMap &data )
            {
                QStringList retVal;
                for ( auto &&ii = data.cbegin(); ii != data.cend(); ++ii )
                    retVal << QString( "%1=%2" ).arg( ii.key() ).arg( ii.value().toString() );
                return retVal;
            }

            QString compareValues( const QString &title, const QVariantMap &defaultValues, const QVariantMap &currValues )
            {
                return compareValues( title, variantMapToStringList( defaultValues ), variantMapToStringList( currValues ) );
            }

            QString compareValues( const QString &title, const QString &defaultValues, const QString &currValues )
            {
                return compareValues( title, QStringList() << defaultValues, QStringList() << currValues );
            }

            QString compareValues( const QString &title, bool defaultValue, bool currValue )
            {
                if ( defaultValue == currValue )
                    return {};

                auto retVal = QString( "<li>%1\n<ul>Default is %2, current setting is %3</ul>\n</li>\n" ).arg( title ).arg( defaultValue ? "true" : "false" ).arg( currValue ? "true" : "false" );
                return retVal;
            }

            QStringList CPreferences::getDefaultFile() const
            {
                auto retVal = QStringList() << R"(// clang-format off)"
                                            << R"(// The MIT License( MIT ))"
                                            << R"(//)"
                                            << R"(// Copyright( c ) 2020-2023 Scott Aron Bloom)"
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
                                            << R"(namespace NMediaManager)" << getIndent( 0 ) + R"({)" << getIndent( 1 ) + R"(namespace NPreferences)" << getIndent( 1 ) + R"({)" << getIndent( 2 ) + R"(namespace NCore)" << getIndent( 2 ) + R"({)"
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

                                            << "%DEFAULT_FORCE_MEDIA_CONTAINER%"
                                            << R"()"
                                            << "%DEFAULT_MEDIA_CONTAINER_NAME%"
                                            << R"()"
                                            << "%DEFAULT_TRANSCODE_AUDIO%"
                                            << R"()"
                                            << "%DEFAULT_ONLY_TRANSCODE_AUDIO_ON_FORMAT_CHANGE%"
                                            << R"()"
                                            << "%DEFAULT_ADD_AAC%"
                                            << R"()"
                                            << "%DEFAULT_TRANSCODE_TO_AUDIO_CODEC%"
                                            << R"()"
                                            << "%DEFAULT_TRANSCODE_VIDEO%"
                                            << R"()"
                                            << "%DEFAULT_ONLY_TRANSCODE_VIDEO_ON_FORMAT_OR_BITRATE_CHANGE%"
                                            << R"()"
                                            << "%DEFAULT_TRANSCODE_TO_VIDEO_CODEC%"
                                            << R"()"
                                            << "%DEFAULT_LOSSLESS_TRANSCODING%"
                                            << R"()"
                                            << "%DEFAULT_GENERATE_LOW_BITRATE_VIDEO%"
                                            << R"()"
                                            << "%DEFAULT_BITRATE_THRESHOLD_PERCENTAGE%"
                                            << R"()"
                                            << "%DEFAULT_RESOLUTION_THRESHOLD_PERCENTAGE%"
                                            << R"()"
                                            << "%DEFAULT_GENERATE_NON_4k_VIDEO%"
                                            << R"()"
                                            << "%DEFAULT_USE_CRF%"
                                            << R"()"
                                            << "%DEFAULT_CRF%"
                                            << R"()"
                                            << "%DEFAULT_USE_TARGET_BITRATE%"
                                            << R"()"
                                            << "%DEFAULT_TARGET_4K_BITRATE%"
                                            << R"()"
                                            << "%DEFAULT_TARGET_HD_BITRATE%"
                                            << R"()"
                                            << "%DEFAULT_TARGET_SUBHD_BITRATE%"
                                            << R"()"
                                            << "%DEFAULT_GREATER_THAN_4K_DIVISOR%"
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
                                            << R"()"

                                            << "%MEDIA_FORMAT_ENCODER%"
                                            << R"()"
                                            << "%MEDIA_FORMAT_ENCODER_EXTENSION_MAP%"
                                            << R"()"
                                            << "%MEDIA_FORMAT_DECODER%"
                                            << R"()"
                                            << "%MEDIA_FORMAT_DECODER_EXTENSION_MAP%"
                                            << R"()"

                                            << "%AVAILABLE_VIDEO_ENCODING_CODECS%"
                                            << R"()"
                                            << "%AVAILABLE_VIDEO_DECODING_CODECS%"
                                            << R"()"
                                            << "%AVAILABLE_AUDIO_ENCODING_CODECS%"
                                            << R"()"
                                            << "%AVAILABLE_AUDIO_DECODING_CODECS%"
                                            << R"()"
                                            << "%AVAILABLE_SUBTITLE_ENCODING_CODECS%"
                                            << R"()"
                                            << "%AVAILABLE_SUBTITLE_DECODING_CODECS%"
                                            << R"()"

                                            << "%AVAILABLE_VIDEO_ENCODERS%"
                                            << R"()"
                                            << "%AVAILABLE_VIDEO_DECODERS%"
                                            << R"()"
                                            << "%AVAILABLE_AUDIO_ENCODERS%"
                                            << R"()"
                                            << "%AVAILABLE_AUDIO_DECODERS%"
                                            << R"()"
                                            << "%AVAILABLE_SUBTITLE_ENCODERS%"
                                            << R"()"
                                            << "%AVAILABLE_SUBTITLE_DECODERS%"
                                            << R"()"

                                            << "%CODEC_TO_ENCODER_MAP%"
                                            << R"()"
                                            << "%CODEC_TO_DECODER_MAP%"
                                            << R"()"

                                            << "%AVAILABLE_HW_ACCELS%"
                                            << R"()"

                                            << getIndent( 2 ) + R"(})" << getIndent( 1 ) + R"(})" << getIndent( 0 ) + R"(})";
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
                                           << compareValues( "Known Hyphenated", getDefaultKnownHyphenated(), getKnownHyphenated() )   //

                                           << compareValues( "Convert Media Format", getConvertMediaContainerDefault(), getConvertMediaContainer() )   //
                                           << compareValues( "Convert Media To Format", getConvertMediaToContainerDefault(), getConvertMediaToContainer() )   //

                                           << compareValues( "Transcode Audio", getTranscodeAudioDefault(), getTranscodeAudio() )   //
                                           << compareValues( "Transcode Audio on Format Change", getOnlyTranscodeAudioOnFormatChangeDefault(), getOnlyTranscodeAudioOnFormatChange() )   //
                                           << compareValues( "Transcode Audio Codec", getTranscodeToAudioCodecDefault(), getTranscodeToAudioCodec() )   //

                                           << compareValues( "Transcode Video", getTranscodeVideoDefault(), getTranscodeVideo() )   //
                                           << compareValues( "Transcode Video on Format or Bitrate Change", getOnlyTranscodeVideoOnFormatChangeDefault(), getOnlyTranscodeVideoOnFormatChange() )   //
                                           << compareValues( "Lossless Video Encoding", getLosslessEncodingDefault(), getLosslessEncoding() )   //

                                           << compareValues( "Generate Low Bitrate Video", getGenerateLowBitrateVideoDefault(), getGenerateLowBitrateVideo() )   //
                                           << compareValues( "Bitrate Threshold Percentage", getBitrateThresholdPercentageDefault(), getBitrateThresholdPercentage() )   //
                                           << compareValues( "Resolution Threshold Percentage", getResolutionThresholdPercentageDefault(), getResolutionThresholdPercentage() )   //
                                           << compareValues( "Generate non-4k Video", getGenerateNon4kVideoDefault(), getGenerateNon4kVideo() )   //

                                           << compareValues( "Use CRF", getUseCRFDefault(), getUseCRF() )   //
                                           << compareValues( "CRF", getCRFDefault(), getCRF() )   //
                                           << compareValues( "Use Target Bitrate", getUseTargetBitrateDefault(), getUseTargetBitrate() )   //
                                           << compareValues( "Target 4k Bitrate", getTarget4kBitrateDefault(), getTarget4kBitrate() )   //
                                           << compareValues( "Target HD Bitrate", getTargetHDBitrateDefault(), getTargetHDBitrate() )   //
                                           << compareValues( "Target Sub-HD Bitrate", getTargetSubHDBitrateDefault(), getTargetSubHDBitrate() )   //
                                           << compareValues( "Greater than 4k Divisor", getGreaterThan4kDivisorDefault(), getGreaterThan4kDivisor() )   //
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
                    copyDefaults = QMessageBox::information( parent, tr( "Default Preferences are the Same" ), tr( "The defaults are up to date.\nWould you like to copy the defaults to the Clipboard" ), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes;
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

                    replaceText( "%DEFAULT_FORCE_MEDIA_CONTAINER%", newFileText, "getConvertMediaContainerDefault", getConvertMediaContainer() );
                    replaceText( "%DEFAULT_MEDIA_CONTAINER_NAME%", newFileText, "getConvertMediaToContainerDefault", getConvertMediaToContainer() );
                    replaceText( "%DEFAULT_TRANSCODE_AUDIO%", newFileText, "getTranscodeAudioDefault", getTranscodeAudio() );
                    replaceText( "%DEFAULT_ONLY_TRANSCODE_AUDIO_ON_FORMAT_CHANGE%", newFileText, "getOnlyTranscodeAudioOnFormatChangeDefault", getOnlyTranscodeAudioOnFormatChange() );
                    replaceText( "%DEFAULT_ADD_AAC%", newFileText, "getAddAACAudioCodecDefault", getAddAACAudioCodec() );
                    replaceText( "%DEFAULT_TRANSCODE_TO_AUDIO_CODEC%", newFileText, "getTranscodeToAudioCodecDefault", getTranscodeToAudioCodec() );

                    replaceText( "%DEFAULT_TRANSCODE_VIDEO%", newFileText, "getTranscodeVideoDefault", getTranscodeVideo() );
                    replaceText( "%DEFAULT_ONLY_TRANSCODE_VIDEO_ON_FORMAT_OR_BITRATE_CHANGE%", newFileText, "getOnlyTranscodeVideoOnFormatChangeDefault", getOnlyTranscodeVideoOnFormatChange() );
                    replaceText( "%DEFAULT_TRANSCODE_TO_VIDEO_CODEC%", newFileText, "getTranscodeToVideoCodecDefault", getTranscodeToVideoCodec() );

                    replaceText( "%DEFAULT_GENERATE_LOW_BITRATE_VIDEO%", newFileText, "getGenerateLowBitrateVideoDefault", getGenerateLowBitrateVideo() );
                    replaceText( "%DEFAULT_BITRATE_THRESHOLD_PERCENTAGE%", newFileText, "getBitrateThresholdPercentageDefault", getBitrateThresholdPercentage() );
                    replaceText( "%DEFAULT_RESOLUTION_THRESHOLD_PERCENTAGE%", newFileText, "getResolutionThresholdPercentageDefault", getResolutionThresholdPercentage() );
                    replaceText( "%DEFAULT_GENERATE_NON_4k_VIDEO%", newFileText, "getGenerateNon4kVideoDefault", getGenerateNon4kVideo() );

                    replaceText( "%DEFAULT_USE_CRF%", newFileText, "getUseCRFDefault", getUseCRF() );
                    replaceText( "%DEFAULT_CRF%", newFileText, "getCRFDefault", getCRF() );
                    replaceText( "%DEFAULT_USE_TARGET_BITRATE%", newFileText, "getUseTargetBitrateDefault", getUseTargetBitrate() );
                    replaceText( "%DEFAULT_TARGET_4K_BITRATE%", newFileText, "getTarget4kBitrateDefault", getTarget4kBitrate() );
                    replaceText( "%DEFAULT_TARGET_HD_BITRATE%", newFileText, "getTargetHDBitrateDefault", getTargetHDBitrate() );
                    replaceText( "%DEFAULT_TARGET_SUBHD_BITRATE%", newFileText, "getTargetSubHDBitrateDefault", getTargetSubHDBitrate() );
                    replaceText( "%DEFAULT_GREATER_THAN_4K_DIVISOR%", newFileText, "getGreaterThan4kDivisorDefault", getGreaterThan4kDivisor() );

                    replaceText( "%DEFAULT_LOSSLESS_TRANSCODING%", newFileText, "getLosslessEncodingDefault", getLosslessEncoding() );
                    replaceText( "%DEFAULT_USE_PRESET%", newFileText, "getUsePresetDefault", getUsePreset() );
                    replaceText( "%DEFAULT_PRESET%", "ETranscodePreset", newFileText, "getPresetDefault", getPreset() );
                    replaceText( "%DEFAULT_USE_TUNE%", newFileText, "getUseTuneDefault", getUseTune() );
                    replaceText( "%DEFAULT_TUNE%", "ETranscodeTune", newFileText, "getTuneDefault", getTune() );
                    replaceText( "%DEFAULT_USE_PROFILE%", newFileText, "getUseProfileDefault", getUseProfile() );
                    replaceText( "%DEFAULT_PROFILE%", "ETranscodeProfile", newFileText, "getProfileDefault", getProfile() );

                    replaceText( "%MEDIA_FORMAT_ENCODER%", newFileText, "availableMediaEncoderFormatsStatic", "verbose", availableEncoderMediaFormats( true ), availableEncoderMediaFormats( false ) );
                    replaceText( "%MEDIA_FORMAT_ENCODER_EXTENSION_MAP%", newFileText, "getEncoderFormatExtensionsMapStatic", getEncoderFormatExtensionsMap() );
                    replaceText( "%MEDIA_FORMAT_DECODER%", newFileText, "availableMediaDecoderFormatsStatic", "verbose", availableDecoderMediaFormats( true ), availableDecoderMediaFormats( false ) );
                    replaceText( "%MEDIA_FORMAT_DECODER_EXTENSION_MAP%", newFileText, "getDecoderFormatExtensionsMapStatic", getDecoderFormatExtensionsMap() );

                    replaceText( "%AVAILABLE_VIDEO_ENCODING_CODECS%", newFileText, "availableVideoEncodingCodecsStatic", "verbose", availableVideoEncodingCodecs( true ), availableVideoEncodingCodecs( false ) );
                    replaceText( "%AVAILABLE_VIDEO_DECODING_CODECS%", newFileText, "availableVideoDecodingCodecsStatic", "verbose", availableVideoDecodingCodecs( true ), availableVideoDecodingCodecs( false ) );
                    replaceText( "%AVAILABLE_AUDIO_ENCODING_CODECS%", newFileText, "availableAudioEncodingCodecsStatic", "verbose", availableAudioEncodingCodecs( true ), availableAudioEncodingCodecs( false ) );
                    replaceText( "%AVAILABLE_AUDIO_DECODING_CODECS%", newFileText, "availableAudioDecodingCodecsStatic", "verbose", availableAudioDecodingCodecs( true ), availableAudioDecodingCodecs( false ) );
                    replaceText( "%AVAILABLE_SUBTITLE_ENCODING_CODECS%", newFileText, "availableSubtitleEncodingCodecsStatic", "verbose", availableSubtitleEncodingCodecs( true ), availableSubtitleEncodingCodecs( false ) );
                    replaceText( "%AVAILABLE_SUBTITLE_DECODING_CODECS%", newFileText, "availableSubtitleDecodingCodecsStatic", "verbose", availableSubtitleDecodingCodecs( true ), availableSubtitleDecodingCodecs( false ) );

                    replaceText( "%AVAILABLE_VIDEO_ENCODERS%", newFileText, "availableVideoEncodersStatic", "verbose", availableVideoEncoders( true ), availableVideoEncoders( false ) );
                    replaceText( "%AVAILABLE_VIDEO_DECODERS%", newFileText, "availableVideoDecodersStatic", "verbose", availableVideoDecoders( true ), availableVideoDecoders( false ) );
                    replaceText( "%AVAILABLE_AUDIO_ENCODERS%", newFileText, "availableAudioEncodersStatic", "verbose", availableAudioEncoders( true ), availableAudioEncoders( false ) );
                    replaceText( "%AVAILABLE_AUDIO_DECODERS%", newFileText, "availableAudioDecodersStatic", "verbose", availableAudioDecoders( true ), availableAudioDecoders( false ) );
                    replaceText( "%AVAILABLE_SUBTITLE_ENCODERS%", newFileText, "availableSubtitleEncodersStatic", "verbose", availableSubtitleEncoders( true ), availableSubtitleEncoders( false ) );
                    replaceText( "%AVAILABLE_SUBTITLE_DECODERS%", newFileText, "availableSubtitleDecodersStatic", "verbose", availableSubtitleDecoders( true ), availableSubtitleDecoders( false ) );

                    replaceText( "%CODEC_TO_ENCODER_MAP%", newFileText, "getCodecToEncoderMapStatic", getCodecToEncoderMap() );
                    replaceText( "%CODEC_TO_DECODER_MAP%", newFileText, "getCodecToDecoderMapStatic", getCodecToDecoderMap() );

                    replaceText( "%AVAILABLE_HW_ACCELS%", newFileText, "availableHWAccelsStatic", "verbose", availableHWAccels( true ), availableHWAccels( false ) );

                    QGuiApplication::clipboard()->setText( newFileText.join( "\n" ) );
                }
            }
        }
    }
}
