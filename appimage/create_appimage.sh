#!/bin/sh

if [ $# -ne 1 ];
then
    echo Usage = $0 [your_linuxdeploy_file.AppImage]
    exit
fi

LinuxDeploy=$1

echo [BUILDING APPIMAGE USING $LinuxDeploy]

chmod +x $LinuxDeploy

echo [Configuration with CMake]

mkdir -p build_dir
cd build_dir
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ../..

echo [Compilation]

make -j 6

echo [Installation in local directory]

make install DESTDIR=../install_dir
cd ..

echo [Building AppImage]

$LinuxDeploy --appdir install_dir -e install_dir/usr/games/superflu_pnc --output appimage 

echo [All done]
