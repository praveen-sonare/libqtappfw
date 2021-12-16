
CONFIGURE_FILE("qtappfw-hvac.pc.in" "qtappfw-hvac.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-hvac.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-hvac SHARED hvac.cpp)

target_include_directories(qtappfw-hvac PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-hvac PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-hvac Qt5::Core)
set_target_properties(qtappfw-hvac PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER hvac.h)

install(TARGETS qtappfw-hvac
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-hvac)
