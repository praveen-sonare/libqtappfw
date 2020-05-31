
CONFIGURE_FILE("qtappfw-signal-composer.pc.in" "qtappfw-signal-composer.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-signal-composer.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-signal-composer SHARED signalcomposer.cpp)

target_include_directories(qtappfw-signal-composer PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-signal-composer PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-signal-composer qtappfw-core)
set_target_properties(qtappfw-signal-composer PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER signalcomposer.h)

install(TARGETS qtappfw-signal-composer
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-signal-composer)
