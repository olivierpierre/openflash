#!/bin/sh

cmake=`whereis cmake`
if [ "$cmake" = "cmake:" ]; then
  echo "cmake not found, please install cmake"
  exit
fi

rm -rf build/* build/.cproject build/.project

cd build
if [ "$1" = "release" ]; then
  echo "============================"
  echo "Creating RELEASE build conf."
  echo "============================"
  cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -D_ECLIPSE_VERSION=4.3 ../src/
else
  if [ "$1" = "profile" ]; then
    echo "============================"
    echo "Creating PROFILE build conf."
    echo "============================"
    cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Profile -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -D_ECLIPSE_VERSION=4.3 ../src/
  else
    echo "============================"
    echo "Creating DEBUG build conf."
    echo "============================"
    cmake -G"Eclipse CDT4 - Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT=TRUE -D_ECLIPSE_VERSION=4.3 ../src/
  fi
fi
cd -
