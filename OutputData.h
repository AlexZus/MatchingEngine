//
// Created by alex on 02.10.21.
//

#ifndef ORDERBOOKSERVER_OUTPUTDATA_H
#define ORDERBOOKSERVER_OUTPUTDATA_H

struct OutputData
{
    virtual ~OutputData() = default;
    virtual std::string toString() const = 0;
};

struct Acknowledgment : public OutputData
{
    int userId;
    int userOrderId;

    Acknowledgment(int userId, int userOrderId)
        : userId(userId)
        , userOrderId(userOrderId)
    {}

    std::string toString() const override
    {
        return "A, " + std::to_string(userId) + ", " + std::to_string(userOrderId);
    }
};

struct Trade : public OutputData
{
    int userIdBuy;
    int userOrderIdBuy;
    int userIdSell;
    int userOrderIdSell;
    int price;
    int qty;

    Trade(int userIdBuy, int userOrderIdBuy, int userIdSell, int userOrderIdSell, int price, int qty)
        : userIdBuy(userIdBuy)
        , userOrderIdBuy(userOrderIdBuy)
        , userIdSell(userIdSell)
        , userOrderIdSell(userOrderIdSell)
        , price(price)
        , qty(qty)
    {}

    std::string toString() const override
    {
        std::stringstream ss;
        ss << "T, " << userIdBuy << ", " << userOrderIdBuy << ", " << userIdSell << ", "
           << userOrderIdSell << ", " << price << ", " << qty;
        return ss.str();
    }
};

struct TopOfBookChanges : public OutputData
{
    char sideSymbol;
    int price;
    int qty;

    TopOfBookChanges(char sideSymbol, int price, int qty)
        : sideSymbol(sideSymbol)
        , price(price)
        , qty(qty)
    {}

    std::string toString() const override
    {
        std::stringstream ss;
        ss << "B, " << sideSymbol << ", " << price << ", " << qty;
        return ss.str();
    }
};

struct EliminateOfBookSideChanges : public OutputData
{
    char sideSymbol;

    EliminateOfBookSideChanges(char sideSymbol)
            : sideSymbol(sideSymbol)
    {}

    std::string toString() const override
    {
        std::stringstream ss;
        ss << "B, " << sideSymbol << ", -, -";
        return ss.str();
    }
};

#endif //ORDERBOOKSERVER_OUTPUTDATA_H
