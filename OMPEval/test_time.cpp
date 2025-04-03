extern "C" {
    #include "hand-isomorphism/src/hand_index.h"
}
#include "omp/EquityCalculator.h"
#include <iostream>
#include <fstream>
int main() {
    hand_indexer_t river_indexer;
    uint8_t cards_per_round[] = {2, 3, 1, 1};
    hand_indexer_init(4, cards_per_round, &river_indexer);
    // uint8_t cards[7];
    // cards[0]=0;cards[1]=1;cards[2]=2;cards[3]=3;cards[4]=4;cards[5]=5;cards[6]=6;
    std::array<uint8_t, 7> cards;
    omp::EquityCalculator eq;
    if (!hand_unindex(&river_indexer, 3, 19999, cards.data())) {
        std::cout << "unindex error\n";
        exit(0);
    }
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    // hand_index_last(&river_indexer, cards);
    std::vector<omp::CardRange> hand{omp::CardRange({std::array<uint8_t, 2>{cards[0], cards[1]}}), {"random"}};
    auto boards = omp::CardRange::getCardMask(std::vector<uint8_t>{cards[2], cards[3], cards[4], cards[5], cards[6]});
    eq.start(hand, boards, 0, false, 1e-5, nullptr, 0, 1);
    auto r = eq.getResults();
    std::cout << r.hands << " " << r.time << " " << r.stdev << std::endl;
    std::cout << r.equity[0] << "\n";
    std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() << "ms\n";
}