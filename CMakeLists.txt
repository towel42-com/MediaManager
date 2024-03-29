# The MIT License (MIT)
#
# Copyright (c) 2020-2022 Scott Aron Bloom
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

cmake_minimum_required(VERSION 3.22)
#cmake_policy(SET CMP0126 NEW)
#cmake_policy(SET CMP0077 NEW)

project(root-MediaManager) 

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/SABUtils/Modules ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
find_package(Threads)
find_package(Qt5 COMPONENTS Core Widgets Multimedia REQUIRED)

find_package(Deploy REQUIRED)
find_package(CheckOpenSSL REQUIRED)
find_package(AddUnitTest REQUIRED)

include( VersionInfo.cmake )

CheckOpenSSL()

option( gtest_force_shared_crt "Use shared ( DLL ) run-time lib even when Google Test is built as static lib." ON ) 

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

file( REAL_PATH ~/Dropbox/home/bin/MediaManager CMAKE_INSTALL_PREFIX EXPAND_TILDE)
MESSAGE( STATUS "Installing to ${CMAKE_INSTALL_PREFIX}" )

SET( SABUTILS_DLL ON )
SET( MKVUTILS ON )
SET( GIFSUPPORT ON )
SET( BIFSUPPORT ON )
SET( SAB_ENABLE_TESTING ON )

add_subdirectory( SABUtils )

add_subdirectory( SABUtils/bif)
add_subdirectory( SABUtils/gif)
add_subdirectory( UI )
add_subdirectory( Preferences/UI )
add_subdirectory( Preferences/Core )
add_subdirectory( Core )
add_subdirectory( Models )
add_subdirectory( App )
add_subdirectory( mkvalidator-0.6.0 )
                         
set_target_properties( branch      PROPERTIES FOLDER MKVValidator )
set_target_properties( bzlib       PROPERTIES FOLDER MKVValidator )
set_target_properties( corec       PROPERTIES FOLDER MKVValidator )
set_target_properties( ebml2       PROPERTIES FOLDER MKVValidator )
set_target_properties( ebmltree    PROPERTIES FOLDER MKVValidator )
set_target_properties( file_test   PROPERTIES FOLDER MKVValidator )
set_target_properties( matroska2   PROPERTIES FOLDER MKVValidator )
set_target_properties( minilzo     PROPERTIES FOLDER MKVValidator )
set_target_properties( mkvalidator PROPERTIES FOLDER MKVValidator )
set_target_properties( mkvtree     PROPERTIES FOLDER MKVValidator )
set_target_properties( node_test   PROPERTIES FOLDER MKVValidator )
set_target_properties( string_test PROPERTIES FOLDER MKVValidator )
set_target_properties( testmini    PROPERTIES FOLDER MKVValidator )
set_target_properties( zlib        PROPERTIES FOLDER MKVValidator )

#set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MediaManager ) 

include( InstallerInfo.cmake )

