#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cassert>
#include <numeric>
#include <cstdlib>
#include "mcts_agent.h"
#include <omp.h>
#include <chrono>

extern "C" {
#include "../hand-isomorphism/src/hand_index.h"
}

#include "../omp/EquityCalculator.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;

class StrategyParser {
public:
    StrategyParser(const std::string& strategy_file, const std::string& pos_table_file) {
        int result = std::system("tar -xzvf strategy.tar.gz");
        if (result == 0) {
            std::cout << "Unzipped successfully.\n";
        } else {
            std::cerr << "Unzip failed.\n";
        }
        load_strategy(strategy_file);
        load_pos_table(pos_table_file);
        load_bucket_table();
        mcts_agent = std::make_unique<MCTSAgent>(0.1, 95);
    }

    int parse(const std::string& state, const std::string& hand, int num_actions,
          bool use_mcts, int pot_size, int call_amount, int invested,
          bool verbose) {
        int num_rounds = std::count(state.begin(), state.end(), '/');
        if (num_rounds <= 2) { use_mcts = false; }

        int bucket = hand_to_bucket(hand, num_rounds);
        std::vector<uint64_t> raw_probs(num_actions, 1);
        get_action_probs(state, bucket, num_rounds, num_actions, raw_probs.data());

        std::vector<double> prior = get_normalized_action_probs(raw_probs);

        if (use_mcts) {
            auto start = std::chrono::high_resolution_clock::now();
            std::string new_state = state + "r";  // future state after raise
            double fold_p = this->opp_fold_prob(new_state, bucket, num_rounds);

            int action = mcts_agent->search(state, hand, num_actions, pot_size, call_amount, invested, prior, fold_p);
            if (verbose) {
                auto end = std::chrono::high_resolution_clock::now();
                std::cout << "Simulation count: " << mcts_agent->get_simulation_count() << std::endl;
                std::cout << "Time taken: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms" << std::endl;
            }
            if (action != -1) {
                return action;
            }
        }

        int max_action = std::distance(prior.begin(), std::max_element(prior.begin(), prior.end()));
        return max_action;
    }


protected:
    int get_bucket(float equity, std::vector<float>& boundary) {
        auto it = std::lower_bound(boundary.begin(), boundary.end(), equity);
        int index = it - boundary.begin();
        return index;
    }

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
        {
            std::array<uint8_t, 5> cards;
            omp::EquityCalculator eq;
            hand_unindex(&indexers[round - 1], 1, index, cards.data());
            std::vector<omp::CardRange> hands{omp::CardRange({std::array<uint8_t, 2>{cards[0], cards[1]}}), {"random"}};
            auto boards = omp::CardRange::getCardMask(std::vector<uint8_t>{cards[2], cards[3], cards[4]});
            eq.start(hands, boards, 0, false, 5e-5, nullptr, 0, 1);
            auto r = eq.getResults();
            auto equity = r.equity[0];
            bucket = get_bucket(equity, flop_bucket_table);
        }
            break;
        case 3:
        {
            std::array<uint8_t, 6> cards;
            omp::EquityCalculator eq;
            hand_unindex(&indexers[round - 1], 2, index, cards.data());
            std::vector<omp::CardRange> hands{omp::CardRange({std::array<uint8_t, 2>{cards[0], cards[1]}}), {"random"}};
            auto boards = omp::CardRange::getCardMask(std::vector<uint8_t>{cards[2], cards[3], cards[4], cards[5]});
            eq.start(hands, boards, 0, false, 5e-5, nullptr, 0, 1);
            auto r = eq.getResults();
            auto equity = r.equity[0];
            bucket = get_bucket(equity, turn_bucket_table);
        }
            break;
        case 4:
        {
            std::array<uint8_t, 7> cards;
            omp::EquityCalculator eq;
            hand_unindex(&indexers[round - 1], 3, index, cards.data());
            std::vector<omp::CardRange> hands{omp::CardRange({std::array<uint8_t, 2>{cards[0], cards[1]}}), {"random"}};
            auto boards = omp::CardRange::getCardMask(std::vector<uint8_t>{cards[2], cards[3], cards[4], cards[5], cards[6]});
            eq.start(hands, boards, 0, false, 5e-5, nullptr, 0, 1);
            auto r = eq.getResults();
            auto equity = r.equity[0];
            bucket = get_bucket(equity, river_bucket_table);
        }
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

