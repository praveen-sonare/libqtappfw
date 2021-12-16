
CONFIGURE_FILE("qtappfw-weather.pc.in" "qtappfw-weather.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-weather.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-weather SHARED weather.cpp)

target_include_directories(qtappfw-weather PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-weather PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-weather Qt5::Qml)
set_target_properties(qtappfw-weather PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER weather.h)

install(TARGETS qtappfw-weather
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-weather)
