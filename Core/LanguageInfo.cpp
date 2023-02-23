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

#include "LanguageInfo.h"
#include "SABUtils/StringUtils.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>
#include <QFile>
#include <QTextStream>

#include <unordered_set>
#include <tuple>

namespace NMediaManager
{
    namespace NCore
    {
        QString SLanguageInfo::sDefaultISOCode{ "en_US" };

        SLanguageInfo::SLanguageInfo()
        {
        }

        SLanguageInfo::SLanguageInfo( const QFileInfo &fi ) :
            SLanguageInfo( fi.completeBaseName() )
        {
            if ( fi.suffix() == "idx" )
                *this = fromIDXFile( fi );
        }

        SLanguageInfo::SLanguageInfo( const QString &fileName ) :
            fFileName( fileName )
        {
            computeLanguage();
        }

        SLanguageInfo::SLanguageInfo( const QString &isoCode, const QString &country )
        {
            auto language = isoCode;
            if ( !country.isEmpty() && !country.startsWith( "_" ) )
                language += "_";
            language += country;
            computeLanguage( language );
        }

        SLanguageInfo SLanguageInfo::fromIDXFile( const QString &path )
        {
            return std::move( fromIDXFile( QFileInfo( path ) ) );
        }

        SLanguageInfo SLanguageInfo::fromIDXFile( const QFileInfo &fileInfo )
        {
            Q_ASSERT( fileInfo.suffix() == "idx" );
            SLanguageInfo langInfo( fileInfo.completeBaseName() );
            langInfo.computeLanguages( fileInfo );
            return langInfo;
        }

        QString SLanguageInfo::baseName() const
        {
            return fBaseName.isEmpty() ? fFileName : fBaseName;
        }

        bool SLanguageInfo::isNameBasedLangFile() const
        {
            return !fBaseName.isEmpty();
        }

        QString SLanguageInfo::displayName() const
        {
            if ( isMultiLanguage() )
            {
                return allLanguages();
            }
            auto retVal = fLanguage;
            if ( !fCountry.isEmpty() )
                retVal += " " + fCountry;
            return retVal;
        }

        QString SLanguageInfo::isoCode() const
        {
            auto retVal = fISOCode.toLower();
            auto pos = retVal.indexOf( '_' );
            if ( pos == -1 )
                pos = retVal.indexOf( '-' );
            if ( pos != -1 )
                retVal = retVal.left( pos );
            return retVal;
        }

        QString SLanguageInfo::allLanguages() const
        {
            std::unordered_set< QString > handled;
            QStringList tmp;
            for ( auto &&ii : fMultiLanguageList )
            {
                tmp << ii.first;
            }
            return tmp.join( ", " );
        }

        void SLanguageInfo::setDefaultISOCode( const QString &value )   // default is "en_us"
        {
            if ( isKnownLanguage( value ) )
            {
                sDefaultISOCode = prettyPrintISOCode( value );
                return;
            }
            Q_ASSERT( isKnownLanguage( value ) );
        }

