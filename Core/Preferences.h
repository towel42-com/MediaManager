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

#ifndef _CORE_PREFERENCES_H
#define _CORE_PREFERENCES_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <QHash>
class QFileInfo;

namespace NMediaManager
{
    namespace NCore
    {
        class CPreferences
        {
            CPreferences();
        public:
            static CPreferences * instance();
            ~CPreferences();

            void setDirectories( const QStringList & dirs );
            QStringList getDirectories() const;

            void setFileNames( const QStringList & fileNames );
            QStringList getFileNames() const;

            void setTreatAsTVShowByDefault( bool value );
            bool getTreatAsTVShowByDefault() const;

            void setExactMatchesOnly( bool value );
            bool getExactMatchesOnly() const;

            void setTVOutFilePattern( const QString & value );
            QString getTVOutFilePattern() const;

            void setTVOutDirPattern( const QString & value );
            QString getTVOutDirPattern() const;

            void setMovieOutFilePattern( const QString & value );
            QString getMovieOutFilePattern() const;

            void setMovieOutDirPattern( const QString & value );
            QString getMovieOutDirPattern() const;

            bool isSkippedPath( const QFileInfo & fileInfo ) const;
            bool isIgnoredPath( const QFileInfo & fileInfo ) const;

            QStringList getSkippedPaths() const;
            void setSkippedPaths( const QStringList & paths );

            QStringList getIgnoredPaths() const;
            void setIgnoredPaths( const QStringList & paths );

            QStringList getPathsToDelete() const;
            void setPathsToDelete( const QStringList & paths );

            bool deleteKnownPaths() const;
            void setDeleteKnownPaths( bool paths );
            bool isPathToDelete( const QString & path ) const;

            void setMediaExtensions( const QString & value );
            void setMediaExtensions( const QStringList & value );
            QStringList getMediaExtensions() const;

            void setSubtitleExtensions( const QString & value );
            void setSubtitleExtensions( const QStringList & value );
            QStringList getSubtitleExtensions() const;

            QStringList getNonMKVMediaExtensions() const;

            void addKnownStrings( const QStringList & value );
            void setKnownStrings( const QStringList & value );
            QStringList getKnownStrings() const;

            void setKnownExtendedStrings( const QStringList & value );
            QStringList getKnownExtendedStrings() const;

            void setKnownAbbreviations( const QVariantMap & value );  // needs to be QString to QString
            void setKnownAbbreviations( const QList<QPair<QString, QString >> & value );
            QVariantMap getKnownAbbreviations() const;
            
            void setMKVMergeEXE( const QString & value );
            QString getMKVMergeEXE() const;

            void setMKVPropEditEXE( const QString & value );
            QString getMKVPropEditEXE() const;

            void setFFMpegEXE( const QString & value );
            QString getFFMpegEXE() const;

            void setFFProbeEXE( const QString & value );
            QString getFFProbeEXE() const;

            bool isMediaFile( const QFileInfo & fi ) const;
            bool isSubtitleFile( const QFileInfo & info, bool * isLangFileFormat = nullptr ) const;

            void setBIFPlayerSpeedMultiplier( int speed ); // 100 = "full speed" ie 1x default is 200x
            int bifPlayerSpeedMultiplier() const;

            void setBIFNumFramesToSkip( int numFrames );
            int bifNumFramesToSkip() const;

            void setBIFLoopCount( int loopCount );
            int bifLoopCount() const;

            void setGIFFlipImage( bool flipImage );
            bool gifFlipImage() const;

            void setGIFDitherImage( bool ditherImage );
            bool gifDitherImage() const;

            void setGIFLoopCount( int loopCount );
            int gifLoopCount() const;

            void setGIFStartFrame( int startFrame );
            int gifStartFrame() const;

            void setGIFEndFrame( int endFrame );
            int gifEndFrame() const;

            int gifDelay() const;
            void setGIFDelay( int delay );
        private:
            bool pathMatches( const QFileInfo & fileInfo, const QStringList & values ) const;
            bool containsValue( const QString & value, const QStringList & values ) const;
            //QString getDefaultInPattern( bool forTV ) const;
            QString getDefaultOutDirPattern( bool forTV ) const;
            QString getDefaultOutFilePattern( bool forTV ) const;
        };
    }
}
#endif 
