//
// Created by alex on 30.09.21.
//

#ifndef ORDERBOOKSERVER_ORDERBOOK_H
#define ORDERBOOKSERVER_ORDERBOOK_H

#include <cassert>
#include "Order.h"
#include "Matcher.h"

class OrderBook
{
public:
    using Ptr = std::shared_ptr<OrderBook>;

    OrderBook(const std::string& symbol, OutputCallback outputCallback)
        : symbol(symbol)
        , outputCallback(outputCallback)
    {
    }

    template<eBookSide BOOKSIDE, eBookSide CROSSBOOKSIDE>
    void add(const UniqueOrderId::type& uniqueOrderId, const BookOrder& order)
    {
        outputCallback(Acknowledgment(order.userId, order.id));
        if (order.price == BookOrder::PRICE_SPECIAL_MEANING_MARKET_ORDER)
        {
            if (isAny<CROSSBOOKSIDE>())
            {
                doMarketOrderMatches<BOOKSIDE, CROSSBOOKSIDE>(order);
            }
            else
            {
                // There is no orders in the order book to match the market order. Assume canceled
            }
        }
        else // order type is Limit
        {
            if (isAny<CROSSBOOKSIDE>() && isBestPriceCrossed<CROSSBOOKSIDE>(order.price))
            {
                doMatches<BOOKSIDE, CROSSBOOKSIDE>(order);
            }
            else
            {
                insertOrder<BOOKSIDE>(order);
            }
        }
    }

    template<eBookSide BOOKSIDE>
    void remove(const UniqueOrderId::type& uniqueOrderId)
    {
        auto iter = idToOrderMap.find(uniqueOrderId);
        if (iter == idToOrderMap.end())
            throw std::runtime_error("The order can't be found in the order book");
        auto& ord = iter->second;
        outputCallback(Acknowledgment(ord->userId, ord->id));

        BookPrice<BOOKSIDE> key(ord->price);
        BookIterator<BOOKSIDE> bookIterator;
        BookSide<BOOKSIDE>& bookSide = getBookSide<BOOKSIDE>();
        for (bookIterator = bookSide.find(key); bookIterator != bookSide.end(); ++bookIterator)
        {
            // If this is the correct quote erase it
            if (bookIterator->second->createUniqueOrderId() == uniqueOrderId)
            {
                if (getBestPrice<BOOKSIDE>() == ord->price)
                {
                    auto topBookQty = getTopBookQty<BOOKSIDE>();
                    if (topBookQty == ord->qty)
                    {
                        eraseOrder<BOOKSIDE>(bookIterator);
                        BookIterator<BOOKSIDE> iter = bookSide.begin();
                        auto newBestPrice = (iter == bookSide.end() ? BookOrder::PRICE_UNDEFINED : iter->first.price);
                        setBestPrice<BOOKSIDE>(newBestPrice);
                        int totalTopBookQty = 0;
                        while (iter != bookSide.end() && iter->first.price == newBestPrice)
                        {
                            totalTopBookQty += iter->second->qty;
                            ++iter;
                        }
                        setTopBookQty<BOOKSIDE>(totalTopBookQty);
                        reportOnTopOfBookChange<BOOKSIDE>();
                    }
                    else
                    {
                        topBookQty -= ord->qty;
                        setTopBookQty<BOOKSIDE>(topBookQty);
                        reportOnTopOfBookChange<BOOKSIDE>();
                        eraseOrder<BOOKSIDE>(bookIterator);
                    }

                }
                else
                {
                    eraseOrder<BOOKSIDE>(bookIterator);
                }
                return;
            }
            else if (key < bookIterator->first)
            {
                // exit early if result is beyond the matching prices
                throw std::runtime_error("The order can't be found in the order book");
            }
        }
    }

    bool isEmpty()
    {
        return bids.empty() && asks.empty();
    }

    const std::string& getSymbol()
    {
        return symbol;
    }

private:
    template<eBookSide BOOKSIDE>
    void updateAfterOrderInsert(const int& insertedPrice, const int& insertedQty)
    {
        auto bestPrice = getBestPrice<BOOKSIDE>();
        if (bestPrice == BookOrder::PRICE_UNDEFINED || !isBestPriceCrossed<BOOKSIDE>(insertedPrice))
        {
            if (bestPrice == BookOrder::PRICE_UNDEFINED)
                setTopBookQty<BOOKSIDE>(insertedQty);
            setBestPrice<BOOKSIDE>(insertedPrice);
            reportOnTopOfBookChange<BOOKSIDE>();
        }
        else if (bestPrice == insertedPrice)
        {
            setTopBookQty<BOOKSIDE>( getTopBookQty<BOOKSIDE>() + insertedQty);
            reportOnTopOfBookChange<BOOKSIDE>();
        }
    }

    template<eBookSide BOOKSIDE>
    bool isAny();

    template<eBookSide CROSSBOOKSIDE>
    bool isBestPriceCrossed(const int& takerPrice);

    template<eBookSide BOOKSIDE>
    void insertOrder(const BookOrder& ord);