        std::unordered_map< QString, std::pair< QString, QString > > SLanguageInfo::sLangMap = {
            { "af", { "Afrikaans", "" } },
            { "af-ZA", { "Afrikaans", "(South Africa)" } },
            { "ar", { "Arabic", "" } },
            { "ar-AE", { "Arabic", "(U.A.E.)" } },
            { "ar-BH", { "Arabic", "(Bahrain)" } },
            { "ar-DZ", { "Arabic", "(Algeria)" } },
            { "ar-EG", { "Arabic", "(Egypt)" } },
            { "ar-IQ", { "Arabic", "(Iraq)" } },
            { "ar-JO", { "Arabic", "(Jordan)" } },
            { "ar-KW", { "Arabic", "(Kuwait)" } },
            { "ar-LB", { "Arabic", "(Lebanon)" } },
            { "ar-LY", { "Arabic", "(Libya)" } },
            { "ar-MA", { "Arabic", "(Morocco)" } },
            { "ar-OM", { "Arabic", "(Oman)" } },
            { "ar-QA", { "Arabic", "(Qatar)" } },
            { "ar-SA", { "Arabic", "(Saudi Arabia)" } },
            { "ar-SY", { "Arabic", "(Syria)" } },
            { "ar-TN", { "Arabic", "(Tunisia)" } },
            { "ar-YE", { "Arabic", "(Yemen)" } },
            { "az", { "Azeri", "(Latin)" } },
            { "az-AZ", { "Azeri", "(Latin) (Azerbaijan)" } },
            { "az-AZ", { "Azeri", "(Cyrillic) (Azerbaijan)" } },
            { "be", { "Belarusian", "" } },
            { "be-BY", { "Belarusian", "(Belarus)" } },
            { "bg", { "Bulgarian", "" } },
            { "bg-BG", { "Bulgarian", "(Bulgaria)" } },
            { "bs-BA", { "Bosnian", "(Bosnia and Herzegovina)" } },
            { "ca", { "Catalan", "" } },
            { "ca-ES", { "Catalan", "(Spain)" } },
            { "cs", { "Czech", "" } },
            { "cs-CZ", { "Czech", "(Czech Republic)" } },
            { "cy", { "Welsh", "" } },
            { "cy-GB", { "Welsh", "(UK)" } },
            { "da", { "Danish", "" } },
            { "da-DK", { "Danish", "(Denmark)" } },
            { "de", { "German", "" } },
            { "de-AT", { "German", "(Austria)" } },
            { "de-CH", { "German", "(Switzerland)" } },
            { "de-DE", { "German", "(Germany)" } },
            { "de-LI", { "German", "(Liechtenstein)" } },
            { "de-LU", { "German", "(Luxembourg)" } },
            { "dv", { "Divehi", "" } },
            { "dv-MV", { "Divehi", "(Maldives)" } },
            { "el", { "Greek", "" } },
            { "el-GR", { "Greek", "(Greece)" } },
            { "en", { "English", "" } },
            { "en-AU", { "English", "(Australia)" } },
            { "en-BZ", { "English", "(Belize)" } },
            { "en-CA", { "English", "(Canada)" } },
            { "en-CB", { "English", "(Caribbean)" } },
            { "en-GB", { "English", "(UK)" } },
            { "en-IE", { "English", "(Ireland)" } },
            { "en-JM", { "English", "(Jamaica)" } },
            { "en-NZ", { "English", "(New Zealand)" } },
            { "en-PH", { "English", "(Republic of the Philippines)" } },
            { "en-TT", { "English", "(Trinidad and Tobago)" } },
            { "en-US", { "English", "(US)" } },
            { "en-ZA", { "English", "(South Africa)" } },
            { "en-ZW", { "English", "(Zimbabwe)" } },
            { "eo", { "Esperanto", "" } },
            { "es", { "Spanish", "" } },
            { "es-AR", { "Spanish", "(Argentina)" } },
            { "es-BO", { "Spanish", "(Bolivia)" } },
            { "es-CL", { "Spanish", "(Chile)" } },
            { "es-CO", { "Spanish", "(Colombia)" } },
            { "es-CR", { "Spanish", "(Costa Rica)" } },
            { "es-DO", { "Spanish", "(Dominican Republic)" } },
            { "es-EC", { "Spanish", "(Ecuador)" } },
            { "es-ES", { "Spanish", "(Castilian)" } },
            { "es-ES", { "Spanish", "(Spain)" } },
            { "es-GT", { "Spanish", "(Guatemala)" } },
            { "es-HN", { "Spanish", "(Honduras)" } },
            { "es-MX", { "Spanish", "(Mexico)" } },
            { "es-NI", { "Spanish", "(Nicaragua)" } },
            { "es-PA", { "Spanish", "(Panama)" } },
            { "es-PE", { "Spanish", "(Peru)" } },
            { "es-PR", { "Spanish", "(Puerto Rico)" } },
            { "es-PY", { "Spanish", "(Paraguay)" } },
            { "es-SV", { "Spanish", "(El Salvador)" } },
            { "es-UY", { "Spanish", "(Uruguay)" } },
            { "es-VE", { "Spanish", "(Venezuela)" } },
            { "et", { "Estonian", "" } },
            { "et-EE", { "Estonian", "(Estonia)" } },
            { "eu", { "Basque", "" } },
            { "eu-ES", { "Basque", "(Spain)" } },
            { "fa", { "Farsi", "" } },
            { "fa-IR", { "Farsi", "(Iran)" } },
            { "fi", { "Finnish", "" } },
            { "fi-FI", { "Finnish", "(Finland)" } },
            { "fil", { "Filipino;Pilipino", "(Philippines)" } },
            { "fo", { "Faroese", "" } },
            { "fo-FO", { "Faroese", "(Faroe Islands)" } },
            { "fr", { "French", "" } },
            { "fr-BE", { "French", "(Belgium)" } },
            { "fr-CA", { "French", "(Canada)" } },
            { "fr-CH", { "French", "(Switzerland)" } },
            { "fr-FR", { "French", "(France)" } },
            { "fr-LU", { "French", "(Luxembourg)" } },
            { "fr-MC", { "French", "(Principality of Monaco)" } },
            { "gl", { "Galician", "" } },
            { "gl-ES", { "Galician", "(Spain)" } },
            { "gu", { "Gujarati", "" } },
            { "gu-IN", { "Gujarati", "(India)" } },
            { "he", { "Hebrew", "" } },
            { "he-IL", { "Hebrew", "(Israel)" } },
            { "hi", { "Hindi", "" } },
            { "hi-IN", { "Hindi", "(India)" } },
            { "hr", { "Croatian", "" } },
            { "hr-BA", { "Croatian", "(Bosnia and Herzegovina)" } },
            { "hr-HR", { "Croatian", "(Croatia)" } },
            { "hu", { "Hungarian", "" } },
            { "hu-HU", { "Hungarian", "(Hungary)" } },
            { "hy", { "Armenian", "" } },
            { "hy-AM", { "Armenian", "(Armenia)" } },
            { "id", { "Indonesian", "" } },
            { "id-ID", { "Indonesian", "(Indonesia)" } },
            { "is", { "Icelandic", "" } },
            { "is-IS", { "Icelandic", "(Iceland)" } },
            { "it", { "Italian", "" } },
            { "it-CH", { "Italian", "(Switzerland)" } },
            { "it-IT", { "Italian", "(Italy)" } },
            { "ja", { "Japanese", "" } },
            { "ja-JP", { "Japanese", "(Japan)" } },
            { "ka", { "Georgian", "" } },
            { "ka-GE", { "Georgian", "(Georgia)" } },
            { "kk", { "Kazakh", "" } },
            { "kk-KZ", { "Kazakh", "(Kazakhstan)" } },
            { "kn", { "Kannada", "" } },
            { "kn-IN", { "Kannada", "(India)" } },
            { "ko", { "Korean", "" } },
            { "ko-KR", { "Korean", "(Korea)" } },
            { "kok", { "Konkani", "" } },
            { "kok-IN", { "Konkani", "(India)" } },
            { "ky", { "Kyrgyz", "" } },
            { "ky-KG", { "Kyrgyz", "(Kyrgyzstan)" } },
            { "lt", { "Lithuanian", "" } },
            { "lt-LT", { "Lithuanian", "(Lithuania)" } },
            { "lv", { "Latvian", "" } },
            { "lv-LV", { "Latvian", "(Latvia)" } },
            { "mi", { "Maori", "" } },
            { "mi-NZ", { "Maori", "(New Zealand)" } },
            { "mk", { "FYRO Macedonian", "" } },
            { "mk-MK", { "FYRO Macedonian", "(Former Yugoslav Republic of Macedonia)" } },
            { "mn", { "Mongolian", "" } },
            { "mn-MN", { "Mongolian", "(Mongolia)" } },
            { "mr", { "Marathi", "" } },
            { "mr-IN", { "Marathi", "(India)" } },
            { "ms", { "Malay", "" } },
            { "ms-BN", { "Malay", "(Brunei Darussalam)" } },
            { "ms-MY", { "Malay", "(Malaysia)" } },
            { "mt", { "Maltese", "" } },
            { "mt-MT", { "Maltese", "(Malta)" } },
            { "nb", { "Norwegian", "(Bokm?l)" } },
            { "nb-NO", { "Norwegian", "(Bokm?l) (Norway)" } },
            { "nl", { "Dutch", "" } },
            { "nl-BE", { "Dutch", "(Belgium)" } },
            { "nl-NL", { "Dutch", "(Netherlands)" } },
            { "nn-NO", { "Norwegian", "(Nynorsk) (Norway)" } },
            { "ns", { "Northern Sotho", "" } },
            { "ns-ZA", { "Northern Sotho", "(South Africa)" } },
            { "pa", { "Punjabi", "" } },
            { "pa-IN", { "Punjabi", "(India)" } },
            { "pl", { "Polish", "" } },
            { "pl-PL", { "Polish", "(Poland)" } },
            { "ps", { "Pashto", "" } },
            { "ps-AR", { "Pashto", "(Afghanistan)" } },
            { "pt", { "Portuguese", "" } },
            { "pt-BR", { "Portuguese", "(Brazil)" } },
            { "pt-PT", { "Portuguese", "(Portugal)" } },
            { "qu", { "Quechua", "" } },
            { "qu-BO", { "Quechua", "(Bolivia)" } },
            { "qu-EC", { "Quechua", "(Ecuador)" } },
            { "qu-PE", { "Quechua", "(Peru)" } },
            { "ro", { "Romanian", "" } },
            { "ro-RO", { "Romanian", "(Romania)" } },
            { "ru", { "Russian", "" } },
            { "ru-RU", { "Russian", "(Russia)" } },
            { "sa", { "Sanskrit", "" } },
            { "sa-IN", { "Sanskrit", "(India)" } },
            { "se", { "Sami", "(Northern)" } },
            { "se-FI", { "Sami", "(Northern) (Finland)" } },
            { "se-FI", { "Sami", "(Skolt) (Finland)" } },
            { "se-FI", { "Sami", "(Inari) (Finland)" } },
            { "se-NO", { "Sami", "(Northern) (Norway)" } },
            { "se-NO", { "Sami", "(Lule) (Norway)" } },
            { "se-NO", { "Sami", "(Southern) (Norway)" } },
            { "se-SE", { "Sami", "(Northern) (Sweden)" } },
            { "se-SE", { "Sami", "(Lule) (Sweden)" } },
            { "se-SE", { "Sami", "(Southern) (Sweden)" } },
            { "sk", { "Slovak", "" } },
            { "sk-SK", { "Slovak", "(Slovakia)" } },
            { "sl", { "Slovenian", "" } },
            { "sl-SI", { "Slovenian", "(Slovenia)" } },
            { "sq", { "Albanian", "" } },
            { "sq-AL", { "Albanian", "(Albania)" } },
            { "sr-BA", { "Serbian", "(Latin) (Bosnia and Herzegovina)" } },
            { "sr-BA", { "Serbian", "(Cyrillic) (Bosnia and Herzegovina)" } },
            { "sr-SP", { "Serbian", "(Latin) (Serbia and Montenegro)" } },
            { "sr-SP", { "Serbian", "(Cyrillic) (Serbia and Montenegro)" } },
            { "sv", { "Swedish", "" } },
            { "sv-FI", { "Swedish", "(Finland)" } },
            { "sv-SE", { "Swedish", "(Sweden)" } },
            { "sw", { "Swahili", "" } },
            { "sw-KE", { "Swahili", "(Kenya)" } },
            { "syr", { "Syriac", "" } },
            { "syr-SY", { "Syriac", "(Syria)" } },
            { "ta", { "Tamil", "" } },
            { "ta-IN", { "Tamil", "(India)" } },
            { "te", { "Telugu", "" } },
            { "te-IN", { "Telugu", "(India)" } },
            { "th", { "Thai", "" } },
            { "th-TH", { "Thai", "(Thailand)" } },
            { "tl", { "Tagalog", "" } },
            { "tl-PH", { "Tagalog", "(Philippines)" } },
            { "tn", { "Tswana", "" } },
            { "tn-ZA", { "Tswana", "(South Africa)" } },
            { "tr", { "Turkish", "" } },
            { "tr-TR", { "Turkish", "(Turkey)" } },
            { "tt", { "Tatar", "" } },
            { "tt-RU", { "Tatar", "(Russia)" } },
            { "ts", { "Tsonga", "" } },
            { "uk", { "Ukrainian", "" } },
            { "uk-UA", { "Ukrainian", "(Ukraine)" } },
            { "ukr", { "Ukrainian", "" } },
            { "ukr-UA", { "Ukrainian", "(Ukraine)" } },
            { "ur", { "Urdu", "" } },
            { "ur-PK", { "Urdu", "(Islamic Republic of Pakistan)" } },
            { "uz", { "Uzbek", "(Latin)" } },
            { "uz-UZ", { "Uzbek", "(Latin) (Uzbekistan)" } },
            { "uz-UZ", { "Uzbek", "(Cyrillic) (Uzbekistan)" } },
            { "vi", { "Vietnamese", "" } },
            { "vi-VN", { "Vietnamese", "(Viet Nam)" } },
            { "xh", { "Xhosa", "" } },
            { "xh-ZA", { "Xhosa", "(South Africa)" } },
            { "zh", { "Chinese", "" } },
            { "zh-CN", { "Chinese", "(S)" } },
            { "zh-HK", { "Chinese", "(Hong Kong)" } },
            { "zh-MO", { "Chinese", "(Macau)" } },
            { "zh-SG", { "Chinese", "(Singapore)" } },
            { "zh-TW", { "Chinese", "(T)" } },
            { "zu", { "Zulu", "" } },
            { "zu-ZA", { "Zulu", "(South Africa)" } } };

