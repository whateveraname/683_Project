#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cassert>
#include <numeric>

extern "C" {
#include "../hand-isomorphism/src/hand_index.h"
}

#include <pybind11/pybind11.h>

namespace py = pybind11;

class StrategyParser {
public:
    StrategyParser(const std::string& strategy_file, const std::string& pos_table_file) {
        load_strategy(strategy_file);
        load_pos_table(pos_table_file);
        load_bucket_table();
    }

    int parse(const std::string& state, const std::string& hand, int num_actions) {
        // count number of "/"s in state
        int num_rounds = 0;
        for (char c : state) {
            if (c == '/') {
                num_rounds++;
            }
        }
        int probs[num_actions];
        int bucket = hand_to_bucket(hand, num_rounds);
        get_action_probs(state, bucket, num_rounds, num_actions, probs);
        // Print action probabilities
        // std::cout << "Action probabilities for state: " << state << ", hand: " << hand << std::endl;
        // for (int i = 0; i < num_actions; ++i) {
        //     std::cout << "Action " << i << ": " << probs[i] << std::endl;
        // }
        // return the action with max probability
        int max_action = 0;
        for (int i = 1; i < num_actions; ++i) {
            if (probs[i] > probs[max_action]) {
                max_action = i;
            }
        }
        return max_action;
    }

protected:
    int hand_to_bucket(const std::string& hand, int round) {
        std::vector<uint8_t> cards(7);
        parse_hand(hand, round, cards.data());
        size_t index = hand_index_last(&indexers[round - 1], cards.data());
        int bucket = 0;
        switch (round) {
        case 1:
            bucket = index;
            break;
        case 2:
            flop_bucket_table.seekg(index * sizeof(int), std::ios::beg);
            flop_bucket_table.read(reinterpret_cast<char*>(&bucket), sizeof(int));
            break;
        case 3:
            turn_bucket_table.seekg(index * sizeof(int), std::ios::beg);
            turn_bucket_table.read(reinterpret_cast<char*>(&bucket), sizeof(int));
            break;
        case 4:
            river_bucket_table.seekg(index * sizeof(int), std::ios::beg);
            river_bucket_table.read(reinterpret_cast<char*>(&bucket), sizeof(int));
            break;
        default:
            break;
        }
        return bucket;
    }

    void parse_hand(const std::string& hand, int round, uint8_t* cards) {
        int num_cards = 2;
        if (round > 1) {
            num_cards += 1 + round;
        }
        assert(num_cards == hand.length() / 2);
        for (int i = 0; i < num_cards; ++i) {
            char suit = hand[i * 2];
            char rank = hand[i * 2 + 1];
            if (suit == 'S') {
                cards[i] = 0;
            } else if (suit == 'H') {
                cards[i] = 1;
            } else if (suit == 'D') {
                cards[i] = 2;
            } else if (suit == 'C') {
                cards[i] = 3;
            } else {
                std::cout << "Invalid suit: " << suit << std::endl;
            }
            if (rank >= '2' && rank <= '9') {
                cards[i] |= ((rank - '2')<<2);
            } else if (rank == 'T') {
                cards[i] |= 32;
            } else if (rank == 'J') {
                cards[i] |= 36;
            } else if (rank == 'Q') {
                cards[i] |= 40;
            } else if (rank == 'K') {
                cards[i] |= 44;
            } else if (rank == 'A') {
                cards[i] |= 48;
            } else {
                std::cout << "Invalid rank: " << rank << std::endl;
            }
        }
    }

