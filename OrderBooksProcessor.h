//
// Created by alex on 30.09.21.
//

#ifndef ORDERBOOKSERVER_ORDERBOOKSPROCESSOR_H
#define ORDERBOOKSERVER_ORDERBOOKSPROCESSOR_H
#include <boost/lockfree/spsc_queue.hpp>
#include <future>
#include <chrono>
#include "Order.h"
#include "OrderBooksContainer.h"
#include "InputStringParser.h"

class OrderBooksProcessor
{
    static constexpr int QUEUE_RING_BUFFER_SIZE = 1000;
    using QueueEntry = std::shared_future<InputOrder>;

public:
    OrderBooksProcessor()
        : isActive(true)
        , orderBookThread(&OrderBooksProcessor::process, std::ref(*this))
        , outputThread(&OrderBooksProcessor::outputThreadMethod, std::ref(*this))
        , orderBooksContainer([this](const OutputData& outputData){ processOutput(outputData); })
    {}

    ~OrderBooksProcessor()
    {
        isActive = false;
        outputDataNotifier.notify_one();
        if (orderBookThread.joinable())
            orderBookThread.join();
        if (outputThread.joinable())
            outputThread.join();
    }

    void feedLines(const std::string& lines)
    {
        for (const auto &line : InputStringParser::splitLines(lines))
        {
            enqueue(InputStringParser::parse(line));
        }
    }

    bool enqueue(const InputOrder& order)
    {
        lastEnqueuedOrderPromise.set_value(order);
        return pushFuture();
    }

private:
    bool pushFuture()
    {
        lastEnqueuedOrderPromise = std::promise<InputOrder>();
        return queue.push(lastEnqueuedOrderPromise.get_future().share());
    }

    void process()
    {
        try
        {
            QueueEntry entry;
            bool isFutureUsed = true;
            pushFuture();
            while (isActive)
            {
                if (isFutureUsed)
                {
                    queue.pop(entry);
                    isFutureUsed = false;
                }
                if (std::future_status::ready == entry.wait_for(std::chrono::seconds(1)))
                {
                    auto order = entry.get();
                    isFutureUsed = true;
                    orderBooksContainer.processOrder(order);
                }
            }
            std::cout << "The order books processing thread done." << std::endl;
        }
        catch (std::exception& e)
        {
            std::cout << "The order books processing thread failed. " << e.what() << std::endl;
        }
    }

    void processOutput(const OutputData& outputData)
    {
        outputQueue.push(outputData.toString());
        {
            std::lock_guard<std::mutex> lk(outputThreadMutex);
            isNewOutputData = true;
        }
        outputDataNotifier.notify_one();
    }

    void outputThreadMethod()
    {
        try
        {
            while (isActive)
            {
                std::unique_lock<std::mutex> lk(outputThreadMutex);
                outputDataNotifier.wait(lk, [this] { return isNewOutputData; });
                isNewOutputData = false;
                std::string str;
                outputQueue.pop(str);
                std::cout << str << std::endl;
                lk.unlock();
            }
            std::cout << "The output thread done. " << std::endl;
        }
        catch (std::exception& e)
        {
            std::cout << "The output thread failed. " << e.what() << std::endl;
        }
    }

private:
    std::atomic<bool> isActive;
    OrderBooksContainer orderBooksContainer;
    std::promise<InputOrder> lastEnqueuedOrderPromise;
    std::thread orderBookThread;
    std::thread outputThread;
    boost::lockfree::spsc_queue<QueueEntry, boost::lockfree::capacity<QUEUE_RING_BUFFER_SIZE>> queue;
    boost::lockfree::spsc_queue<std::string, boost::lockfree::capacity<QUEUE_RING_BUFFER_SIZE>> outputQueue;
    std::condition_variable outputDataNotifier;
    std::mutex outputThreadMutex;
    bool isNewOutputData = false;
};

#endif //ORDERBOOKSERVER_ORDERBOOKSPROCESSOR_H