        std::unordered_map< QString, std::pair< QString, std::unordered_set< QString > > > SLanguageInfo::sPrimToSecondaryMap;
        std::unordered_map< QString, std::pair< QString, QString > > SLanguageInfo::sNameToCodeMap;

        void SLanguageInfo::setupMaps()
        {
            if ( sPrimToSecondaryMap.empty() )
            {
                sLangMap[ "Hin" ] = ( *sLangMap.find( "hi-IN" ) ).second;
                sLangMap[ "May" ] = ( *sLangMap.find( "ms-MY" ) ).second;
                sLangMap[ "Nor" ] = ( *sLangMap.find( "nb-NO" ) ).second;
                sLangMap[ "Tam" ] = ( *sLangMap.find( "ta-IN" ) ).second;
                sLangMap[ "Tel" ] = ( *sLangMap.find( "te-IN" ) ).second;
                sLangMap[ "Eng" ] = ( *sLangMap.find( "en-US" ) ).second;
                sLangMap[ "no" ] = ( *sLangMap.find( "nb" ) ).second;
                sLangMap[ "slo" ] = ( *sLangMap.find( "sl-SI" ) ).second;

                for ( auto &&ii : sLangMap )
                {
                    sLangMap[ ii.first.toLower() ] = ii.second;
                    sLangMap[ ii.first.toLower().replace( "-", "_" ) ] = ii.second;

                    auto code = ii.first;

                    auto prim = ii.second.first;
                    auto secondary = ii.second.second;

                    if ( secondary.isEmpty() )
                        sNameToCodeMap[ prim.toLower() ] = std::make_pair( prim, code );
                    else
                    {
                        sPrimToSecondaryMap[ prim.toLower() ].first = prim;
                        sPrimToSecondaryMap[ prim.toLower() ].second.insert( secondary );
                    }
                }
            }
        }

