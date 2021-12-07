# The MIT License (MIT)
#
# Copyright (c) 2020 Scott Aron Bloom
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

project( MediaManager ) 

include( include.cmake )
include( ${CMAKE_SOURCE_DIR}/SABUtils/Project.cmake )

add_executable( MediaManager WIN32
                 ${project_SRCS} 
                 ${project_H} 
                 ${qtproject_SRCS} 
                 ${qtproject_QRC} 
                 ${qtproject_QRC_SRCS} 
                 ${qtproject_UIS_H} 
                 ${qtproject_MOC_SRCS} 
                 ${qtproject_H} 
                 ${qtproject_UIS}
                 ${qtproject_QRC_SOURCES}
                 ${_CMAKE_FILES}
                 ${_CMAKE_MODULE_FILES}
                application.rc 
          )
set_target_properties( MediaManager PROPERTIES FOLDER Apps )

target_link_libraries( MediaManager 
                 Qt5::Widgets
                 Qt5::Core
                 Qt5::Multimedia
                 SABUtils
                 UI
                 Core
          )

DeployQt( MediaManager . )
DeploySystem( MediaManager . )

INSTALL( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION . )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/Debug/${PROJECT_NAME}.pdb DESTINATION . CONFIGURATIONS Debug )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/RelWithDebInfo/${PROJECT_NAME}.pdb DESTINATION . CONFIGURATIONS Debug RelWithDebInfo )

