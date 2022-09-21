qt5_dep = dependency('qt5', modules: ['Qml'])

moc_files = qt5.compile_moc(headers : 'map.h',
                            dependencies: qt5_dep)

src = ['map.cpp', moc_files]
lib = shared_library('qtappfw-bt-map',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: [qt5_dep, qtappfw_bt_dep],
                     install: true)

install_headers('map.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-bt-map',
                 filebase : 'qtappfw-bt-map',
                 requires: 'Qt5Qml',
                 description : 'Library wrapping AGL Bluetooth MAP profile API in Qt objects')
