qt5_dep = dependency('qt5', modules: ['Qml'])

moc_files = qt5.compile_moc(headers: 'radio.h',
                            dependencies: qt5_dep)

src = ['radio.cpp', moc_files]
lib = shared_library('qtappfw-radio',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: qt5_dep,
                     install: true)

install_headers('radio.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries : lib,
                 version : '1.0',
                 name : 'libqtappfw-radio',
                 filebase : 'qtappfw-radio',
                 requires: 'Qt5Qml',
                 description : 'Library wrapping AGL radio API in Qt objects')
