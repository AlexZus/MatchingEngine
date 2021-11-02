cmake_minimum_required(VERSION 3.16)
project(OrderBookServer)

set(CMAKE_CXX_STANDARD 17)

set (Boost_USE_STATIC_LIBS OFF)
find_package(Boost REQUIRED COMPONENTS unit_test_framework system)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
        OrderBooksServer.cpp
        InputStringParser.h
        OrderBooksProcessor.h
        Order.h
        OrderBooksContainer.h
        OrderBook.h
        BookPrice.h
        Matcher.h
        OutputData.h)

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_link_libraries(${PROJECT_NAME} pthread)

add_executable (runUnitTests UnitTests.cpp)
target_link_libraries (runUnitTests ${Boost_LIBRARIES})

add_executable (runPerformanceTests PerformanceTests.cpp InputFileForTests.h)
