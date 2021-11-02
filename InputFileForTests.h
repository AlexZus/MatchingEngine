//
// Created by alex on 02.10.21.
//

#ifndef ORDERBOOKSERVER_INPUTFILEFORTESTS_H
#define ORDERBOOKSERVER_INPUTFILEFORTESTS_H

constexpr auto inputLinesFromFile
 = "#Format new order:\n"
   "# N, user(int),symbol(string),price(int),qty(int),side(char B or S),userOrderId(int)\n"
   "#\n"
   "#Format cancel order:\n"
   "# C, user(int),userOrderId(int)\n"
   "#\n"
   "#Format flush order book:\n"
   "# F\n"
   "\n"
   "# Notes:\n"
   "# * Price is 0 for market order, <>0 for limit order\n"
   "# * TOB = Top Of Book, highest bid, lowest offer\n"
   "# * Between scenarios flush order books\n"
   "\n"
   "#scenario1, balanced book\n"
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
   "F\n"
   "\n"
   "#scenario2, shallow bid\n"
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
   "F\n"
   "\n"
   "#scenario3, shallow ask\n"
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
   "F\n"
   "\n"
   "#scenario4, balanced book, limit below best bid\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# limit below best bid, generate trade, TOB = 9/11\n"
   "N, 2, IBM, 9, 100, S, 103\n"
   "F\n"
   "\n"
   "#scenario5, balanced book, limit above best ask\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# limit above best ask, generate trade, TOB = 10/12\n"
   "N, 1, IBM, 12, 100, B, 103\n"
   "F\n"
   "\n"
   "#scenario6, balanced book, market sell\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# market sell, generate trade, TOB = 9/11\n"
   "N, 2, IBM, 0, 100, S, 103\n"
   "F\n"
   "\n"
   "#scenario7, balanced book, market buy\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# market buy, generate trade, TOB = 10/12\n"
   "N, 1, IBM, 0, 100, B, 3\n"
   "F\n"
   "\n"
   "#scenario8, tighten spread through new limit orders\n"
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
   "F\n"
   "\n"
   "#scenario9, balanced book, market sell partial\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# market sell, generate partial trade, TOB = 10/11\n"
   "N, 2, IBM, 0, 20, S, 103\n"
   "F\n"
   "\n"
   "#scenario10, balanced book, market buy partial\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# market buy, generate partial trade, TOB = 10/11\n"
   "N, 1, IBM, 0, 20, B, 3\n"
   "F\n"
   "\n"
   "#scenario11, balanced book, limit sell partial\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# limit sell, generate partial trade, TOB = 10/11\n"
   "N, 2, IBM, 10, 20, S, 103\n"
   "F\n"
   "\n"
   "#scenario12, balanced book, limit buy partial\n"
   "\n"
   "# build book, TOB = 10/11\n"
   "N, 1, IBM, 10, 100, B, 1\n"
   "N, 1, IBM, 12, 100, S, 2\n"
   "N, 2, IBM, 9, 100, B, 101\n"
   "N, 2, IBM, 11, 100, S, 102\n"
   "\n"
   "# limit buy, generate partial trade, TOB = 10/11\n"
   "N, 1, IBM, 11, 20, B, 3\n"
   "F\n"
   "\n"
   "#scenario13, balanced book multiple orders at best bid offer\n"
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
   "F\n"
   "\n"
   "#scenario14, balanced book, cancel best bid and offer\n"
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
   "F\n"
   "\n"
   "#scenario15, balanced book, cancel behind best bid and offer\n"
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
   "F\n"
   "\n"
   "#scenario16, balanced book, cancel all bids\n"
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
   "F\n"
   "\n"
   "";

#endif //ORDERBOOKSERVER_INPUTFILEFORTESTS_H
