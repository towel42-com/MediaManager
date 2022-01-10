# The MIT License (MIT)
#
# Copyright (c) 2020-2021 Scott Aron Bloom
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

set(qtproject_SRCS
    DirModel.cpp
    MergeSRTModel.cpp
    MakeMKVModel.cpp
    TransformModel.cpp
    LanguageInfo.cpp
    NetworkReply.cpp
    Preferences.cpp
    TransformResult.cpp
    SearchTMDB.cpp
    SearchTMDBInfo.cpp
)

set(qtproject_H
    SearchTMDB.h
    DirModel.h
    MergeSRTModel.h
    MakeMKVModel.h
    TransformModel.h
)

set(project_H
    LanguageInfo.h
    NetworkReply.h
    Preferences.h
    TransformResult.h
    SearchTMDBInfo.h
)

set(qtproject_UIS
)


set(qtproject_QRC
    core.qrc
)

file(GLOB qtproject_QRC_SOURCES "resources/*")

