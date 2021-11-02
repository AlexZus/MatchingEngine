//
// Created by alex on 01.10.21.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "OrderBooksProcessor.h"

class TestsOrderBooksProcessor
{
public:
    TestsOrderBooksProcessor()
        : orderBooksContainer([this](const OutputData& outputData){ processOutput(outputData); })
    {
    }

    std::string feedLinesAndGetOutput(const std::string& lines)
    {
        for (const auto &line : InputStringParser::splitLines(lines))
        {
            orderBooksContainer.processOrder(InputStringParser::parse(line));
        }
        return ss.str();
    }

private:
    void processOutput(const OutputData& outputData)
    {
        ss << outputData.toString() << std::endl;
    }

private:
    OrderBooksContainer orderBooksContainer;
    std::stringstream ss;
};

BOOST_AUTO_TEST_CASE( balanced_book )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario1, balanced book\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102 \n"
                 "\n"
                 "# hit book on each side, generate trades, TOB = 9/12\n"
                 "N, 1, IBM, 11, 100, B, 3\n"
                 "N, 2, IBM, 10, 100, S, 103\n"
                 "\n"
                 "# replenish book on each side, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 4\n"
                 "N, 2, IBM, 11, 100, S, 104\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 3\n"
                     "T, 1, 3, 2, 102, 11, 100\n"
                     "B, S, 12, 100\n"
                     "A, 2, 103\n"
                     "T, 1, 1, 2, 103, 10, 100\n" // was "T, 2, 103, 1, 1, 10, 100" in output.csv
                     "B, B, 9, 100\n"
                     "A, 1, 4\n"
                     "B, B, 10, 100\n"
                     "A, 2, 104\n"
                     "B, S, 11, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}

BOOST_AUTO_TEST_CASE( shallow_bid )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario2, shallow bid\n"
                 "\n"
                 "# build book, shallow bid, TOB = 10/11\n"
                 "N, 1, AAPL, 10, 100, B, 1\n"
                 "N, 1, AAPL, 12, 100, S, 2\n"
                 "N, 2, AAPL, 11, 100, S, 102\n"
                 "\n"
                 "# hit bid, generate trade, TOB = -/11\n"
                 "N, 2, AAPL, 10, 100, S, 103\n"
                 "\n"
                 "# replenish bid, TOB = 10/11\n"
                 "N, 1, AAPL, 10, 100, B, 3\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 2, 103\n"
                     "T, 1, 1, 2, 103, 10, 100\n"
                     "B, B, -, -\n"
                     "A, 1, 3\n"
                     "B, B, 10, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}

BOOST_AUTO_TEST_CASE( shallow_ask )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario3, shallow ask\n"
                 "\n"
                 "# build book, shallow ask, TOB = 10/11\n"
                 "N, 1, VAL, 10, 100, B, 1\n"
                 "N, 2, VAL, 9, 100, B, 101\n"
                 "N, 2, VAL, 11, 100, S, 102\n"
                 "\n"
                 "# hit ask, generate trade, TOB = 10/-\n"
                 "N, 1, VAL, 11, 100, B, 2\n"
                 "\n"
                 "# replenish ask, TOB = 10/11\n"
                 "N, 2, VAL, 11, 100, S, 103\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    //std::cout << output;
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 2\n"
                     "T, 1, 2, 2, 102, 11, 100\n"
                     "B, S, -, -\n"
                     "A, 2, 103\n"
                     "B, S, 11, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}

BOOST_AUTO_TEST_CASE( limit_below_best_bid )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario4, balanced book, limit below best bid\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# limit below best bid, generate trade, TOB = 9/11\n"
                 "N, 2, IBM, 9, 100, S, 103\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 2, 103\n"
                     "T, 1, 1, 2, 103, 10, 100\n"
                     "B, B, 9, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}

BOOST_AUTO_TEST_CASE( limit_above_best_ask )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario5, balanced book, limit above best ask\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# limit above best ask, generate trade, TOB = 10/12\n"
                 "N, 1, IBM, 12, 100, B, 103\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 103\n"
                     "T, 1, 103, 2, 102, 11, 100\n"
                     "B, S, 12, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( market_sell )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario6, balanced book, market sell\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# market sell, generate trade, TOB = 9/11\n"
                 "N, 2, IBM, 0, 100, S, 103\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 2, 103\n"
                     "T, 1, 1, 2, 103, 10, 100\n"
                     "B, B, 9, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( market_buy )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario7, balanced book, market buy\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# market buy, generate trade, TOB = 10/12\n"
                 "N, 1, IBM, 0, 100, B, 3\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 3\n"
                     "T, 1, 3, 2, 102, 11, 100\n"
                     "B, S, 12, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( tighten_spread_through_new_limit_orders )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario8, tighten spread through new limit orders\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 16, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 15, 100, S, 102\n"
                 "\n"
                 "# new bid, ask TOB = 11/14\n"
                 "N, 2, IBM, 11, 100, B, 103\n"
                 "N, 1, IBM, 14, 100, S, 3\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 16, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 15, 100\n"
                     "A, 2, 103\n"
                     "B, B, 11, 100\n"
                     "A, 1, 3\n"
                     "B, S, 14, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}

