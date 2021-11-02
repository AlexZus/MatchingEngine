#!/bin/sh

rm -rf cmake-build-release
mkdir -p cmake-build-release
cd ./cmake-build-release
cmake ../ -DCMAKE_BUILD_TYPE=Release
make OrderBookServer
make runUnitTests
make runPerformanceTests