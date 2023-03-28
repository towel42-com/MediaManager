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
#include <memory>
class QFileInfo;
class QWidget;

namespace NSABUtils
{
    class CMediaInfo;
    enum class EMediaTags;
    class CFFMpegFormats;
    enum class EFormatType;
    using TFormatMap = std::unordered_map< EFormatType, std::unordered_map< QString, QStringList > >;
    using TCodecToEncoderDecoderMap = std::unordered_map< EFormatType, std::unordered_multimap< QString, QString > >;
}

class QProgressDialog;
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
            eMediaRenamerPrefs = 8,
            eTagPrefs = 16,
            eExtToolsPrefs = 32,
            eBIFPrefs = 64,
            eGIFPrefs = 128,
            eTranscodePrefs = 256
        };
        Q_DECLARE_FLAGS( EPreferenceTypes, EPreferenceType );
        Q_DECLARE_OPERATORS_FOR_FLAGS( EPreferenceTypes );
        QString toString( EPreferenceType prefType, bool forEnum=false );

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
            QString toString( EMakeMKVPreset preset, bool forEnum = false );

            enum EMakeMKVTune
            {
                eFilm,
                eAnimation,
                eGrain,
                eStillImage,
                eFastDecode,
                eZeroLatency
            };
            QString toString( EMakeMKVTune preset, bool forEnum = false );

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
            QString toString( EMakeMKVProfile profile, bool forEnum = false );

            class CPreferences;
            struct STranscodeNeeded
            {
                STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const CPreferences *prefs );
                STranscodeNeeded( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo  );

                bool transcodeNeeded() const
                {
                    return fVideo || fAudio || fFormat;
                }
                bool formatOnly() const
                {
                    return fFormat && !fVideo && !fAudio;
                }
                bool fVideo{ false };
                bool fAudio{ false };
                bool fFormat{ false };
            };

            class CPreferences : public QObject
            {
                Q_OBJECT;

                CPreferences();

            public:
                static CPreferences *instance();
                virtual ~CPreferences() override;

                void recomputeSupportedFormats( QProgressDialog * dlg );
                QString validateDefaults() const;
                void showValidateDefaults( QWidget *parent, bool showNoChange );

                void setDirectories( const QStringList &dirs );
                QStringList getDirectories() const;

                void setFileNames( const QStringList &fileNames );
                QStringList getFileNames() const;

                void setTreatAsTVShowByDefault( bool value );
                bool getTreatAsTVShowByDefault() const;
                
                bool isEncoderFormat( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &formatName ) const;
                bool isDecoderFormat( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &formatName ) const;
                QStringList getTranscodeArgs( std::shared_ptr< NSABUtils::CMediaInfo > mediaInfo, const QString &srcName, const QString &destName ) const;
                QStringList getTranscodeArgs( const QString &srcName, const QString &destName ) const;

                std::shared_ptr< NSABUtils::CMediaInfo > getMediaInfo( const QFileInfo & fi );
                std::shared_ptr< NSABUtils::CMediaInfo > getMediaInfo( const QString & fileName );

                // ffmpeg results
                QStringList availableEncoderMediaFormats( bool verbose ) const;   // if true returns name - desc, otherwise name only
                NSABUtils::TFormatMap getEncoderFormatExtensionsMap() const;
                QStringList getExtensionsForEncoderMediaFormat( const QString &format ) const;

                QStringList availableDecoderMediaFormats( bool verbose ) const;   // if true returns name - desc, otherwise name only
                NSABUtils::TFormatMap getDecoderFormatExtensionsMap() const;
                QStringList getExtensionsForDecoderMediaFormat( const QString &format ) const;

                QStringList availableVideoEncodingCodecs( bool verbose ) const;   // if true returns name - desc, otherwise name only
                QStringList availableVideoDecodingCodecs( bool verbose ) const;   // if true returns name - desc, otherwise name only

                QStringList availableAudioEncodingCodecs( bool verbose ) const;   // if true returns name - desc, otherwise name only
                QStringList availableAudioDecodingCodecs( bool verbose ) const;   // if true returns name - desc, otherwise name only

                QStringList availableSubtitleEncodingCodecs( bool verbose ) const;   // if true returns name - desc, otherwise name only
                QStringList availableSubtitleDecodingCodecs( bool verbose ) const;   // if true returns name - desc, otherwise name only

                QStringList availableVideoEncoders( bool verbose ) const;   // if true returns name - desc, otherwise name only
                QStringList availableVideoDecoders( bool verbose ) const;   // if true returns name - desc, otherwise name only

                QStringList availableAudioEncoders( bool verbose ) const;   // if true returns name - desc, otherwise name only
                QStringList availableAudioDecoders( bool verbose ) const;   // if true returns name - desc, otherwise name only

                QStringList availableSubtitleEncoders( bool verbose ) const;   // if true returns name - desc, otherwise name only
                QStringList availableSubtitleDecoders( bool verbose ) const;   // if true returns name - desc, otherwise name only

                NSABUtils::TCodecToEncoderDecoderMap getCodecToEncoderMap() const;
                NSABUtils::TCodecToEncoderDecoderMap getCodecToDecoderMap() const;

                QStringList availableHWAccels( bool verbose ) const;   // if true returns name - desc, otherwise name only

                QStringList availableMediaEncoderFormatsStatic( bool verbose ) const;
                NSABUtils::TFormatMap getEncoderFormatExtensionsMapStatic() const;

                QStringList availableMediaDecoderFormatsStatic( bool verbose ) const;
                NSABUtils::TFormatMap getDecoderFormatExtensionsMapStatic() const;

                QStringList availableVideoEncodingCodecsStatic( bool verbose ) const;
                QStringList availableVideoDecodingCodecsStatic( bool verbose ) const;

                QStringList availableAudioEncodingCodecsStatic( bool verbose ) const;
                QStringList availableAudioDecodingCodecsStatic( bool verbose ) const;

                QStringList availableSubtitleEncodingCodecsStatic( bool verbose ) const;
                QStringList availableSubtitleDecodingCodecsStatic( bool verbose ) const;

                QStringList availableVideoEncodersStatic( bool verbose ) const;
                QStringList availableVideoDecodersStatic( bool verbose ) const;

                QStringList availableAudioEncodersStatic( bool verbose ) const;
                QStringList availableAudioDecodersStatic( bool verbose ) const;

                QStringList availableSubtitleEncodersStatic( bool verbose ) const;
                QStringList availableSubtitleDecodersStatic( bool verbose ) const;

                NSABUtils::TCodecToEncoderDecoderMap getCodecToEncoderMapStatic() const;
                NSABUtils::TCodecToEncoderDecoderMap getCodecToDecoderMapStatic() const;

                QStringList availableHWAccelsStatic( bool verbose ) const;

                // container transcode arguments
                void setConvertMediaContainer( bool value );
                bool getConvertMediaContainerDefault() const;
                bool getConvertMediaContainer() const;

                void setConvertMediaToContainer( const QString &value );
                QString getConvertMediaToContainer() const;
                QString getConvertMediaToContainerDefault() const;

                QString getMediaContainerExt() const;

                // audio codec transcode arguments
                void setTranscodeAudio( bool value );
                bool getTranscodeAudioDefault() const;
                bool getTranscodeAudio() const;

                void setAlwaysAddAACAudioCodec( bool value );
                bool getAlwaysAddAACAudioCodecDefault() const;
                bool getAlwaysAddAACAudioCodec() const;

                void setOnlyTranscodeAudioOnFormatChange( bool value );
                bool getOnlyTranscodeAudioOnFormatChangeDefault() const;
                bool getOnlyTranscodeAudioOnFormatChange() const;

                void setTranscodeToAudioCodec( const QString &value );
                QString getTranscodeToAudioCodecDefault() const;
                QString getTranscodeToAudioCodec() const;

                // video codec transcode arguments
                void setTranscodeVideo( bool value );
                bool getTranscodeVideoDefault() const;
                bool getTranscodeVideo() const;

                QString getTranscodeHWAccel() const;
                QString getTranscodeHWAccel( const QString &codec ) const;
                QString getCodecForHWAccel( const QString &codec ) const;
                
                void setOnlyTranscodeVideoOnFormatChange( bool value );
                bool getOnlyTranscodeVideoOnFormatChangeDefault() const;
                bool getOnlyTranscodeVideoOnFormatChange() const;

                void setTranscodeToVideoCodec( const QString &value );
                QString getTranscodeToVideoCodecDefault() const;
                QString getTranscodeToVideoCodec() const;

                void setLosslessEncoding( bool value );
                bool getLosslessEncodingDefault() const;
                bool getLosslessEncoding() const;

                void setUseCRF( bool value );
                bool getUseCRFDefault() const;
                bool getUseCRF() const;

                void setUseExplicitCRF( bool value );
                bool getUseExplicitCRFDefault() const;
                bool getUseExplicitCRF() const;

                void setExplicitCRF( int value );
                int getExplicitCRFDefault() const;
                int getExplicitCRF() const;

                void setUsePreset( bool value );
                bool getUsePresetDefault() const;
                bool getUsePreset() const;

                void setPreset( EMakeMKVPreset value );
                EMakeMKVPreset getPresetDefault() const;
                EMakeMKVPreset getPreset() const;

                void setUseTune( bool value );
                bool getUseTuneDefault() const;
                bool getUseTune() const;

                void setTune( EMakeMKVTune value );
                EMakeMKVTune getTuneDefault() const;
                EMakeMKVTune getTune() const;

                void setUseProfile( bool value );
                bool getUseProfileDefault() const;
                bool getUseProfile() const;

                void setProfile( EMakeMKVProfile value );
                EMakeMKVProfile getProfileDefault() const;
                EMakeMKVProfile getProfile() const;

                void setExactMatchesOnly( bool value );
                bool getExactMatchesOnly() const;

                void setLoadMediaInfo( bool value );
                bool getLoadMediaInfo() const;

                void setBackgroundLoadMediaInfo( bool value );
                bool getBackgroundLoadMediaInfo() const;

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

                QStringList getVideoEncoderExtensions() const;
                QStringList getAudioEncoderExtensions() const;
                QStringList getSubtitleEncoderExtensions() const;

                QStringList getVideoDecoderExtensions() const;
                QStringList getAudioDecoderExtensions() const;
                QStringList getSubtitleDecoderExtensions() const;

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

                void setUseCustomExternalTools( bool value );
                bool getUseCustomExternalTools() const;
                QString  getExternalToolPath( const QString &settingsLoc, const QString &exe, const QString &defaultPath ) const;

                QString getMKVValidatorEXE() const;

                void setMKVMergeEXE( const QString &value );
                QString getMKVMergeEXE() const;

                void setMKVPropEditEXE( const QString &value );
                QString getMKVPropEditEXE() const;

                void setFFMpegEXE( const QString &value );
                QString getFFMpegEXE() const;

                void setFFMpegEmbyEXE( const QString &value );
                QString getFFMpegEmbyEXE() const;

                void setFFProbeEXE( const QString &value );
                QString getFFProbeEXE() const;

                bool hasIntelGPU() const;
                bool hasNVidiaGPU() const;
                bool hasAMDGPU() const;
                int getGPUCount() const;

                NSABUtils::CFFMpegFormats *getMediaFormats() const;

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

            private Q_SLOTS:
                void slotMediaInfoLoaded( const QString &fileName );
            Q_SIGNALS:
                void sigPreferencesChanged( EPreferenceTypes prefType );
                void sigMediaInfoLoaded( const QString &fileName ) const;

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

                void loadMediaFormats( bool forceFromFFMpeg, QProgressDialog * dlg = nullptr ) const;

                QTimer *fPrefChangeTimer{ nullptr };
                EPreferenceTypes fPending;

                mutable std::unique_ptr< NSABUtils::CFFMpegFormats > fMediaFormats;

                mutable std::unordered_set< QString > fMediaExtensionsHash;
                mutable std::unordered_map< QString, bool > fIsMediaExtension;

                mutable std::optional< bool > fHasIntelGPU;
                mutable std::optional< bool > fHasNVidiaGPU;
                mutable std::optional< bool > fHasAMDGPU;

                mutable std::unordered_set< QString > fSubtitleExtensionsHash;
                mutable std::unordered_map< QString, bool > fIsSubtitleExtension;
                mutable QStringList fKnownStringRegExsCache;

                std::unordered_map< QString, std::shared_ptr< NSABUtils::CMediaInfo > > fQueuedMediaInfo;
            };
        }
    }
}
#endif
