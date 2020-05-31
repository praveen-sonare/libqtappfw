
CONFIGURE_FILE("qtappfw-network.pc.in" "qtappfw-network.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-network.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-network SHARED network.cpp
                              wifiadapter.cpp
                              wiredadapter.cpp
                              wifinetworkmodel.cpp
                              wirednetworkmodel.cpp
                              abstractnetworkmodel.cpp
                              connectionprofile.cpp)

target_include_directories(qtappfw-network PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-network PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-network qtappfw-core)
set_target_properties(qtappfw-network PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "network.h;networkadapter.h;wifiadapter.h;wiredadapter.h;wifinetworkmodel.h;wirednetworkmodel.h;abstractnetworkmodel.h;connectionprofile.h")

install(TARGETS qtappfw-network
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-network)
