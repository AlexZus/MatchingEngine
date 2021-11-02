//
// Created by alex on 02.10.21.
//

#include <chrono>
#include <vector>
#include <iostream>
#include <numeric>
#include <cmath>
#include "InputFileForTests.h"
#include "InputStringParser.h"
#include "OrderBooksContainer.h"

constexpr int iterationsNumber = 10000;

std::vector<InputOrder> getInputOrders()
{
    std::vector<InputOrder> inputOrders;
    for (const auto &line : InputStringParser::splitLines(inputLinesFromFile))
    {
        inputOrders.push_back(InputStringParser::parse(line));
    }
    return inputOrders;
}

int getOutputDataNumber()
{
    auto inputOrders = getInputOrders();
    int counter = 0;
    OrderBooksContainer orderBooksContainer(
            [&counter](const OutputData& outputData){ ++counter; });
    for (const auto &inputOrder : inputOrders)
        orderBooksContainer.processOrder(inputOrder);
    return counter;
}

void printTotalDurationStats(long duration_usec, unsigned long ordersNumber)
{
    auto meanOrderProcessingTime_usec = double(duration_usec) / ordersNumber;
    auto throughputOrdersPerSec = 1000000 / meanOrderProcessingTime_usec;
    std::cout << "Total time is " << duration_usec << " usec" << std::endl;
    std::cout << "Avarage processing time for one order is " << meanOrderProcessingTime_usec << " usec" << std::endl;
    std::cout << "Avarage number of orders precessed per second is " << throughputOrdersPerSec << std::endl;
}

void simpleThroughputTest()
{
    std::cout << "Starting throughput test. It is testing by measure total time. Without string parsing." << std::endl;
    auto inputOrders = getInputOrders();
    std::vector<std::reference_wrapper<const OutputData>> outputDataArray;
    outputDataArray.reserve(iterationsNumber * getOutputDataNumber());
    OrderBooksContainer orderBooksContainer(
            [&outputDataArray](const OutputData& outputData){ outputDataArray.push_back(std::cref(outputData)); });

    std::cout << "Processing " << iterationsNumber * inputOrders.size() << " orders" << std::endl;

    using namespace std::chrono;
    auto timeStampStart = high_resolution_clock::now();
    for (int c = 0; c < iterationsNumber; ++c)
    {
        for (const auto &inputOrder: inputOrders)
        {
            orderBooksContainer.processOrder(inputOrder);
        }
    }
    auto duration_usec = duration_cast<microseconds>(high_resolution_clock::now() - timeStampStart).count();
    printTotalDurationStats(duration_usec, iterationsNumber * inputOrders.size());
    std::cout << std::endl;
}

void throughputTest()
{
    std::cout << "Starting throughput test. It is testing by measure execution time for each order. Without string parsing." << std::endl;
    auto inputOrders = getInputOrders();
    std::vector<std::reference_wrapper<const OutputData>> outputDataArray;
    outputDataArray.reserve(iterationsNumber * getOutputDataNumber());
    OrderBooksContainer orderBooksContainer(
            [&outputDataArray](const OutputData& outputData){ outputDataArray.push_back(std::cref(outputData)); });
    std::cout << "Processing " << iterationsNumber * inputOrders.size() << " orders" << std::endl;

    using namespace std::chrono;
    using TimeStamp = time_point<high_resolution_clock>;
    std::vector<TimeStamp> timeStamps;
    timeStamps.resize(iterationsNumber * inputOrders.size() + 1);
    int indx = -1;

    timeStamps[++indx] = high_resolution_clock::now();
    for (int c = 0; c < iterationsNumber; ++c)
    {
        for (const auto &inputOrder: inputOrders)
        {
            orderBooksContainer.processOrder(inputOrder);
            timeStamps[++indx] = high_resolution_clock::now();
        }
    }
    auto duration_usec = duration_cast<microseconds>(high_resolution_clock::now() - timeStamps[0]).count();
    printTotalDurationStats(duration_usec, iterationsNumber * inputOrders.size());
    std::vector<double> ordersExecTime_usec;
    ordersExecTime_usec.reserve(timeStamps.size() - 1);
    for (long c = 1; c < timeStamps.size(); ++c)
    {
        ordersExecTime_usec.push_back(double(duration_cast<nanoseconds>(timeStamps[c] - timeStamps[c - 1]).count()) / 1000);
    }
    double sum = std::accumulate(ordersExecTime_usec.begin(), ordersExecTime_usec.end(), 0.0);
    double mean = sum / ordersExecTime_usec.size();

    double sq_sum = std::inner_product(ordersExecTime_usec.begin(), ordersExecTime_usec.end(), ordersExecTime_usec.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / ordersExecTime_usec.size() - mean * mean);
    std::cout << "Standard deviation for order execution time is " << stdev << " usec" << std::endl;

}

int main(int argc, char* argv[])
{
    simpleThroughputTest();
    throughputTest();
    return 0;
}