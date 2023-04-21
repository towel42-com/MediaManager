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

#include "PatternInfo.h"

#include <QDir>
#include <QTimer>
#include <QTreeView>
#include <QApplication>

#include <QDirIterator>

namespace NMediaManager
{
    namespace NCore
    {
        QString patternToRegExp( const QString &captureName, const QString &inPattern, const QString &value, bool removeOptional )
        {
            if ( captureName.isEmpty() || inPattern.isEmpty() )
                return inPattern;

            // see if the capture name exists in the return pattern
            auto capRegEx = QString( R"({(?<optname>[^\{\}]+)}(\\)?\:\<%1\>)" ).arg( captureName );
            auto regExp = QRegularExpression( capRegEx );
            auto retVal = inPattern;
            retVal = retVal.replace( regExp, removeOptional ? R"(\1)" : R"((\1)?)" );

            capRegEx = QString( R"(\<%1\>)" ).arg( captureName );
            regExp = QRegularExpression( capRegEx );
            retVal = retVal.replace( regExp, value );

            return retVal;
        }

        QString patternToRegExp( const QString &pattern, bool removeOptional )
        {
            QString retVal = pattern;
            retVal.replace( "(", "\\(" );
            retVal.replace( ")", "\\)" );
            retVal.replace( ":", "\\:" );

            retVal = patternToRegExp( "title", retVal, ".*", removeOptional );
            retVal = patternToRegExp( "year", retVal, "((\\d{2}){1,2})", removeOptional );
            retVal = patternToRegExp( "tmdbid", retVal, "\\d+", removeOptional );
            retVal = patternToRegExp( "show_tmdbid", retVal, "\\d+", removeOptional );
            retVal = patternToRegExp( "season", retVal, "\\d+", removeOptional );
            retVal = patternToRegExp( "episode", retVal, "\\d+", removeOptional );
            retVal = patternToRegExp( "episode_title", retVal, ".*", removeOptional );
            retVal = patternToRegExp( "extra_info", retVal, ".*", removeOptional );
            return retVal;
        }

        bool SPatternInfo::isValidName( const QFileInfo &fi ) const
        {
            return isValidName( fi.fileName(), fi.isDir() );
        }

        bool SPatternInfo::isValidName( const QString &name, bool isDir ) const
        {
            if ( name.isEmpty() )
                return false;
            QStringList patterns;
            if ( isDir )
            {
                patterns << patternToRegExp( dirPattern(), true ) << patternToRegExp( dirPattern(), false ) << "(.*)\\s\\(((\\d{2}){1,2}\\))\\s(-\\s(.*)\\s)?\\[(tmdbid=\\d+)|(imdbid=tt.*)\\]";
            }
            else
            {
                patterns << patternToRegExp( filePattern(), true ) << patternToRegExp( filePattern(), false );
            }
            for ( auto &&ii : patterns )
            {
                QRegularExpression regExp( ii );
                if ( !ii.isEmpty() && regExp.match( name ).hasMatch() )
                    return true;
            }
            return false;
        }
    }
}
