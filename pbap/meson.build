qt5_dep = dependency('qt5', modules: ['Qml'])

moc_files = qt5.compile_moc(headers : 'pbap.h',
                            dependencies: qt5_dep)

src = ['pbap.cpp', moc_files]
lib = shared_library('qtappfw-bt-pbap',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: qt5_dep,
                     install: true)

install_headers('pbap.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-bt-pbap',
                 filebase : 'qtappfw-bt-pbap',
                 requires: 'Qt5Qml',
                 description : 'Library wrapping AGL Bluetooth PBAP profile API in Qt objects')