    void get_action_probs(const std::string& state, int bucket, int round, int num_actions, uint64_t* probs) {
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
    std::vector<double> get_normalized_action_probs(const std::vector<uint64_t>& action_probs) {
        assert(!action_probs.empty());
        double total = std::accumulate(action_probs.begin(), action_probs.end(), 0.0);
        std::vector<double> normalized(action_probs.size(), 1.0 / action_probs.size());

        if (total > 0) {
            for (size_t i = 0; i < action_probs.size(); ++i) {
                normalized[i] = static_cast<double>(action_probs[i]) / total;
            }
        }
        return normalized;
    }
    double opp_fold_prob(const std::string& state, int bucket, int round) const {
        auto it = pos_table.find(state);
        if (it == pos_table.end()) return 0.0;
    
        int p = it->second ^ 1;  // opponent position
    
        if (round == 1) {
            // preflop is std::vector<uint64_t>
            const uint64_t* row = &preflop[bucket * 21 + p];
            double s = static_cast<double>(row[0]) + row[1] + row[2];
            return s ? static_cast<double>(row[0]) / s : 0.0;
        }
    
        // flop, turn, river are std::vector<uint32_t>
        const uint32_t* row = nullptr;
        if (round == 2) row = &flop[bucket * 182 + p];
        else if (round == 3) row = &turn[bucket * 1512 + p];
        else if (round == 4) row = &river[bucket * 8028 + p];
        else return 0.0;
    
        double s = static_cast<double>(row[0]) + row[1] + row[2];
        return s ? static_cast<double>(row[0]) / s : 0.0;
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
        flop_bucket_table.resize(4999);
        turn_bucket_table.resize(4999);
        river_bucket_table.resize(4999);
        uint8_t cards_per_round[] = {2, 3, 1, 1};
        for (int i = 0; i < 4; i++) {
            hand_indexer_init(i + 1, cards_per_round, &indexers[i]);
        }
        std::ifstream flop_bucket_file("flop_buckets.bin", std::ios::binary);
        if (!flop_bucket_file) {
            std::cout << "Error opening file: flop_buckets.bin" << std::endl;
            return;
        }
        std::ifstream turn_bucket_file("turn_buckets.bin", std::ios::binary);
        std::ifstream river_bucket_file("river_buckets.bin", std::ios::binary);
        flop_bucket_file.read(reinterpret_cast<char*>(flop_bucket_table.data()), flop_bucket_table.size() * sizeof(float));
        turn_bucket_file.read(reinterpret_cast<char*>(turn_bucket_table.data()), turn_bucket_table.size() * sizeof(float));
        river_bucket_file.read(reinterpret_cast<char*>(river_bucket_table.data()), river_bucket_table.size() * sizeof(float));
    }
    std::unique_ptr<MCTSAgent> mcts_agent;

    std::vector<uint64_t> preflop;
    std::vector<uint32_t> flop;
    std::vector<uint32_t> turn;
    std::vector<uint32_t> river;

    std::unordered_map<std::string, int> pos_table;

    hand_indexer_t indexers[MAX_ROUNDS];
    std::vector<float> flop_bucket_table;
    std::vector<float> turn_bucket_table;
    std::vector<float> river_bucket_table;
};

PYBIND11_MODULE(strategy_parser, m) {
    py::class_<StrategyParser>(m, "StrategyParser")
        .def(py::init<const std::string&, const std::string&>())
        .def("parse", &StrategyParser::parse);
}