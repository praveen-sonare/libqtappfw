
CONFIGURE_FILE("qtappfw-vehicle-signals.pc.in" "qtappfw-vehicle-signals.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-vehicle-signals.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-vehicle-signals SHARED vehiclesignals.cpp)

target_include_directories(qtappfw-vehicle-signals PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-vehicle-signals PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-vehicle-signals Qt5::Core Qt5::WebSockets)
set_target_properties(qtappfw-vehicle-signals PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER vehiclesignals.h)

install(TARGETS qtappfw-vehicle-signals
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-vehicle-signals)
