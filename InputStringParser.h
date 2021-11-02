//
// Created by alex on 30.09.21.
//

#ifndef ORDERBOOKSERVER_INPUTSTRINGPARSER_H
#define ORDERBOOKSERVER_INPUTSTRINGPARSER_H

#include <sstream>
#include "Order.h"

class InputStringParser
{
public:
    static InputOrder parse (std::string inputLine)
    {
        std::stringstream ssInput(inputLine);
        std::string token;
        getTokenOrThrow(inputLine, ssInput, token);
        switch (token[0])
        {
            case 'N':
                return parseNewOrder(inputLine, ssInput, token);
            case 'C':
                return parseCancelOrder(inputLine, ssInput, token);
            case 'F':
                return InputOrder::createFlushBooksOrder();
            default:
                throwFormatException(inputLine);
        }
        return InputOrder(); // not reachable code, just to avoid warnings
    }

    static std::vector<std::string> splitLines(const std::string& lines)
    {
        std::vector<std::string> result;
        std::stringstream ssInput(lines);
        std::string line;
        while(std::getline(ssInput, line))
        {
            if (line[0] == '#' || line.empty())
                continue; // drop comment and empty lines
            result.push_back(line);
        }
        return result;
    }

private:
    static InputOrder parseNewOrder(std::string& inputLine, std::stringstream& ssInput, std::string& token)
    {
        getTokenOrThrow(inputLine, ssInput, token);
        auto userId = std::atoi(token.c_str());
        getTokenOrThrow(inputLine, ssInput, token);
        auto symbol = token;
        getTokenOrThrow(inputLine, ssInput, token);
        auto price = std::atoi(token.c_str());
        if (price < 0)
            throwFormatException(inputLine);
        getTokenOrThrow(inputLine, ssInput, token);
        auto qty = std::atoi(token.c_str());
        if (qty == 0)
            throwFormatException(inputLine);
        qty = std::abs(qty);
        getTokenOrThrow(inputLine, ssInput, token);
        eOrderSide orderSide;
        switch (token[0])
        {
            case 'B':
                orderSide = eOrderSide::Buy;
                break;
            case 'S':
                orderSide = eOrderSide::Sell;
                break;
            default:
                throwFormatException(inputLine);
        }
        getTokenOrThrow(inputLine, ssInput, token);
        auto userOrderId = std::atoi(token.c_str());
        return InputOrder(userId, symbol, price, qty, orderSide, userOrderId);
    }

    static InputOrder parseCancelOrder(std::string& inputLine, std::stringstream& ssInput, std::string& token)
    {
        getTokenOrThrow(inputLine, ssInput, token);
        auto userId = std::atoi(token.c_str());
        getTokenOrThrow(inputLine, ssInput, token);
        auto userOrderId = std::atoi(token.c_str());
        return InputOrder(userId, userOrderId);
    }

    static void getTokenOrThrow(std::string& inputLine, std::stringstream& ssInput, std::string& token)
    {
        if (!std::getline(ssInput, token, ','))
            throwFormatException(inputLine);
        trim(token);
    }

    static void throwFormatException(std::string str)
    {
        throw std::runtime_error("Input string wrong format: '" + str + "'");
    }

    static void ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    }

    static void rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    }

    static void trim(std::string &s)
    {
        ltrim(s);
        rtrim(s);
    }
};

#endif //ORDERBOOKSERVER_INPUTSTRINGPARSER_H