        bool SLanguageInfo::isKnownLanguage( const QString &lang ) const
        {
            setupMaps();
            if ( sLangMap.find( lang.toLower() ) != sLangMap.end() )
                return true;
            if ( sNameToCodeMap.find( lang.toLower() ) != sNameToCodeMap.end() )
                return true;
            return false;
        }

        bool SLanguageInfo::isLangFileFormat( const QFileInfo &fi )
        {
            auto fn = fi.completeBaseName();
            auto regExp = QRegularExpression( "\\d+_\\S+" );
            return regExp.match( fn ).hasMatch();
        }

        QString SLanguageInfo::prettyPrintISOCode( const QString &isoCode )
        {
            auto tmp = isoCode.split( QRegularExpression( "_|-" ), NSABUtils::NStringUtils::TSkipEmptyParts );
            if ( tmp.length() > 1 )
                tmp[ 1 ] = tmp[ 1 ].toUpper();
            return tmp.join( "_" );
        }

        bool SLanguageInfo::operator==( const SLanguageInfo &rhs ) const
        {
            bool retVal = ( fFileName == rhs.fFileName ) && ( fBaseName == rhs.fBaseName ) && ( fISOCode == rhs.fISOCode ) && ( fLanguage == rhs.fLanguage ) && ( fCountry == rhs.fCountry ) && ( fIsForced == rhs.fIsForced )
                          && ( fIsSDH == rhs.fIsSDH ) && ( fUsingDefault == rhs.fUsingDefault );
            if ( !retVal )
                return false;
            if ( fMultiLanguageList.size() != rhs.fMultiLanguageList.size() )
                return false;
            auto ii = fMultiLanguageList.begin();
            auto jj = rhs.fMultiLanguageList.begin();
            for ( ; ( ii != fMultiLanguageList.end() ) && ( jj != rhs.fMultiLanguageList.end() ); ++ii, ++jj )
            {
                auto &&kk = ( *ii ).first;
                auto &&ll = ( *jj ).first;

                if ( kk != ll )
                    return false;
            }
            return true;
        }

