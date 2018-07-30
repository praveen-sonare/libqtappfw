cmake_minimum_required(VERSION 3.7.2)
project(libqtappfw VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt5Qml REQUIRED)
find_package(Qt5WebSockets REQUIRED)

include(GNUInstallDirs)

macro (add_headers)
	file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
	foreach (_hdr ${ARGN})
		if (_relPath)
			list (APPEND HDRS "${_relPath}/${_hdr}")
		else()
			list (APPEND HDRS "${_hdr}")
		endif()
	endforeach()
	if (_relPath)
		set (HDRS ${HDRS} PARENT_SCOPE)
	endif()
endmacro()

macro (add_sources)
	file (RELATIVE_PATH _relPath "${PROJECT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
	foreach (_src ${ARGN})
		if (_relPath)
			list (APPEND SRCS "${_relPath}/${_src}")
		else()
			list (APPEND SRCS "${_src}")
		endif()
	endforeach()
	if (_relPath)
		set (SRCS ${SRCS} PARENT_SCOPE)
	endif()
endmacro()

set(DEST_DIR "${CMAKE_INSTALL_PREFIX}")
set(PRIVATE_LIBS "${PRIVATE_LIBS} -lqtappfw")
CONFIGURE_FILE("qtappfw.pc.in" "qtappfw.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

set (SUBDIRS
	bluetooth
	mediaplayer
	network
	pbap
	telephony
	weather)

add_headers(message.h messageengine.h responsemessage.h)
add_sources(message.cpp messageengine.cpp responsemessage.cpp)

foreach(subdir ${SUBDIRS})
	add_subdirectory(${subdir})
endforeach()

add_library(qtappfw SHARED ${SRCS})

target_link_libraries(qtappfw Qt5::WebSockets vcard)
set_target_properties(qtappfw PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "${HDRS}")

target_include_directories(qtappfw PRIVATE .  ${SUBDIRS})

install(TARGETS qtappfw
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw)
