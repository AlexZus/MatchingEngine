//
// Created by alex on 01.10.21.
//

#ifndef ORDERBOOKSERVER_MATCHER_H
#define ORDERBOOKSERVER_MATCHER_H

#include "Order.h"

template<eBookSide CROSSBOOKSIDE>
class Matcher
{
public:
    using OrderEraser = std::function<void(BookIterator<CROSSBOOKSIDE>&)>;

    Matcher(const BookOrder& ord, BookIterator<CROSSBOOKSIDE> bookIterator, BookIterator<CROSSBOOKSIDE> bookIteratorEnd,
            OrderEraser orderEraser)
            : takerOrd(ord)
            , bookIteratorNext(bookIterator)
            , bookIteratorEnd(bookIteratorEnd)
            , orderEraser(orderEraser)
    {
        doStepToNextQuote();
        filledAmount = 0;
    };

    void doStepToNextQuote()
    {
        bookIterator = bookIteratorNext++;
        crossedPrice = bookIterator->second->price;
    }

    bool isNextQuoteOnLevelExist()
    {
        return bookIteratorNext != bookIteratorEnd && bookIteratorNext->second->price == crossedPrice;
    }

    bool isNextQuoteExist()
    {
        return bookIteratorNext != bookIteratorEnd;
    };

    bool isQuoteMatchingAllowed()
    {
        return bookIterator->second->userId != takerOrd.userId;
    }

    bool isNextQuoteCrossed()
    {
        return bookIteratorNext->first.isCrossed(takerOrd.price);
    };

    void createTrade(OutputCallback outputCallback)
    {
        auto& bookOrd = bookIterator->second;
        int takerOldAmount = takerOrd.qty - filledAmount;
        int tradeVolume = std::min(takerOldAmount, bookOrd->qty);
        filledAmount += tradeVolume;
        if (CROSSBOOKSIDE == eBookSide::Bid)
            outputCallback(Trade(bookOrd->userId, bookOrd->id,
                           takerOrd.userId, takerOrd.id, bookOrd->price, tradeVolume));
        else
            outputCallback(Trade(takerOrd.userId, takerOrd.id,
                           bookOrd->userId, bookOrd->id, bookOrd->price, tradeVolume));
        if (filledAmount < takerOrd.qty || (filledAmount == takerOrd.qty && tradeVolume == bookOrd->qty))
        {
            // book quote was filled
            eraseCurrentOrder();
        }
        else
        {
            // book quote was partially filled
            bookOrd->qty -= tradeVolume;
            isLastQuotePartiallyFilled = true;
        }
    };

    void eraseCurrentOrder()
    {
        orderEraser(bookIterator);
    }

    int getFilledAmount() const
    {
        return filledAmount;
    };

    std::pair<int /*bestPrice*/, int /*totalTopBookQty*/> getBestPriceAndTotalTopBookQty()
    {
        auto& bookIteratorOfBestPriceQuote = isLastQuotePartiallyFilled ? bookIterator : bookIteratorNext;
        int bestPrice = bookIteratorOfBestPriceQuote != bookIteratorEnd ?
                bookIteratorOfBestPriceQuote->first.price : BookOrder::PRICE_UNDEFINED;
        int totalTopBookQty = 0;
        while (bookIteratorOfBestPriceQuote != bookIteratorEnd && bookIteratorOfBestPriceQuote->first.price == bestPrice)
        {
            totalTopBookQty += bookIteratorOfBestPriceQuote->second->qty;
            ++bookIteratorOfBestPriceQuote;
        }
        return {bestPrice, totalTopBookQty};
    }

private:
    BookOrder takerOrd;
    BookIterator<CROSSBOOKSIDE> bookIterator;
    BookIterator<CROSSBOOKSIDE> bookIteratorNext;
    BookIterator<CROSSBOOKSIDE> bookIteratorEnd;
    int filledAmount;
    int crossedPrice;
    bool isLastQuotePartiallyFilled = false;
    OrderEraser orderEraser;
};

#endif //ORDERBOOKSERVER_MATCHER_H
