#!/bin/sh

if [ $# -ne 1 ];
then
    echo Usage = $0 [where_the_game_data_is]
    exit
fi

GameData=`realpath $1`

echo [BUILDING DEB PACKAGE]

echo [Configuration with CMake]

mkdir -p build_dir
cd build_dir
cmake -DSOSAGE_DATA_FOLDER=$GameData -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr ../../..

echo [Compilation]

make -j 6

echo [Installation in local directory]

make install DESTDIR=../superflu-pnc/
cd ..

echo [Building Deb]

dpkg-deb --build superflu-pnc

echo [All done]