        void SLanguageInfo::computeLanguages( const QFileInfo &fi )
        {
            setupMaps();

            if ( fi.suffix() != "idx" )
                return;
            QFile file( fi.absoluteFilePath() );
            file.open( QFile::ReadOnly | QFile::Text );
            if ( !file.isOpen() )
                return;

            auto regExStr = R"(^id\:\s?(?<isocode>[A-Za-z]{2})(?<country>_[A-Za-z]{2}(\d+)?)?,\s?index\:\s?(?<index>\d+)\s?$)";
            auto regExp = QRegularExpression( regExStr, QRegularExpression::CaseInsensitiveOption );

            regExStr = R"(^timestamp\:)";
            auto tsRegExp = QRegularExpression( regExStr, QRegularExpression::CaseInsensitiveOption );

            std::list< SMultLangInfo > languages;

            QTextStream ts( &file );
            QString currLine;
            int lineNum = 0;
            int currSubTitleLines = 0;
            while ( ts.readLineInto( &currLine ) )
            {
                lineNum++;
                if ( !languages.empty() )
                {
                    if ( tsRegExp.match( currLine ).hasMatch() )
                    {
                        currSubTitleLines++;
                    }
                }
                auto match = regExp.match( currLine );
                if ( match.hasMatch() )
                {
                    if ( !languages.empty() )
                    {
                        languages.back().fSize = currSubTitleLines;
                    }

                    currSubTitleLines = 0;
                    auto isoCode = match.captured( "isocode" );
                    auto country = match.captured( "country" );
                    int index = match.captured( "index" ).toInt();

                    auto currLanguage = std::make_shared< SLanguageInfo >( isoCode, country );
                    if ( !currLanguage->usingDefault() )
                        languages.emplace_back( SMultLangInfo( currLanguage, index, -1 ) );
                }
            }

            if ( !languages.empty() )
            {
                languages.back().fSize = currSubTitleLines;
            }

            for ( auto &&ii : languages )
            {
                auto dispName = ii.fLanguage->displayName();
                fMultiLanguageList[ dispName ].emplace_back( ii );
            }
            for ( auto &&ii : fMultiLanguageList )
            {
                std::sort(
                    ii.second.begin(), ii.second.end(),
                    []( const SMultLangInfo &lhs, const SMultLangInfo &rhs )
                    {
                        if ( lhs.fLanguage->isoCode() != rhs.fLanguage->isoCode() )
                            return lhs.fLanguage->isoCode() < lhs.fLanguage->isoCode();

                        return lhs.fSize < rhs.fSize;
                    } );
            }
        }