    template<eBookSide BOOKSIDE>
    void eraseOrder(BookIterator<BOOKSIDE>& bookIterator);

    template<eBookSide BOOKSIDE, eBookSide CROSSBOOKSIDE>
    void doMatches(const BookOrder& ord);

    template<eBookSide BOOKSIDE, eBookSide CROSSBOOKSIDE>
    void doMarketOrderMatches(const BookOrder& ord);

    template<eBookSide BOOKSIDE>
    BookSide<BOOKSIDE>& getBookSide();

    template<eBookSide BOOKSIDE>
    void reportOnTopOfBookChange();

    template<eBookSide BOOKSIDE>
    void setBestPrice(const int& bestPrice);

    template<eBookSide BOOKSIDE>
    const int& 	getBestPrice() const;

    template<eBookSide BOOKSIDE>
    void setTopBookQty(const int& qty);

    template<eBookSide BOOKSIDE>
    const int& getTopBookQty();

private:
    OutputCallback outputCallback;
    std::string symbol;
    Bids bids;
    Asks asks;
    int bestBidPrice = BookOrder::PRICE_UNDEFINED;
    int bestAskPrice = BookOrder::PRICE_UNDEFINED;
    int topBidQty = 0;
    int topAskQty = 0;
    IdToOrderMap idToOrderMap;
};

template<> inline
bool 	OrderBook::isAny<eBookSide::Ask>() { return (asks.size() > 0); }
template<> inline
bool 	OrderBook::isAny<eBookSide::Bid>() { return (bids.size() > 0); }

template<> inline
bool	OrderBook::isBestPriceCrossed<eBookSide::Ask>(const int& takerPrice)
{ return bestAskPrice <= takerPrice; }

template<> inline
bool	OrderBook::isBestPriceCrossed<eBookSide::Bid>(const int& takerPrice)
{ return bestBidPrice >= takerPrice; }

template<> inline
Asks& OrderBook::getBookSide<eBookSide::Ask>() { return asks; }
template<> inline
Bids& OrderBook::getBookSide<eBookSide::Bid>() { return bids; }

template<> inline
void OrderBook::reportOnTopOfBookChange<eBookSide::Ask>()
{
    if (bestAskPrice == BookOrder::PRICE_UNDEFINED)
        outputCallback(EliminateOfBookSideChanges('S'));
    else
        outputCallback(TopOfBookChanges('S', bestAskPrice, topAskQty));
}

template<> inline
void OrderBook::reportOnTopOfBookChange<eBookSide::Bid>()
{
    if (bestBidPrice == BookOrder::PRICE_UNDEFINED)
        outputCallback(EliminateOfBookSideChanges('B'));
    else
        outputCallback(TopOfBookChanges('B', bestBidPrice, topBidQty));
}

template<> inline
void OrderBook::setBestPrice<eBookSide::Ask>(const int& bestPrice)
{
    bestAskPrice = bestPrice;
}

template<> inline
void OrderBook::setBestPrice<eBookSide::Bid>(const int& bestPrice)
{
    bestBidPrice = bestPrice;
}

template<> inline
const int& 	OrderBook::getBestPrice<eBookSide::Ask>() const
{ return bestAskPrice; }
template<> inline
const int& 	OrderBook::getBestPrice<eBookSide::Bid>() const
{ return bestBidPrice; }

template<> inline
void OrderBook::setTopBookQty<eBookSide::Ask>(const int& qty)
{
    topAskQty = qty;
}
template<> inline
void OrderBook::setTopBookQty<eBookSide::Bid>(const int& qty)
{
    topBidQty = qty;
}

template<> inline
const int& OrderBook::getTopBookQty<eBookSide::Ask>()
{
    return topAskQty;
}

template<> inline
const int& OrderBook::getTopBookQty<eBookSide::Bid>()
{
    return topBidQty;
}


template<> inline
void OrderBook::insertOrder<eBookSide::Ask>(const BookOrder& ord)
{
    auto ordPtr = std::make_shared<BookOrder>(ord);
    asks.emplace(BookPrice<eBookSide::Ask>(ord.price), ordPtr);
    idToOrderMap.emplace(ord.createUniqueOrderId(), ordPtr);
    updateAfterOrderInsert<eBookSide::Ask>(ord.price, ord.qty);
}

template<> inline
void OrderBook::insertOrder<eBookSide::Bid>(const BookOrder& ord)
{
    auto ordPtr = std::make_shared<BookOrder>(ord);
    bids.emplace(BookPrice<eBookSide::Bid>(ord.price), ordPtr);
    idToOrderMap.emplace(ord.createUniqueOrderId(), ordPtr);
    updateAfterOrderInsert<eBookSide::Bid>(ord.price, ord.qty);
}

template<> inline
void OrderBook::eraseOrder<eBookSide::Ask>(BookIterator<eBookSide::Ask>& bookIterator)
{
    assert(idToOrderMap[bookIterator->second->createUniqueOrderId()] == bookIterator->second);
    idToOrderMap.erase(bookIterator->second->createUniqueOrderId());
    asks.erase(bookIterator);
}

