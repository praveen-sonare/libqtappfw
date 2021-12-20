cmake_minimum_required(VERSION 3.7.2)
project(qtappfw VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

set(OE_QMAKE_PATH_EXTERNAL_HOST_BINS $ENV{OE_QMAKE_PATH_HOST_BINS})

find_package(Qt5Qml REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(glib REQUIRED IMPORTED_TARGET glib-2.0)
pkg_check_modules(bluez_glib REQUIRED IMPORTED_TARGET bluez-glib)
#pkg_check_modules(connman_glib REQUIRED IMPORTED_TARGET connman-glib)

include(GNUInstallDirs)

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS "${PRIVATE_LIBS} -lqtappfw-bt
                                  -lqtappfw-hvac
                                  -lqtappfw-navigation
                                  -lqtappfw-network
                                  -lqtappfw-weather")

set (SUBDIRS
	docs
	bluetooth
	hvac
	navigation
	network
	weather)

foreach(subdir ${SUBDIRS})
	add_subdirectory(${subdir})
endforeach()
