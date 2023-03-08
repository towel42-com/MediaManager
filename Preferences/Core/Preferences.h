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
#include <unordered_set>
#include <optional>
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
            eGIFPrefs = 128,
            eMakeMKVPrefs = 256
        };
        Q_DECLARE_FLAGS( EPreferenceTypes, EPreferenceType );
        Q_DECLARE_OPERATORS_FOR_FLAGS( EPreferenceTypes );
        QString toString( EPreferenceType prefType );

        namespace NCore
        {
            enum EMakeMKVPreset
            {
                eUltraFast,
                eSuperFast,
                eVeryFast,
                eFaster,
                eFast,
                eMedium,
                eSlow,
                eSlower,
                eVerySlow
            };
            QString toString( EMakeMKVPreset preset );

            enum EMakeMKVTune
            {
                eFilm,
                eAnimation,
                eGrain,
                eStillImage,
                eFastDecode,
                eZeroLatency
            };
            QString toString( EMakeMKVTune preset );

            enum EMakeMKVProfile
            {
                eMain,
                eMainIntra,
                eMailStillPicture,
                eMain444_8,
                eMain444Intra,
                eMain444StillPicture,
                eMain10,
                eMain10Intra,
                eMain422_10,
                eMain422_10Intra,
                eMain444_10,
                eMain444_10Intra,
                eMain12,
                eMain12Intra,
                eMain422_12,
                eMain422_12Intra,
                eMain444_12,
                eMain444_12Intra
            };
            QString toString( EMakeMKVProfile profile );


            class CPreferences : public QObject
            {
                Q_OBJECT;

                CPreferences();

            public:
                static CPreferences *instance();
                virtual ~CPreferences() override;

                QString validateDefaults() const;
                void showValidateDefaults( QWidget *parent, bool showNoChange );

                void setDirectories( const QStringList &dirs );
                QStringList getDirectories() const;

                void setFileNames( const QStringList &fileNames );
                QStringList getFileNames() const;

                void setTreatAsTVShowByDefault( bool value );
                bool getTreatAsTVShowByDefault() const;


                QStringList getConvertToMKVArgs( bool sourceH265, const QString & srcName, const QString & destName ) const;
                void setConvertToH265( bool value );
                bool getConvertToH265() const;

                void setLosslessTranscoding( bool value );
                bool getLosslessTranscoding() const;

                void setUseCRF( bool value );
                bool getUseCRF() const;

                void setUseExplicitCRF( bool value );
                bool getUseExplicitCRF() const;

                void setExplicitCRF( int value );
                int getExplicitCRF() const;

                void setUsePreset( bool value );
                bool getUsePreset() const;

                void setPreset( EMakeMKVPreset value );
                EMakeMKVPreset getPreset() const;

                void setUseTune( bool value );
                bool getUseTune() const;

                void setTune( EMakeMKVTune value );
                EMakeMKVTune getTune() const;

                void setUseProfile( bool value );
                bool getUseProfile() const;

                void setProfile( EMakeMKVProfile value );
                EMakeMKVProfile getProfile() const;

                void setExactMatchesOnly( bool value );
                bool getExactMatchesOnly() const;

                void setLoadMediaInfo( bool value );
                bool getLoadMediaInfo() const;

                void setOnlyTransformDirectories( bool value );
                bool getOnlyTransformDirectories() const;

                void setSeasonOutDirPattern( const QString &value );
                QString getSeasonOutDirPattern() const;

                void setTVOutFilePattern( const QString &value );
                QString getTVOutFilePattern() const;

                void setTVOutDirPattern( const QString &value );
                QString getTVOutDirPattern( bool expandSeasonDir ) const;

                void setMovieOutFilePattern( const QString &value );
                QString getMovieOutFilePattern() const;

                void setMovieOutDirPattern( const QString &value );
                QString getMovieOutDirPattern() const;

                bool isSkippedPath( bool forMediaNaming, const QFileInfo &fileInfo ) const;
                QStringList getDefaultSkippedPaths( bool forMediaNaming ) const;
                QStringList getSkippedPaths( bool forMediaNaming ) const;
                void setSkippedPaths( bool forMediaNaming, const QStringList &value );

                void setIgnorePathNamesToSkip( bool forMediaNaming, bool value );
                bool getIgnorePathNamesToSkip( bool forMediaNaming ) const;

                bool isIgnoredPath( const QFileInfo &fileInfo ) const;
                QStringList getDefaultIgnoredPaths() const;
                QStringList getIgnoredPaths() const;
                void setIgnoredPaths( const QStringList &value );

                void setIgnorePathNamesToIgnore( bool value );
                bool getIgnorePathNamesToIgnore() const;

                std::list< std::pair< NSABUtils::EMediaTags, bool > > getAllMediaTags() const;
                std::list< NSABUtils::EMediaTags > getEnabledTags() const;
                QStringList getEnabledTagsForDisplay() const;
                void setEnabledTags( const std::list< NSABUtils::EMediaTags > &value );

                bool getVerifyMediaTags() const;
                void setVerifyMediaTags( bool value );

                bool getVerifyMediaTitle() const;
                void setVerifyMediaTitle( bool value );

                QString getVerifyMediaTitleExpr() const;
                QRegularExpression getVerifyMediaTitleExpr( const QFileInfo &fi, const QDate &date ) const;   // fills in the fields and returns a regularExpression
                void setVerifyMediaTitleExpr( const QString &value );

                bool getVerifyMediaDate() const;
                void setVerifyMediaDate( bool value );

                QString getVerifyMediaDateExpr() const;
                QRegularExpression getVerifyMediaDateExpr( const QFileInfo &fi, const QDate &date ) const;
                void setVerifyMediaDateExpr( const QString &value );

                bool getVerifyMediaComment() const;
                void setVerifyMediaComment( bool value );

                QString getVerifyMediaCommentExpr() const;
                QRegularExpression getVerifyMediaCommentExpr( const QFileInfo &fi, const QDate &date ) const;
                void setVerifyMediaCommentExpr( const QString &value );

                QStringList getDefaultCustomPathsToDelete() const;
                QStringList getCustomPathsToDelete() const;
                void setCustomPathsToDelete( const QStringList &paths );

                QString getDefaultRippedWithMakeMKVRegEX() const;
                QString getRippedWithMakeMKVRegEX() const;
                void setRippedWithMakeMKVRegEX( const QString &value );

                bool getDefaultDeleteCustom() const;
                bool deleteCustom() const;
                void setDeleteCustom( bool deleteCustom );

                bool getDefaultDeleteEXE() const;
                bool deleteEXE() const;
                void setDeleteEXE( bool deleteEXEs );

                bool getDefaultDeleteNFO() const;
                bool deleteNFO() const;
                void setDeleteNFO( bool deleteNFO );

                bool getDefaultDeleteBAK() const;
                bool deleteBAK() const;
                void setDeleteBAK( bool deleteBAK );

                bool getDefaultDeleteImages() const;
                bool deleteImages() const;
                void setDeleteImages( bool deleteImages );

                QStringList imageExtensions() const;

                bool deleteTXT() const;
                void setDeleteTXT( bool deleteTXT );

                QStringList getExtensionsToDelete() const;
                bool isPathToDelete( const QString &path ) const;

                void setMediaExtensions( const QString &value );
                void setMediaExtensions( const QStringList &value );
                QStringList getVideoExtensions() const;

                static QStringList defaultVideoExtensions( bool forceReset );

                void setSubtitleExtensions( const QString &value );
                void setSubtitleExtensions( const QStringList &value );
                QStringList getSubtitleExtensions() const;

                QStringList getNonMKVVideoExtensions() const;

                void addKnownStrings( const QStringList &value );
                void setKnownStrings( const QStringList &value );

                QStringList getDefaultKnownStrings() const;
                QStringList getKnownStrings() const;
                QStringList getKnownStringRegExs() const;

                void setKnownExtendedStrings( const QStringList &value );
                QStringList getDefaultKnownExtendedStrings() const;
                QStringList getKnownExtendedStrings() const;

                void setKnownAbbreviations( const QVariantMap &value );   // needs to be QString to QString
                void setKnownAbbreviations( const QList< QPair< QString, QString > > &value );
                QVariantMap getDefaultKnownAbbreviations() const;
                QVariantMap getKnownAbbreviations() const;

                void setKnownHyphenated( const QStringList &value );
                QStringList getDefaultKnownHyphenated() const;
                QStringList getKnownHyphenated() const;
                std::list< std::pair< QString, int > > getKnownHyphenatedData() const;

                void setMKVValidatorEXE( const QString & value );
                QString getMKVValidatorEXE() const;
                                
                void setMKVMergeEXE( const QString &value );
                QString getMKVMergeEXE() const;

                void setMKVPropEditEXE( const QString &value );
                QString getMKVPropEditEXE() const;

                void setFFMpegEXE( const QString &value );
                QString getFFMpegEXE() const;

                void setFFProbeEXE( const QString &value );
                QString getFFProbeEXE() const;

                bool hasIntelGPU() const;
                bool hasNVidiaGPU() const;

                void setIntelGPUTranscode( bool value );
                bool getIntelGPUTranscodeDefault() const;
                bool getIntelGPUTranscode() const;

                void setNVidiaGPUTranscode( bool value );
                bool getNVidiaGPUTranscodeDefault() const;
                bool getNVidiaGPUTranscode() const;

                void setSoftwareTranscode( bool value );
                bool getSoftwareTranscodeDefault() const;
                bool getSoftwareTranscode() const;

                bool isMediaFile( const QFileInfo &fi ) const;
                bool isSubtitleFile( const QFileInfo &info, bool *isLangFileFormat = nullptr ) const;

                void setBIFPlayerSpeedMultiplier( int speed );   // 100 = "full speed" ie 1x default is 200x
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
                void setColorForStatus( EItemStatus, bool background, const QColor &value );

                bool getPageVisible( const QString &pageName ) const;
                void setPageVisible( const QString &pageName, bool isVisible );

                int getNumSearchPages() const;
                void setNumSearchPages( int numpages );

                QSize getThumbnailSize( const QFileInfo &fi ) const;
                QString getImageFileName( const QFileInfo &fi, const QString &ext ) const;
                QString getImageFileName( const QFileInfo &fi, const QSize &sz, const QString &ext ) const;

                int imageInterval() const;
                void setImageInterval( int value );

                bool imageOriginalSize() const;
                void setImageOriginalSize( bool value );

                bool byImageWidth() const;
                void setByImageWidth( bool value );

                int imageWidth() const;
                void setImageWidth( int value );

                bool byImageHeight() const;
                void setByImageHeight( bool value );

                int imageHeight() const;
                void setImageHeight( int value );

                bool generateGIF() const;
                void setGenerateGIF( bool value );

                bool generateBIF() const;
                void setGenerateBIF( bool value );

                bool keepTempDir() const;
                void setKeepTempDir( bool value );

            Q_SIGNALS:
                void sigPreferencesChanged( EPreferenceTypes prefType );

            private:
                QStringList getDefaultFile() const;
                QString compareValues( const QString &title, const QStringList &defaultValues, const QStringList &currValues ) const;
                QString compareValues( const QString &title, const QString &defaultValues, const QString &currValues ) const;
                QString compareValues( const QString &title, bool defaultValue, bool currValue ) const;
                QString compareValues( const QString &title, const QVariantMap &defaultValues, const QVariantMap &currValues ) const;
                QStringList variantMapToStringList( const QVariantMap &data ) const;
                bool isFileWithExtension( const QFileInfo &fi, std::function< QStringList() > getExtensions, std::unordered_set< QString > &hash, std::unordered_map< QString, bool > &cache ) const;

                QStringList cleanUpPaths( const QStringList &paths, bool areDirs ) const;
                void emitSigPreferencesChanged( EPreferenceTypes prefType );
                bool pathMatches( const QFileInfo &fileInfo, const QStringList &values ) const;
                bool containsValue( const QString &value, const QStringList &values ) const;
                //QString getDefaultInPattern( bool forTV ) const;
                QString getDefaultSeasonDirPattern() const;
                QString getDefaultOutDirPattern( bool forTV ) const;
                QString getDefaultOutFilePattern( bool forTV ) const;
                QTimer *fPrefChangeTimer{ nullptr };
                EPreferenceTypes fPending;
                mutable std::optional< bool > fHasIntelGPU;
                mutable std::optional< bool > fHasNVidiaGPU;
                mutable std::unordered_set< QString > fMediaExtensionsHash;
                mutable std::unordered_map< QString, bool > fIsMediaExtension;

                mutable std::unordered_set< QString > fSubtitleExtensionsHash;
                mutable std::unordered_map< QString, bool > fIsSubtitleExtension;
                mutable QStringList fKnownStringRegExsCache;
            };
        }
    }
}
#endif
