project("mkvalidator" VERSION 0.6.0)

add_executable("mkvalidator" mkvalidator.c)

configure_file(mkvalidator_project.h.in mkvalidator_project.h)
target_include_directories("mkvalidator" PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries("mkvalidator" PUBLIC "matroska2" "ebml2" "corec")

# Source packaging script
configure_file(pkg.sh.in pkg.sh)
configure_file(src.br.in src.br)
configure_file(CMakeLists.txt.in CMakeLists.txt)

INSTALL( TARGETS mkvalidator RUNTIME DESTINATION . )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/mkvalidator.pdb DESTINATION . CONFIGURATIONS Debug RelWithDebInfo )

add_custom_command(TARGET mkvalidator POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E echo "Copying mkvalidator '$<TARGET_FILE:mkvalidator>' to '$<TARGET_FILE_DIR:MediaManager>/$<TARGET_FILE_NAME:mkvalidator>'"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different \"$<TARGET_FILE:mkvalidator>\" \"$<TARGET_FILE_DIR:MediaManager>/$<TARGET_FILE_NAME:mkvalidator>\"
)
 