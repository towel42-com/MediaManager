# The MIT License (MIT)
#
# Copyright (c) 2020-2023 Scott Aron Bloom
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

set(_PROJECT_NAME PreferencesUI)
set(USE_QT TRUE)
set(FOLDER_NAME Libs/Preferences)

set(qtproject_SRCS
    BasePrefPage.cpp
    BIFGeneration.cpp
    ExtendedInfo.cpp
    FFMpegInfo.cpp
    ExternalTools.cpp
    GeneralSettings.cpp
    IgnoredPaths.cpp
    KnownAbbreviations.cpp
    KnownHyphenated.cpp
    MovieSettings.cpp
    TranscodeGeneralSettings.cpp
    TranscodeVideoCodec.cpp
    TranscodeVideoQuality.cpp
    TranscodeVideoSettings.cpp
    TranscodeAudioSettings.cpp
    PathsToDelete.cpp
    Preferences.cpp
    RemoveFromPaths.cpp
    SearchSettings.cpp
    SkippedPaths.cpp
    TagAnalysisSettings.cpp
    MediaNamingSettings.cpp
    QualityExperiment.cpp
    TVShowSettings.cpp
)

set(qtproject_H
    BasePrefPage.h
    BIFGeneration.h
    ExtendedInfo.h
    FFMpegInfo.h
    GeneralSettings.h
    ExternalTools.h
    IgnoredPaths.h
    KnownAbbreviations.h
    KnownHyphenated.h
    TranscodeGeneralSettings.h
    TranscodeVideoCodec.h
    TranscodeVideoQuality.h
    TranscodeVideoSettings.h
    TranscodeAudioSettings.h
    MovieSettings.h
    PathsToDelete.h
    Preferences.h
    RemoveFromPaths.h
    SearchSettings.h
    SkippedPaths.h
    TagAnalysisSettings.h
    MediaNamingSettings.h
    QualityExperiment.h
    TVShowSettings.h
)

set(project_H
)

set(qtproject_UIS
    BIFGeneration.ui
    ExtendedInfo.ui
    FFMpegInfo.ui
    ExternalTools.ui
    GeneralSettings.ui
    IgnoredPaths.ui
    KnownAbbreviations.ui
    KnownHyphenated.ui
    TranscodeGeneralSettings.ui
    TranscodeVideoCodec.ui
    TranscodeVideoQuality.ui
    TranscodeVideoSettings.ui
    TranscodeAudioSettings.ui
    MovieSettings.ui
    PathsToDelete.ui
    Preferences.ui
    RemoveFromPaths.ui
    SearchSettings.ui
    SkippedPaths.ui
    TagAnalysisSettings.ui
    MediaNamingSettings.ui
    QualityExperiment.ui
    TVShowSettings.ui   
)

set(qtproject_QRC
)

file(GLOB qtproject_QRC_SOURCES "resources/*")

