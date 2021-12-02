// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#include <QString>
namespace NMediaManager
{
    namespace NCore
    {
        struct SLanguageInfo
        {
            SLanguageInfo( const QString &path );

            QString language() const { return fLanguage; }
            QString displayName() const;
            bool isForced() const { return fIsForced; }
            bool isSDH() const { return fIsSDH; }
            QString isoCode() const { return fISOCode; }
        private:
            void computeLanguage();
            QString fFileName;
            QString fISOCode;
            QString fLanguage;
            QString fCountry;
            bool fIsForced{ false };
            bool fIsSDH{ false };
        };
    }
}
QDebug operator<<( QDebug debug, const NMediaManager::NCore::SLanguageInfo & info );

#endif 