    void get_action_probs(const std::string& state, int bucket, int round, int num_actions, int* probs) {
        switch (round) {
            case 1:
                // Preflop
                if (pos_table.find(state) != pos_table.end()) {
                    int pos = pos_table[state];
                    auto it = preflop.data() + bucket * 21 + pos;
                    for (int i = 0; i < num_actions; ++i) {
                        probs[i] = *it;
                        it++;
                    }
                } else {
                    std::cout << "State not found in preflop table: " << state << std::endl;
                }
                break;
            case 2:
                // Flop
                if (pos_table.find(state) != pos_table.end()) {
                    int pos = pos_table[state];
                    auto it = flop.data() + bucket * 182 + pos;
                    for (int i = 0; i < num_actions; ++i) {
                        probs[i] = *it;
                        it++;
                    }
                } else {
                    std::cout << "State not found in flop table: " << state << std::endl;
                }
                break;
            case 3:
                // Turn
                if (pos_table.find(state) != pos_table.end()) {
                    int pos = pos_table[state];
                    auto it = turn.data() + bucket * 1512 + pos;
                    for (int i = 0; i < num_actions; ++i) {
                        probs[i] = *it;
                        it++;
                    }
                } else {
                    std::cout << "State not found in turn table: " << state << std::endl;
                }
                break;
            case 4:
                // River
                if (pos_table.find(state) != pos_table.end()) {
                    int pos = pos_table[state];
                    auto it = river.data() + bucket * 8028 + pos;
                    for (int i = 0; i < num_actions; ++i) {
                        probs[i] = *it;
                        it++;
                    }
                } else {
                    std::cout << "State not found in river table: " << state << std::endl;
                }
                break;
            default:
                std::cout << "Invalid number of rounds: " << round << std::endl;
                break;
        }
    }

    void load_strategy(const std::string& strategy_file) {
        preflop.resize(3549);
        flop.resize(910000);
        turn.resize(7560000);
        river.resize(40140000);

        // Load the strategy from file
        std::ifstream file(strategy_file, std::ios::binary);
        if (!file) {
            std::cout << "Error opening file: " << strategy_file << std::endl;
            return;
        }
        file.seekg(4, std::ios::cur);
        file.read(reinterpret_cast<char*>(preflop.data()), preflop.size() * sizeof(uint64_t));
        file.seekg(4, std::ios::cur);
        file.read(reinterpret_cast<char*>(flop.data()), flop.size() * sizeof(uint32_t));
        file.seekg(4, std::ios::cur);
        file.read(reinterpret_cast<char*>(turn.data()), turn.size() * sizeof(uint32_t));
        file.seekg(4, std::ios::cur);
        file.read(reinterpret_cast<char*>(river.data()), river.size() * sizeof(uint32_t));
        if (!file) {
            std::cout << "Error reading file: " << strategy_file << std::endl;
            return;
        }
        file.close();
    }

    void load_pos_table(const std::string& pos_table_file) {
        // Load the position table from file
        std::ifstream file(pos_table_file);
        if (!file) {
            std::cout << "Error opening file: " << pos_table_file << std::endl;
            return;
        }
        int num_entries;
        while (file) {
            std::string state;
            int pos;
            file >> state >> pos;
            if (file) {
                pos_table[state] = pos;
                num_entries++;
            }
        }
        assert(num_entries == 4042);
        file.close();
    }

    void load_bucket_table() {
        uint8_t cards_per_round[] = {2, 3, 1, 1};
        for (int i = 0; i < 4; i++) {
            hand_indexer_init(i + 1, cards_per_round, &indexers[i]);
        }
        flop_bucket_table.open("/mnt/nfs/work1/ameli/yanqichen/flop_bucket_table.bin", std::ios::binary);
        if (!flop_bucket_table) {
            std::cout << "Error opening file: flop_bucket_table.bin" << std::endl;
            return;
        }
        turn_bucket_table.open("/mnt/nfs/work1/ameli/yanqichen/turn_bucket_table.bin", std::ios::binary);
        river_bucket_table.open("/mnt/nfs/work1/ameli/yanqichen/river_bucket_table.bin", std::ios::binary);
    }

    std::vector<uint64_t> preflop;
    std::vector<uint32_t> flop;
    std::vector<uint32_t> turn;
    std::vector<uint32_t> river;

    std::unordered_map<std::string, int> pos_table;

    hand_indexer_t indexers[MAX_ROUNDS];
    std::ifstream flop_bucket_table;
    std::ifstream turn_bucket_table;
    std::ifstream river_bucket_table;
};

PYBIND11_MODULE(strategy_parser, m) {
    py::class_<StrategyParser>(m, "StrategyParser")
        .def(py::init<const std::string&, const std::string&>())
        .def("parse", &StrategyParser::parse);
}