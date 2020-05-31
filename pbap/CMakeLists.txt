
CONFIGURE_FILE("qtappfw-bt-pbap.pc.in" "qtappfw-bt-pbap.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-bt-pbap.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-bt-pbap SHARED pbap.cpp)

target_include_directories(qtappfw-bt-pbap PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-bt-pbap PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-bt-pbap qtappfw-core)
set_target_properties(qtappfw-bt-pbap PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER pbap.h)

install(TARGETS qtappfw-bt-pbap
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-bt-pbap)
