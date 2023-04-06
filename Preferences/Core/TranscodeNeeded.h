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

#ifndef _CORE_TRANSCODENEEDED_H
#define _CORE_TRANSCODENEEDED_H

#include <memory>
#include <optional>
#include <QString>

namespace NSABUtils
{
    class CMediaInfo;
}

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NCore
        {
            class CPreferences;
            struct STranscodeNeeded
            {
                STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const CPreferences *prefs );
                STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo );

                std::optional< QString > getFormatMessage() const;
                std::optional< QString > getVideoCodecMessage() const;
                std::optional< QString > getAudioCodecMessage() const;
                QStringList getActions() const;
                QString getProgressLabelHeader( const QString &from, const QStringList & otherFiles, const QString &to ) const;

                bool transcodeNeeded() const { return fForce || fVideo || fAudio || fMissingAAC || fFormat; }
                bool formatOnly() const { return fFormat && !fVideo && !fAudio; }

                bool formatChangeNeeded() const { return fFormat; }
                bool addAACAudioCodec() const { return fMissingAAC; }
                bool audioTranscodeNeeded() const { return fAudio; }
                bool videoTranscodeNeeded() const { return fVideo; }
            private:
                bool fVideo{ false };
                bool fAudio{ false };
                bool fMissingAAC{ false };
                bool fFormat{ false };
                bool fForce{ false };
                std::shared_ptr< NSABUtils::CMediaInfo > fMediaInfo;
            };
        }
    }
}
#endif
