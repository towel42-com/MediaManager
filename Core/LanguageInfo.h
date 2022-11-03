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

#ifndef _LANGUAGEINFO_H
#define _LANGUAGEINFO_H

#include "SABUtils/HashUtils.h"
#include <QString>
#include <unordered_map>
#include <unordered_set>
#include <QVariant>
#include <memory>
class QFileInfo;
namespace NMediaManager
{
    namespace NCore
    {
        struct SLanguageInfo;
        struct SMultLangInfo
        {
            SMultLangInfo(std::shared_ptr< SLanguageInfo > lang, int index, int size) :
                fLanguage(lang),
                fIndex(index),
                fSize(size)
            {
            }
            std::shared_ptr< SLanguageInfo > fLanguage;
            int fIndex{ -1 };
            int fSize{ -1 };
        };

        struct SLanguageInfo
        {
            SLanguageInfo();
            SLanguageInfo(const QFileInfo & path);
            SLanguageInfo( const QString & path );
            SLanguageInfo(const QString & isoCode, const QString & country);

            static SLanguageInfo fromIDXFile(const QString & path);
            static SLanguageInfo fromIDXFile(const QFileInfo & path);

            bool isNameBasedLangFile() const; // if the filename wasnt of the form XX_Language.srt, but rather basename-xx_yy.srt or basename.xx.srt
            QString baseName() const; // when the path sent in is a name based lang file, return basename from above

            QString language() const { return fLanguage; }
            QString displayName() const;
            bool isForced() const { return fIsForced; }
            bool isSDH() const { return fIsSDH; }
            QString isoCode() const;

            bool isMultiLanguage() const { return !fMultiLanguageList.empty(); }
            QString allLanguages() const;
            std::unordered_map< QString, std::vector< SMultLangInfo > > allLanguageInfos() const { return fMultiLanguageList; }

            // when the iso code/language cant be determined use this
            // this happens when the following happens
            // 1) its not of the format xx_Language
            // 2) its not of the form file.Language<_country.xx>
            // 3) the language from 1 or 2 isnt a known language or country
            void setDefaultISOCode( const QString & value );  // default is "en_US"

            static bool isLangFileFormat( const QFileInfo & fi );
            bool usingDefault() const { return fUsingDefault; }
            bool knownLanguage() const { return !usingDefault(); }
            static QString prettyPrintISOCode( const QString & isoCode );
            bool operator==(const SLanguageInfo & rhs) const;
            bool operator!=(const SLanguageInfo & rhs) const { return !operator==(rhs); }
        private:
            bool isKnownLanguage( const QString & lang ) const;
            void computeLanguages( const QFileInfo & fi );
            void computeLanguage();
            void computeLanguage( const QString & langName );
            static std::tuple< QString, QString, QString, bool > computeLanguageInt( const QString & langName ); // returns lang, country, iscode, using default


            static void setupMaps();

            QString fFileName;
            QString fBaseName;
            QString fISOCode;
            QString fLanguage;
            QString fCountry;
            bool fIsForced{ false };
            bool fIsSDH{ false };
            static QString sDefaultISOCode;
            bool fUsingDefault{ false };

            std::unordered_map< QString, std::vector< SMultLangInfo > > fMultiLanguageList; // langinfo, index, numtimestamps

            static std::unordered_map< QString, std::pair< QString, QString > > sLangMap;
            static std::unordered_map< QString, std::pair< QString, std::unordered_set< QString > > > sPrimToSecondaryMap;
            static std::unordered_map< QString, std::pair< QString, QString > > sNameToCodeMap;
        };

        Q_DECLARE_METATYPE(SLanguageInfo);
    }
}
QDebug operator<<( QDebug debug, const NMediaManager::NCore::SLanguageInfo & info );

#endif 
