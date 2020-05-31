
CONFIGURE_FILE("qtappfw-voice.pc.in" "qtappfw-voice.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-voice.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-voice SHARED voice.cpp
                                 voiceagentregistry.cpp
                                 voiceagentprofile.cpp
                                 voiceagentmodel.cpp)

target_include_directories(qtappfw-voice PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-voice PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-voice qtappfw-core)
set_target_properties(qtappfw-voice PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "voice.h;voiceagentregistry.h;voiceagentprofile.h;voiceagentmodel.h")

install(TARGETS qtappfw-voice
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-voice)
