### Simple Matching engine for trading platform

##### Brief description
This is simple matching engine that can perform up to 
7 millions orders per second per trading symbol.
It can hold order books for a number of symbols.
It supports market and limit orders. There is unit tests and performance tests.
And there is a OrderBookServer that can receive orders through UDP in string format.

##### Building project requirements
To build project the GNU g++ building environment with cmake must be installed.
The Boost library must be also installed. The 'unit_test_framework' and
the 'system' Boost components are needed for build.
The build tested on g++ 9.3.0 vesion, cmake 3.16.3 version, and Boost 1.71.0 version.

##### Building project instructions

###### Ubuntu 20.04

* Ensure that these dependencies are installed by executing the next command:
```shell script
sudo apt-get install g++ make cmake
```
* Install Boost
```shell script
wget -O boost_1_72_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.72.0/boost_1_72_0.tar.gz/download
tar xzvf boost_1_72_0.tar.gz
cd boost_1_72_0/
sudo apt-get update
sudo apt-get install build-essential g++ python-dev autotools-dev libicu-dev libbz2-dev
./bootstrap.sh --prefix=/usr/local
sudo ./b2 --with=all -j 6 install
sudo ldconfig
```
* Build project by executing the `do_make_build_release.sh` script
after all dependencies were installed .

##### Usage
###### Unit tests
Run the next command to execute unit tests.
```shell script
./cmake-build-release/runUnitTests
```

###### Performance tests
Run the next command to execute performance tests.
```shell script
./cmake-build-release/runPerformanceTests
```

###### Matching engine UDP server
Run the next command to start the matching engine UDP server.
```shell script
./cmake-build-release/OrderBookServer <UDP port value>
```
The netcat can be used to check the server.
```shell script
cat ./inputfile.csv | netcat -u 127.0.0.1 <UDP port value>
```

###### Documentation
Run the next command to open documentation.
```shell script
nohup xdg-open ./documentation/html/index.html
```

##### Architectural Aspects

###### OrderBooksServer class
This class runs UDP server and receive orders as text.
And it have its own thread to poll from socket.

###### OrderBooksProcessor class
This class runs two threads.
One for the order books processing. And second for output to console.

###### OrderBooksContainer class
This class used as container for order books.
It can contain one order book per trading symbol.

###### OrderBook class
This class contains logic for order books operation and matching.