extern "C" {
    #include "hand-isomorphism/src/hand_index.h"
}
#include "omp/EquityCalculator.h"
#include <iostream>
#include <fstream>
int main()
{
if (1) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    hand_indexer_t flop_indexer;
    uint8_t cards_per_round[] = {2, 3};
    hand_indexer_init(2, cards_per_round, &flop_indexer);
    std::vector<double> equities(hand_indexer_size(&flop_indexer, 1));
#pragma omp parallel for schedule(dynamic)
    for (hand_index_t i = 0; i < hand_indexer_size(&flop_indexer, 1); i++) {
        std::array<uint8_t, 5> cards;
        omp::EquityCalculator eq;
        if (!hand_unindex(&flop_indexer, 1, i, cards.data())) {
            std::cout << "unindex error\n";
            exit(0);
        }
        std::vector<omp::CardRange> hand{omp::CardRange({std::array<uint8_t, 2>{cards[0], cards[1]}}), {"random"}};
        auto boards = omp::CardRange::getCardMask(std::vector<uint8_t>{cards[2], cards[3], cards[4]});
        eq.start(hand, boards, 0, false, 5e-5, nullptr, 0, 1);
        auto r = eq.getResults();
        equities[i] = r.equity[0];
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() << "s\n";
    std::ofstream out("/mnt/nfs/work1/ameli/yanqichen/flop_equities.bin", std::ios::binary);
    if (!out) {
        std::cout << "cannot open file!\n";
        exit(0);
    }
    out.write(reinterpret_cast<char*>(equities.data()), equities.size() * sizeof(double));
    out.close();
}

if (1) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    hand_indexer_t turn_indexer;
    uint8_t cards_per_round[] = {2, 3, 1};
    hand_indexer_init(3, cards_per_round, &turn_indexer);
    std::vector<double> equities(hand_indexer_size(&turn_indexer, 2));
#pragma omp parallel for schedule(dynamic)
    for (hand_index_t i = 0; i < hand_indexer_size(&turn_indexer, 2); i++) {
        std::array<uint8_t, 6> cards;
        omp::EquityCalculator eq;
        if (!hand_unindex(&turn_indexer, 2, i, cards.data())) {
            std::cout << "unindex error\n";
            exit(0);
        }
        std::vector<omp::CardRange> hand{omp::CardRange({std::array<uint8_t, 2>{cards[0], cards[1]}}), {"random"}};
        auto boards = omp::CardRange::getCardMask(std::vector<uint8_t>{cards[2], cards[3], cards[4], cards[5]});
        eq.start(hand, boards, 0, false, 5e-5, nullptr, 0, 1);
        auto r = eq.getResults();
        equities[i] = r.equity[0];
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() << "s\n";
    std::ofstream out("/mnt/nfs/work1/ameli/yanqichen/turn_equities.bin", std::ios::binary);
    out.write(reinterpret_cast<char*>(equities.data()), equities.size() * sizeof(double));
    out.close();
}

if (1) {
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    hand_indexer_t river_indexer;
    uint8_t cards_per_round[] = {2, 3, 1, 1};
    hand_indexer_init(4, cards_per_round, &river_indexer);
    std::vector<double> equities(hand_indexer_size(&river_indexer, 3));
#pragma omp parallel for schedule(dynamic)
    for (hand_index_t i = 0; i < hand_indexer_size(&river_indexer, 3); i++) {
        if (i % (equities.size() / 1000) == 0) {
            std::chrono::high_resolution_clock::time_point t3 = std::chrono::high_resolution_clock::now();
            std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(t3 - t1).count() << "s\n";
        }
        std::array<uint8_t, 7> cards;
        omp::EquityCalculator eq;
        if (!hand_unindex(&river_indexer, 3, i, cards.data())) {
            std::cout << "unindex error\n";
            exit(0);
        }
        std::vector<omp::CardRange> hand{omp::CardRange({std::array<uint8_t, 2>{cards[0], cards[1]}}), {"random"}};
        auto boards = omp::CardRange::getCardMask(std::vector<uint8_t>{cards[2], cards[3], cards[4], cards[5], cards[6]});
        eq.start(hand, boards, 0, false, 5e-5, nullptr, 0, 1);
        auto r = eq.getResults();
        equities[i] = r.equity[0];
    }
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::seconds>(t2 - t1).count() << "s\n";
    std::ofstream out("/mnt/nfs/work1/ameli/yanqichen/river_equities.bin", std::ios::binary);
    out.write(reinterpret_cast<char*>(equities.data()), equities.size() * sizeof(double));
    out.close();
}
}