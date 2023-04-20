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

cmake_minimum_required(VERSION 3.22)
 
find_package(IncludeProjectSettings REQUIRED)
include( ${CMAKE_CURRENT_LIST_DIR}/include.cmake )
project( ${_PROJECT_NAME} ) 
IncludeProjectSettings(QT ${USE_QT})

include_directories( ${CMAKE_BINARY_DIR} )

add_executable( ${PROJECT_NAME} WIN32
                ${_PROJECT_DEPENDENCIES} 
                ${_CMAKE_MODULE_FILES}
                application.rc 
          )
if ( NOT DEPLOYQT_EXECUTABLE )
    message( FATAL_ERROR "DEPLOYQT_EXECUTABLE not set" )
endif()

if ( NOT OPENSSL_DEPLOY_BINDIR )
    message( FATAL_ERROR "OPENSSL_DEPLOY_BINDIR not set" )
endif()

get_filename_component( QTDIR ${DEPLOYQT_EXECUTABLE} DIRECTORY )

set ( DEBUG_PATH 
        "%PATH%"
        "$<TARGET_FILE_DIR:SABUtils>"
        "${QTDIR}"
        )
if ( MKVUTILS )
    set ( DEBUG_PATH 
        ${DEBUG_PATH}
        "$<TARGET_FILE_DIR:mediainfo>"
    )
endif()

set ( DEBUG_PLUGINPATH 
        "$<TARGET_FILE_DIR:bif>/.."
        "$<TARGET_FILE_DIR:gif>/.."
        )
        
set_target_properties( ${PROJECT_NAME} PROPERTIES 
                                    FOLDER ${FOLDER_NAME} 
                                    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>" 
                                    VS_DEBUGGER_COMMAND "$<TARGET_FILE:${PROJECT_NAME}>" 
                                    VS_DEBUGGER_ENVIRONMENT "PATH=${DEBUG_PATH}\nQT_PLUGIN_PATH=${DEBUG_PLUGINPATH}" 
                     )


target_link_libraries( ${PROJECT_NAME}
    PUBLIC
        ${project_pub_DEPS}
    PRIVATE 
        ${project_pri_DEPS}
)

DeployQt( ${PROJECT_NAME} . )
DeploySystem( ${PROJECT_NAME} . INSTALL_ONLY 1 )
DeployOpenSSL( ${PROJECT_NAME} . )

INSTALL( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION . )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/${PROJECT_NAME}.pdb DESTINATION . CONFIGURATIONS Debug RelWithDebInfo )

file( REAL_PATH ~/Dropbox/home/bin BINDIR EXPAND_TILDE)
file( REAL_PATH $ENV{APPDATA} APPDATA EXPAND_TILDE)
file( REAL_PATH $ENV{PROGRAMFILES} PROGRAMFILES EXPAND_TILDE)

find_program( FFMPEG_EXEC ffmpeg.exe
    PATHS "${BINDIR}/ffmpeg/bin"
    REQUIRED )
find_program( FFPROBE_EXEC ffprobe.exe
    PATHS "${BINDIR}/ffmpeg/bin"
    REQUIRED )
find_program( FFMPEG_EMBY_EXEC ffmpeg.exe
    PATHS "${APPDATA}/Emby-Server/system"
    )
find_program( MKVPROPEDIT_EXEC mkvpropedit.exe
    PATHS "${PROGRAMFILES}/MKVToolNix"
    REQUIRED )

INSTALL( FILES "${FFMPEG_EXEC}" "${FFPROBE_EXEC}" "${MKVPROPEDIT_EXEC}"  DESTINATION . )

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E echo "Copying External Executables to '$<TARGET_FILE_DIR:MediaManager>'"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different \"${FFMPEG_EXEC}\" \"${FFPROBE_EXEC}\" \"${MKVPROPEDIT_EXEC}\" \"$<TARGET_FILE_DIR:MediaManager>\"
)

if( FFMPEG_EMBY_EXEC )
    INSTALL( FILES "${FFMPEG_EMBY_EXEC}"  DESTINATION . RENAME ffmpeg-emby.exe )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E echo "Copying External Executables to '$<TARGET_FILE_DIR:MediaManager>'"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different \"${FFMPEG_EMBY_EXEC}\" \"$<TARGET_FILE_DIR:MediaManager>/ffmpeg-emby.exe\"
    )
endif()
 