        void SLanguageInfo::computeLanguage()
        {
            setupMaps();

            fLanguage.clear();
            fIsSDH = fIsForced = false;
            if ( fFileName.isEmpty() )
                return;

            auto regExpStr = R"((?<num>\d+)_(?<langname>\S+))";
            auto regExp1 = QRegularExpression( regExpStr );
            Q_ASSERT( regExp1.isValid() );

            regExpStr = R"((?<filename>[^<>:"\/\\|?*]+)\.(((?<isocode>[A-Za-z]{2})(?<country>_[A-Za-z]{2}(\d+)?)?$)|(?<langname>[A-Za-z]{2,3}$)))";
            auto regExp2 = QRegularExpression( regExpStr );
            Q_ASSERT( regExp2.isValid() );

            QRegularExpressionMatch match1;
            QRegularExpressionMatch match2;

            QString num;
            QString langName;
            if ( ( match1 = regExp1.match( fFileName ) ).hasMatch() )
            {
                num = match1.captured( "num" );
                langName = match1.captured( "langname" );
            }
            else if ( ( match2 = regExp2.match( fFileName ) ).hasMatch() )
            {
                fBaseName = match2.captured( "filename" );
                if ( match2.captured( "langname" ).isEmpty() )
                {
                    langName = match2.captured( "isocode" );
                    auto country = match2.captured( "country" );
                    if ( !country.isEmpty() )
                        langName += country;
                }
                else
                    langName = match2.captured( "langname" );
            }
            else
            {
                fUsingDefault = true;
                langName = sDefaultISOCode;
            }

            computeLanguage( langName );
        }

        void SLanguageInfo::computeLanguage( const QString &langName )
        {
            std::tie( fLanguage, fCountry, fISOCode, fUsingDefault ) = computeLanguageInt( langName );
        }

        std::tuple< QString, QString, QString, bool > SLanguageInfo::computeLanguageInt( const QString &langName )
        {
            setupMaps();

            if ( langName.isEmpty() )
                return {};

            QString language;
            QString country;
            QString isoCode;
            auto pos = sLangMap.find( langName.toLower() );
            if ( pos != sLangMap.end() )
            {
                language = ( *pos ).second.first;
                country = ( *pos ).second.second;
                isoCode = prettyPrintISOCode( ( *pos ).first );
            }
            else
            {
                auto pos = sNameToCodeMap.find( langName.toLower() );
                if ( pos != sNameToCodeMap.end() )
                {
                    language = ( *pos ).second.first;
                    isoCode = prettyPrintISOCode( ( *pos ).second.second );
                }
            }
            bool usingDefault{ false };
            if ( language.isEmpty() )
            {
                std::tie( language, country, isoCode, usingDefault ) = computeLanguageInt( sDefaultISOCode );
                usingDefault = true;
            }

            if ( ( isoCode.toLower() == sDefaultISOCode.left( 2 ) ) && ( isoCode.length() != 2 ) && country.isEmpty() )
            {
                std::tie( language, country, isoCode, usingDefault ) = computeLanguageInt( sDefaultISOCode );
                usingDefault = true;
            }
            return std::make_tuple( language, country, isoCode, usingDefault );
        }
    }
}