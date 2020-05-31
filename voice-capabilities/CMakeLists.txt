
CONFIGURE_FILE("qtappfw-voice-metadata.pc.in" "qtappfw-voice-metadata.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-voice-metadata.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-voice-metadata SHARED guimetadata.cpp)

target_include_directories(qtappfw-voice-metadata PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-voice-metadata PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-voice-metadata qtappfw-core)
set_target_properties(qtappfw-voice-metadata PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER guimetadata.h)

install(TARGETS qtappfw-voice-metadata
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-voice-metadata)
