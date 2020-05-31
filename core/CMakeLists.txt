
CONFIGURE_FILE("qtappfw-core.pc.in" "qtappfw-core.pc" @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qtappfw-core.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)

add_library(qtappfw-core SHARED message.cpp
                                messagefactory.cpp
                                messageengine.cpp
                                messageenginefactory.cpp
                                responsemessage.cpp
                                callmessage.cpp
                                eventmessage.cpp)

target_include_directories(qtappfw-core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(qtappfw-core PUBLIC "${CMAKE_INSTALL_INCLUDEDIR}")

target_link_libraries(qtappfw-core Qt5::WebSockets)

set_target_properties(qtappfw-core PROPERTIES
	VERSION ${PROJECT_VERSION}
	SOVERSION 1
	PUBLIC_HEADER "message.h;messagefactory.h;messageengine.h;messageenginefactory.h;responsemessage.h;callmessage.h;eventmessage.h")


install(TARGETS qtappfw-core
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtappfw-core)

