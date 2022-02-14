
CONFIGURE_FILE("qtappfw-bt-map.pc.in" "qtappfw-bt-map.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-bt-map.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-bt-map SHARED map.cpp)

target_include_directories(qtappfw-bt-map PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-bt-map PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-bt-map Qt5::Qml)
set_target_properties(qtappfw-bt-map PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER map.h)

install(TARGETS qtappfw-bt-map
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-bt-map)
