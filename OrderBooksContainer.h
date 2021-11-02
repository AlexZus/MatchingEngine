//
// Created by alex on 30.09.21.
//

#ifndef ORDERBOOKSERVER_ORDERBOOKSCONTAINER_H
#define ORDERBOOKSERVER_ORDERBOOKSCONTAINER_H

#include "OrderBook.h"

class OrderBooksContainer
{
public:
    OrderBooksContainer(OutputCallback outputCallback)
        : outputCallback(outputCallback)
    {}

    void processOrder(const InputOrder& order)
    {
        switch (order.type)
        {
            case eInputOrderType::FlushBooks:
                flushBooks();
                break;
            case eInputOrderType::NewOrder:
                newOrder(order.symbol, order.bookOrder.value(), order.side);
                break;
            case eInputOrderType::CancelOrder:
                cancelOrder(order.cancelOrder.value());
                break;
            default:
                throw std::runtime_error("Unknown eInputOrderType in the OrderBooksProcessor::process()");
        }
    }

private:

    void flushBooks()
    {
        symbolToOrderBookMap.clear();
        uniqueOrderIdToOrderBookMap.clear();
    }

    void newOrder(const std::string& symbol, const BookOrder& order, eOrderSide orderSide)
    {
        auto uniqueOrderId = order.createUniqueOrderId();
        auto iter = symbolToOrderBookMap.find(symbol);
        OrderBook::Ptr book;
        if (iter == symbolToOrderBookMap.end())
        {
            book = std::make_shared<OrderBook>(symbol, outputCallback);
            symbolToOrderBookMap.emplace(symbol, book);
        }
        else
        {
            book = iter->second;
        }
        if (orderSide == eOrderSide::Sell)
            book->add<eBookSide::Ask, eBookSide::Bid>(uniqueOrderId, order);
        else
            book->add<eBookSide::Bid, eBookSide::Ask>(uniqueOrderId, order);
        uniqueOrderIdToOrderBookMap.emplace(uniqueOrderId, OrderInfo{book, orderSide});
    }

    void cancelOrder(OrderIds cancelOrder)
    {
        auto uniqueOrderId = cancelOrder.createUniqueOrderId();
        auto iter = uniqueOrderIdToOrderBookMap.find(uniqueOrderId);
        if (iter == uniqueOrderIdToOrderBookMap.end())
            throw std::runtime_error("The order canceling was failed. Order not in the book.");
        auto& book = iter->second.book;
        auto& orderSide = iter->second.side;
        if (orderSide == eOrderSide::Buy)
            book->remove<eBookSide::Bid>(uniqueOrderId);
        else
            book->remove<eBookSide::Ask>(uniqueOrderId);
        if (book->isEmpty())
            symbolToOrderBookMap.erase(book->getSymbol());
        uniqueOrderIdToOrderBookMap.erase(uniqueOrderId);
    }

private:
    struct OrderInfo
    {
        OrderBook::Ptr book;
        eOrderSide side;
    };

    OutputCallback outputCallback;
    std::unordered_map<std::string, OrderBook::Ptr> symbolToOrderBookMap;
    std::unordered_map<UniqueOrderId::type, OrderInfo> uniqueOrderIdToOrderBookMap;
};

#endif //ORDERBOOKSERVER_ORDERBOOKSCONTAINER_H
