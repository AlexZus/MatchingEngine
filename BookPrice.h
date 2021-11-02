//
// Created by alex on 01.10.21.
//

#ifndef ORDERBOOKSERVER_BOOKPRICE_H
#define ORDERBOOKSERVER_BOOKPRICE_H


enum eBookSide : bool
{
    Bid,
    Ask
};

template<eBookSide BOOKSIDE>
struct BookPrice
{
    int price;

    BookPrice(const int& price)
            : price(price) {};

    /// @brief Check possible trade
    /// Assumes takerPrice is on the opposite side
    bool isCrossed(int& takerPrice) const;

    /// @brief less than compare to a price
    /// Assumes both prices are on the same side.
    /// Uses side to determine the sense of the comparison.
    bool operator <(const BookPrice & rhs) const;
    bool operator >(const BookPrice & rhs) const;
};


template<> inline
bool BookPrice<eBookSide::Ask>::isCrossed(int& takerPrice) const
{
    return price <= takerPrice;
}

template<> inline
bool BookPrice<eBookSide::Bid>::isCrossed(int& takerPrice) const
{
    return takerPrice <= price;
}

template<> inline
bool BookPrice<eBookSide::Ask>::operator <(const BookPrice & rhs) const
{
    // Selling: lowest prices first
    return price < rhs.price;
}

template<> inline
bool BookPrice<eBookSide::Bid>::operator <(const BookPrice & rhs) const
{
    // Buying: Highest prices first.
    return price > rhs.price;
}

template<> inline
bool BookPrice<eBookSide::Ask>::operator >(const BookPrice & rhs) const
{
    // Selling: lowest prices first
    return price > rhs.price;
}

template<> inline
bool BookPrice<eBookSide::Bid>::operator >(const BookPrice & rhs) const
{
    // Buying: Highest prices first.
    return price < rhs.price;
}

#endif //ORDERBOOKSERVER_BOOKPRICE_H
