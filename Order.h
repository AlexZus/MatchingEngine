//
// Created by alex on 30.09.21.
//

#ifndef ORDERBOOKSERVER_ORDER_H
#define ORDERBOOKSERVER_ORDER_H

#include <map>
#include <memory>
#include <functional>
#include "BookPrice.h"
#include "OutputData.h"

using OutputCallback = std::function<void(const OutputData&)>;

enum eOrderSide : bool
{
    Buy,
    Sell
};

struct UniqueOrderId
{
    using type = std::int64_t;

    static type createUniqueOrderId(int userId, int userOrderId)
    {
        return (std::int64_t(userId) << 32) + userOrderId;
    }
};

struct OrderIds
{
    int userId;
    int id;

    OrderIds(int userId, int userOrderId)
            : userId(userId)
            , id(userOrderId)
    {}

    UniqueOrderId::type createUniqueOrderId() const
    {
        return UniqueOrderId::createUniqueOrderId(userId, id);
    }
};

struct BookOrder : public OrderIds
{
    using ptr = std::shared_ptr<BookOrder>;
    static constexpr int PRICE_SPECIAL_MEANING_MARKET_ORDER = 0;
    static constexpr int PRICE_UNDEFINED = -1;
    static constexpr int ORDER_ID_UNDEFINED = 0;
    static constexpr int USER_ID_UNDEFINED = 0;

    int qty;
    int price;

    BookOrder(int qty, int price, int userId, int userOrderId)
        : qty(qty)
        , price(price)
        , OrderIds(userId, userOrderId)
    {}
};

enum class eInputOrderType
{
    FlushBooks,
    NewOrder,
    CancelOrder
};

struct InputOrder
{
    std::optional<BookOrder> bookOrder;
    std::optional<OrderIds> cancelOrder;
    eInputOrderType type;
    std::string symbol;
    eOrderSide side;

    static InputOrder createFlushBooksOrder()
    {
        InputOrder order;
        order.type = eInputOrderType::FlushBooks;
        return order;
    }

    InputOrder() {}

    InputOrder(int userId, const std::string& symbol, int price, int qty, eOrderSide orderSide, int userOrderId)
        : bookOrder(BookOrder{qty, price, userId, userOrderId})
        , type(eInputOrderType::NewOrder)
        , symbol(symbol)
        , side(orderSide)
    {}

    InputOrder(int userId, int userOrderId)
        : cancelOrder(OrderIds{userId, userOrderId})
        , type(eInputOrderType::CancelOrder)
    {}
};

template <eBookSide BOOKSIDE>
using BookSide = std::multimap<BookPrice<BOOKSIDE>, BookOrder::ptr>;

using Bids = BookSide<eBookSide::Bid>;
using Asks = BookSide<eBookSide::Ask>;

template <eBookSide BOOKSIDE>
using BookIterator = typename std::multimap<BookPrice<BOOKSIDE>, BookOrder::ptr>::iterator;

using IdToOrderMap = std::unordered_map<UniqueOrderId::type , BookOrder::ptr>;

#endif //ORDERBOOKSERVER_ORDER_H
