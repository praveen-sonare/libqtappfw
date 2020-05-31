cmake_minimum_required(VERSION 3.7.2)
project(qtappfw VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS $ENV{OE_QMAKE_PATH_HOST_BINS})

find_package(Qt5Qml REQUIRED)
find_package(Qt5WebSockets REQUIRED)

include(GNUInstallDirs)

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS "${PRIVATE_LIBS} -lqtappfw-core
                                  -lqtappfw-bt
                                  -lqtappfw-hvac
                                  -lqtappfw-bt-map
                                  -lqtappfw-mediaplayer
                                  -lqtappfw-navigation
                                  -lqtappfw-network
                                  -lqtappfw-bt-pbap
                                  -lqtappfw-radio
                                  -lqtappfw-signal-composer
                                  -lqtappfw-phone
                                  -lqtappfw-voice
                                  -lqtappfw-voice-metadata
                                  -lqtappfw-weather")


set (SUBDIRS
	core
	docs
	bluetooth
	hvac
	map
	mediaplayer
	navigation
	network
	pbap
	radio
	signal-composer
	telephony
	voice
	voice-capabilities
	weather)

foreach(subdir ${SUBDIRS})
	add_subdirectory(${subdir})
endforeach()