BOOST_AUTO_TEST_CASE( market_sell_partial )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario9, balanced book, market sell partial\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# market sell, generate partial trade, TOB = 10/11\n"
                 "N, 2, IBM, 0, 20, S, 103\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    //std::cout << output;
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n" // was "B, B, 10, 10" in the outputFile.csv
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 2, 103\n"
                     "T, 1, 1, 2, 103, 10, 20\n"
                     "B, B, 10, 80";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( market_buy_partial )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario10, balanced book, market buy partial\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# market buy, generate partial trade, TOB = 10/11\n"
                 "N, 1, IBM, 0, 20, B, 3\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 3\n"
                     "T, 1, 3, 2, 102, 11, 20\n"
                     "B, S, 11, 80";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( limit_sell_partial )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario11, balanced book, limit sell partial\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# limit sell, generate partial trade, TOB = 10/11\n"
                 "N, 2, IBM, 10, 20, S, 103\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n" // was "B, B, 100, 10" in the outputFile.csv
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 2, 103\n"
                     "T, 1, 1, 2, 103, 10, 20\n"
                     "B, B, 10, 80"; // was "B, B, 80, 10" in the outputFile.csv
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( limit_buy_partial )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario12, balanced book, limit buy partial\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# limit buy, generate partial trade, TOB = 10/11\n"
                 "N, 1, IBM, 11, 20, B, 3\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 3\n"
                     "T, 1, 3, 2, 102, 11, 20\n"
                     "B, S, 11, 80";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( multiple_orders_at_best_bid_offer )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario13, balanced book multiple orders at best bid offer\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "N, 2, IBM, 10, 50, B, 103\n"
                 "N, 1, IBM, 11, 50, S, 3\n"
                 "\n"
                 "# market buy and sell, generate trades, TOB = 10/11\n"
                 "N, 1, IBM, 11, 100, B, 4\n"
                 "N, 2, IBM, 10, 100, S, 104\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 2, 103\n"
                     "B, B, 10, 150\n"
                     "A, 1, 3\n"
                     "B, S, 11, 150\n"
                     "A, 1, 4\n"
                     "T, 1, 4, 2, 102, 11, 100\n"
                     "B, S, 11, 50\n"
                     "A, 2, 104\n"
                     "T, 1, 1, 2, 104, 10, 100\n"
                     "B, B, 10, 50";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( cancel_best_bid_and_offer )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario14, balanced book, cancel best bid and offer\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# limit buy, generate partial trade, TOB = 9/12\n"
                 "C, 1, 1\n"
                 "C, 2, 102\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 1\n"
                     "B, B, 9, 100\n"
                     "A, 2, 102\n"
                     "B, S, 12, 100";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( cancel_behind_best_bid_and_offer )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario15, balanced book, cancel behind best bid and offer\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# cancel orders, TOB = 10/11\n"
                 "C, 1, 2\n"
                 "C, 2, 101\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 2\n"  // was "C, 1, 2, 3\nA, 1, 3" in the outputFile.csv
                     "A, 2, 101";  // was "C, 2, 101, 103\nA, 2, 103" in the outputFile.csv
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}


BOOST_AUTO_TEST_CASE( cancel_all_bids )
{
    TestsOrderBooksProcessor processor;
    auto input = "#scenario16, balanced book, cancel all bids\n"
                 "\n"
                 "# build book, TOB = 10/11\n"
                 "N, 1, IBM, 10, 100, B, 1\n"
                 "N, 1, IBM, 12, 100, S, 2\n"
                 "N, 2, IBM, 9, 100, B, 101\n"
                 "N, 2, IBM, 11, 100, S, 102\n"
                 "\n"
                 "# cancel all bids, TOB = -/11\n"
                 "C, 1, 1\n"
                 "C, 2, 101\n"
                 "F";
    auto output = processor.feedLinesAndGetOutput(input);
    auto reference = "A, 1, 1\n"
                     "B, B, 10, 100\n"
                     "A, 1, 2\n"
                     "B, S, 12, 100\n"
                     "A, 2, 101\n"
                     "A, 2, 102\n"
                     "B, S, 11, 100\n"
                     "A, 1, 1\n"
                     "B, B, 9, 100\n"
                     "A, 2, 101\n"
                     "B, B, -, -";
    auto referenceLines = InputStringParser::splitLines(reference);
    auto outputLines = InputStringParser::splitLines(output);
    BOOST_CHECK_EQUAL_COLLECTIONS(referenceLines.begin(), referenceLines.end(),
                                  outputLines.begin(), outputLines.end());
}