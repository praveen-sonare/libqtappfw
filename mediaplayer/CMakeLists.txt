
CONFIGURE_FILE("qtappfw-mediaplayer.pc.in" "qtappfw-mediaplayer.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-mediaplayer.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-mediaplayer SHARED
	mediaplayer.cpp
	MediaplayerBackend.cpp
	MediaplayerMpdBackend.cpp
	MediaplayerBluezBackend.cpp
	MpdEventHandler.cpp)

target_include_directories(qtappfw-mediaplayer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-mediaplayer PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-mediaplayer qtappfw-bt Qt5::Qml PkgConfig::libmpdclient)
set_target_properties(qtappfw-mediaplayer PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER mediaplayer.h)

install(TARGETS qtappfw-mediaplayer
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-mediaplayer)