template<> inline
void OrderBook::eraseOrder<eBookSide::Bid>(BookIterator<eBookSide::Bid>& bookIterator)
{
    assert(idToOrderMap[bookIterator->second->createUniqueOrderId()] == bookIterator->second);
    idToOrderMap.erase(bookIterator->second->createUniqueOrderId());
    bids.erase(bookIterator);
}

/**
 * \brief Performs limit order matching.
 *
 * During Limit order matching a stepping performed in the same way as for
 * Market orders.
 * Stepping performed until all Taker order amount would be filled or until no
 * more Limit orders in the Book or until Taker order price no more cross the
 * Book. All matched Limit orders would be deleted from the Book.
 * If is not all amount of Taker order filled (no more Limit orders in
 * the Book) then Taker order with rest amount will be inserted into Limit
 * Order Book.
 */
template<eBookSide BOOKSIDE, eBookSide CROSSBOOKSIDE> inline
void OrderBook::doMatches(const BookOrder& ord)
{
    Matcher<CROSSBOOKSIDE> matcher(ord, getBookSide<CROSSBOOKSIDE>().begin(), getBookSide<CROSSBOOKSIDE>().end(),
                                   [this](BookIterator<CROSSBOOKSIDE>& it){ eraseOrder<CROSSBOOKSIDE>(it); });
    while (true) // loop by levels
    {
        while (true) // loop within one level
        {
            if (matcher.isQuoteMatchingAllowed())
            {
                matcher.createTrade(outputCallback);
            }
            else
            {
                throw std::runtime_error("User can't trade against his own orders");
            }

            if (matcher.isNextQuoteOnLevelExist() && matcher.isNextQuoteCrossed() && matcher.getFilledAmount() < ord.qty)
                matcher.doStepToNextQuote();
            else
                break;
        }

        if (matcher.isNextQuoteExist() && matcher.isNextQuoteCrossed() && matcher.getFilledAmount() < ord.qty)
            matcher.doStepToNextQuote();
        else
            break;
    }

    if (matcher.getFilledAmount() < ord.qty)
    {
        // order was partially filled
        BookOrder orderToInsert = ord;
        orderToInsert.qty -= matcher.getFilledAmount();
        insertOrder<BOOKSIDE>(orderToInsert);
    }
    auto [newBestPrice, totalTopBookQty] = matcher.getBestPriceAndTotalTopBookQty();
    setTopBookQty<CROSSBOOKSIDE>(totalTopBookQty);
    auto bestPrice = getBestPrice<CROSSBOOKSIDE>();
    if (bestPrice != newBestPrice)
    {
        setBestPrice<CROSSBOOKSIDE>(newBestPrice);
    }
    reportOnTopOfBookChange<CROSSBOOKSIDE>();
}


/**
 * \brief Performs market order matching.
 *
 * When an order cross the Book and matching performed this order named Taker
 * order. Taker order has Amount parameter. It represent amount that can be
 * filled by opposite Limit orders from the Book.
 *
 * During Market order matching a stepping within Limit Order Book by levels
 * from first level (best bid/ask price level) to deeper levels performed.
 * And within one level a stepping by Limit Orders from the oldest to newer
 * performed.
 *
 * Stepping performed until all Taker order amount would be filled or until no
 * more Limit orders in the Book. All matched Limit orders would be deleted from
 * the Book. If is not all amount of Taker order filled (no more Limit orders in
 * the Book) then Order will be killed.
 */
template<eBookSide BOOKSIDE, eBookSide CROSSBOOKSIDE> inline
void OrderBook::doMarketOrderMatches(const BookOrder& ord)
{
    Matcher<CROSSBOOKSIDE> matcher(ord, getBookSide<CROSSBOOKSIDE>().begin(), getBookSide<CROSSBOOKSIDE>().end(),
                                   [this](BookIterator<CROSSBOOKSIDE>& it){ eraseOrder<CROSSBOOKSIDE>(it); });
    while (true) // loop by levels
    {
        while (true) // loop within one level
        {
            if (matcher.isQuoteMatchingAllowed())
            {
                matcher.createTrade(outputCallback);
            }
            else
            {
                throw std::runtime_error("User can't trade against his own orders");
            }

            if (matcher.isNextQuoteOnLevelExist() && matcher.getFilledAmount() < ord.qty)
                matcher.doStepToNextQuote();
            else
                break;
        }

        if (matcher.isNextQuoteExist() && matcher.getFilledAmount() < ord.qty)
            matcher.doStepToNextQuote();
        else
            break;
    }
    auto [newBestPrice, totalTopBookQty] = matcher.getBestPriceAndTotalTopBookQty();
    setTopBookQty<CROSSBOOKSIDE>(totalTopBookQty);
    auto bestPrice = getBestPrice<CROSSBOOKSIDE>();
    if (bestPrice != newBestPrice)
    {
        setBestPrice<CROSSBOOKSIDE>(newBestPrice);
    }
    reportOnTopOfBookChange<CROSSBOOKSIDE>();
}


#endif //ORDERBOOKSERVER_ORDERBOOK_H
