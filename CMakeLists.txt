cmake_minimum_required(VERSION 3.7.2)
project(libqtappfw VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5WebSockets REQUIRED)

include(GNUInstallDirs)

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS "${PRIVATE_LIBS} -lqtappfw")
CONFIGURE_FILE("qtappfw.pc.in" "qtappfw.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw SHARED message.cpp messageengine.cpp
	responsemessage.cpp
	bluetooth.cpp bluetoothmessage.cpp
	mediaplayer.cpp mediaplayermessage.cpp
	pbap.cpp pbapmessage.cpp
	telephony.cpp telephonymessage.cpp
	weather.cpp weathermessage.cpp)
target_link_libraries(qtappfw Qt5::WebSockets vcard)
set_target_properties(qtappfw PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "message.h;messageengine.h;bluetooth.h;bluetoothmessage.h;mediaplayer.h;mediaplayermessage.h;pbap.h;pbapmessage.h;responsemessage.h;telephony.h;telephonymessage.h;weather.h;weathermessage.h")
target_include_directories(qtappfw PRIVATE .)
install(TARGETS qtappfw
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw)
