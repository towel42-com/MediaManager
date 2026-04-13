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

cmake_minimum_required(VERSION 3.31)
#cmake_policy(SET CMP0126 NEW)
#cmake_policy(SET CMP0077 NEW)

set( MAIN_EXE_PROJECT_NAME "MediaManager" )
set( APP_NAME              "Media Manager" )
set( MAJOR_VERSION         1 )
set( MINOR_VERSION        35 )
set( PATCH_VERSION        3 )

SET_PROPERTY(GLOBAL PROPERTY MAIN_EXE_PROJECT_NAME ${MAIN_EXE_PROJECT_NAME})
project(Root-${MAIN_EXE_PROJECT_NAME} VERSION ${MAJOR_VERSION}.${MINOR_VERSION})

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/T42-CMakeUtils/Modules ${CMAKE_MODULE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)
find_package(Threads)
find_package(Qt6 COMPONENTS Core Widgets Multimedia REQUIRED)

find_package(Deploy REQUIRED)
find_package(CheckOpenSSL REQUIRED)
find_package(AddUnitTest REQUIRED)
find_package(LoadVersionInfo REQUIRED)

LoadVersionInfoFile()

CheckOpenSSL( 3 )

option( gtest_force_shared_crt "Use shared ( DLL ) run-time lib even when Google Test is built as static lib." ON ) 

set_property( GLOBAL PROPERTY USE_FOLDERS ON )

file( REAL_PATH ~/Dropbox/home/bin/${MAIN_EXE_PROJECT_NAME}  CMAKE_INSTALL_PREFIX EXPAND_TILDE)
MESSAGE( STATUS "Installing to ${CMAKE_INSTALL_PREFIX}" )

SET(TOWEL42_MEDIAUTILS_DLL ON)
SET(TOWEL42_MKVUTILS ON)

SET(TOWEL42_UTILS_DLL ON)
SET(TOWEL42_GIFSUPPORT ON)
SET(TOWEL42_BIFSUPPORT ON)
SET(TOWEL42_QNETWORK_SUPPORT ON)
SET(TOWEL42_QCORE_SUPPORT ON)
SET(TOWEL42_QWIDGET_SUPPORT ON)
SET(TOWEL42_ENABLE_TESTING ON)

add_subdirectory( T42-Utils )

add_subdirectory( T42-Utils/bif )
add_subdirectory( T42-Utils/gif )
add_subdirectory( T42-MediaUtils )

add_compile_definitions( 
    $<$<BOOL:TOWEL42_UTILS_DLL>:TOWEL42_UTILS_DLL>
    $<$<BOOL:TOWEL42_MEDIAUTILS_DLL>:TOWEL42_MEDIAUTILS_DLL>
)

add_subdirectory( UI )
add_subdirectory( Preferences/UI )
add_subdirectory( Preferences/Core )
add_subdirectory( Core )
add_subdirectory( Models )
add_subdirectory( mkvalidator-0.6.0 )

add_subdirectory( App )
                         
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

#set_property( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${MAIN_EXE_PROJECT_NAME} ) 

include( InstallerInfo.cmake )

