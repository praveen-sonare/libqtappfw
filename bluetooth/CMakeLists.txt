
CONFIGURE_FILE("qtappfw-bt.pc.in" "qtappfw-bt.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-bt.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-bt SHARED bluetooth.cpp bluetoothmodel.cpp)

target_include_directories(qtappfw-bt PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-bt PRIVATE "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-bt qtappfw-core)
set_target_properties(qtappfw-bt PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "bluetooth.h;bluetoothmodel.h")

install(TARGETS qtappfw-bt
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-bt)
