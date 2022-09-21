qt5_dep = dependency('qt5', modules: ['Core', 'WebSockets'])

moc_files = qt5.compile_moc(headers: 'vehiclesignals.h',
                            dependencies: qt5_dep)

src = ['vehiclesignals.cpp', moc_files]
lib = shared_library('qtappfw-vehicle-signals',
                     sources: src,
                     version: '1.0.0',
                     soversion: '0',
                     dependencies: qt5_dep,
                     install: true)

install_headers('vehiclesignals.h')

pkg_mod = import('pkgconfig')
pkg_mod.generate(libraries: lib,
                 version: '1.0',
                 name: 'libqtappfw-vehicle-signals',
                 filebase: 'qtappfw-vehicle-signals',
                 requires: ['Qt5Core', 'Qt5WebSockets'],
                 description: 'Library wrapping VIS vehicle signaling API in Qt objects')

qtappfw_vs_dep = declare_dependency(dependencies: qt5_dep,
                                    link_with: lib,
                                    include_directories: '.',
                                    sources: ['vehiclesignals.h'])
