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
#include <QRegularExpression>
class QFileInfo;
class QWidget;

namespace NSABUtils
{
    enum class EMediaTags;
}

class QTimer;
namespace NMediaManager
{
    namespace NPreferences
    {
        enum class EItemStatus
        {
            eOK,
            eWarning,
            eError
        };

        QString toString( EItemStatus status );

        enum EPreferenceType
        {
            eSystemPrefs = 1,
            eColorsPrefs = 2,
            eLoadPrefs = 4,
            eTransformPrefs = 8,
            eTagPrefs = 16,
            eExtToolsPrefs = 32,
            eBIFPrefs = 64,
            eGIFPrefs = 128
        };
        Q_DECLARE_FLAGS( EPreferenceTypes, EPreferenceType );
        Q_DECLARE_OPERATORS_FOR_FLAGS( EPreferenceTypes );
        QString toString( EPreferenceType prefType );

        namespace NCore
        {
            class CPreferences : public QObject
            {
                Q_OBJECT;

                CPreferences();
            public:
                static CPreferences * instance();
                virtual ~CPreferences() override;

                QString validateDefaults() const;
                void showValidateDefaults( QWidget * parent, bool showNoChange );

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
                QStringList getDefaultSkippedPaths() const;
                QStringList getSkippedPaths() const;
                void setSkippedPaths( const QStringList & value );

                void setIgnorePathNamesToSkip( bool value );
                bool getIgnorePathNamesToSkip() const;

                bool isIgnoredPath( const QFileInfo & fileInfo ) const;
                QStringList getDefaultIgnoredPaths() const;
                QStringList getIgnoredPaths() const;
                void setIgnoredPaths( const QStringList & value );

                void setIgnorePathNamesToIgnore( bool value );
                bool getIgnorePathNamesToIgnore() const;

                std::list< std::pair< NSABUtils::EMediaTags, bool > > getAllMediaTags() const;
                std::list< NSABUtils::EMediaTags > getEnabledTags() const;
                QStringList getEnabledTagsForDisplay() const;
                void setEnabledTags( const std::list< NSABUtils::EMediaTags > & value );

                bool getVerifyMediaTags() const;
                void setVerifyMediaTags( bool value );

                bool getVerifyMediaTitle() const;
                void setVerifyMediaTitle( bool value );

                QString getVerifyMediaTitleExpr() const;
                QRegularExpression getVerifyMediaTitleExpr( const QFileInfo & fi, const QDate & date ) const; // fills in the fields and returns a regularExpression
                void setVerifyMediaTitleExpr( const QString & value );

                bool getVerifyMediaDate() const;
                void setVerifyMediaDate( bool value );

                QString getVerifyMediaDateExpr() const;
                QRegularExpression getVerifyMediaDateExpr( const QFileInfo & fi, const QDate & date ) const;
                void setVerifyMediaDateExpr( const QString & value );

                bool getVerifyMediaComment() const;
                void setVerifyMediaComment( bool value );

                QString getVerifyMediaCommentExpr() const;
                QRegularExpression getVerifyMediaCommentExpr( const QFileInfo & fi, const QDate & date ) const;
                void setVerifyMediaCommentExpr( const QString & value );

                QStringList getDefaultCustomPathsToDelete() const;
                QStringList getCustomPathsToDelete() const;
                void setCustomPathsToDelete( const QStringList & paths );

                bool deleteCustom() const;
                void setDeleteCustom( bool deleteCustom );

                bool deleteEXE() const;
                void setDeleteEXE( bool deleteEXEs );

                bool deleteNFO() const;
                void setDeleteNFO( bool deleteNFO );

                bool deleteBAK() const;
                void setDeleteBAK( bool deleteBAK );

                bool deleteTXT() const;
                void setDeleteTXT( bool deleteTXT );

                QStringList getExtensionsToDelete() const;
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

                QStringList getDefaultKnownStrings() const;
                QStringList getKnownStrings() const;
                QStringList getKnownStringRegExs() const;

                void setKnownExtendedStrings( const QStringList & value );
                QStringList getDefaultKnownExtendedStrings() const;
                QStringList getKnownExtendedStrings() const;

                void setKnownAbbreviations( const QVariantMap & value );  // needs to be QString to QString
                void setKnownAbbreviations( const QList<QPair<QString, QString >> & value );
                QVariantMap getDefaultKnownAbbreviations() const;
                QVariantMap getKnownAbbreviations() const;

                void setKnownHyphenated( const QStringList & value );
                QStringList getDefaultKnownHyphenated() const;
                QStringList getKnownHyphenated() const;
                std::list< std::pair< QString, int > > getKnownHyphenatedData() const;
                void setMKVMergeEXE( const QString & value );
                QString getMKVMergeEXE() const;

                void setMKVPropEditEXE( const QString & value );
                QString getMKVPropEditEXE() const;

                void setFFMpegEXE( const QString & value );
                QString getFFMpegEXE() const;

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

                QColor getColorForStatus( EItemStatus status, bool background ) const;
                void setColorForStatus( EItemStatus, bool background, const QColor & value );

                bool getPageVisible( const QString & pageName ) const;
                void setPageVisible( const QString & pageName, bool isVisible );

                int getNumSearchPages() const;
                void setNumSearchPages( int numpages );
            Q_SIGNALS:
                void sigPreferencesChanged( EPreferenceTypes prefType );
            private:
                QStringList getDefaultFile() const;
                QString compareValues( const QString & title, const QStringList & defaultValues, const QStringList & currValues ) const;
                QString compareValues( const QString & title, const QString & defaultValues, const QString & currValues ) const;
                QString compareValues( const QString & title, const QVariantMap & defaultValues, const QVariantMap & currValues ) const;
                QStringList variantMapToStringList( const QVariantMap & data ) const;

                QStringList cleanUpPaths( const QStringList & paths, bool areDirs ) const;
                void emitSigPreferencesChanged( EPreferenceTypes prefType );
                bool pathMatches( const QFileInfo & fileInfo, const QStringList & values ) const;
                bool containsValue( const QString & value, const QStringList & values ) const;
                //QString getDefaultInPattern( bool forTV ) const;
                QString getDefaultOutDirPattern( bool forTV ) const;
                QString getDefaultOutFilePattern( bool forTV ) const;
                QTimer * fPrefChangeTimer{ nullptr };
                EPreferenceTypes fPending;
            };
        }
    }
}
#endif 
