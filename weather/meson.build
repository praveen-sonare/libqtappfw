qt5_dep = dependency('qt5', modules: ['Qml'])

moc_files = qt5.compile_moc(headers: 'weather.h',
                            dependencies: qt5_dep)

src = ['weather.cpp', moc_files]
lib = shared_library('qtappfw-weather',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: qt5_dep,
                     install: true)

install_headers('weather.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-weather',
                 filebase : 'qtappfw-weather',
                 requires: 'Qt5Qml',
                 description : 'Library wrapping AGL weather API in Qt objects')
