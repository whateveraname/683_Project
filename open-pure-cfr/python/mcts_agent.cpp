#include "mcts_agent.h"
#include <unordered_set>
#include <algorithm>
#include <chrono>
#include <cctype>
#include <iostream>
#include <numeric>
#include <limits>
#include <vector>

MCTSAgent::MCTSAgent(double e, int ms)
    : epsilon(e), time_limit(ms), rng(std::random_device{}()), simulation_count_this_round(0) {}

int MCTSAgent::search(const std::string& state, const std::string& hand,
                      int num_actions,
                      int pot_size, int call_amount, int invested,
                      const std::vector<double>& prior, double fold_p)
{
    simulation_count_this_round = 0;
    auto root = std::make_unique<MCTSNode>(state, hand, num_actions, pot_size, call_amount, invested);
    root->prior = prior;

    using Clock = std::chrono::steady_clock;
    const auto deadline = Clock::now() + std::chrono::milliseconds(time_limit);

    root->visit_count[0] = 1;  // fold
    root->total_value[0] = 0.0;  // fold

    while (Clock::now() < deadline)
        simulate(root.get(), fold_p);

    std::vector<double> q_values(num_actions, 0.0);  // very low for unvisited
    std::vector<double> probs(num_actions, 0.0);
    double sum_exp = 0.0;

    for (int i = 0; i < num_actions; ++i) {
        if (root->visit_count[i] > 0) {
            q_values[i] = root->total_value[i] / root->visit_count[i];
            std::cout << "Action " << i << ": Q-value = " << q_values[i] << ", Visits = " << root->visit_count[i] << std::endl;
        }
    }

    for (int i = 0; i < num_actions; ++i) {
        probs[i] = std::exp(q_values[i]);
        sum_exp += probs[i];
    }

    for (int i = 0; i < num_actions; ++i) {
        probs[i] /= sum_exp;
    }

    int best = std::distance(probs.begin(), std::max_element(probs.begin(), probs.end()));
    return probs[best] > 0.9 ? best : -1;
}

int MCTSAgent::select_action(MCTSNode* node) {
    int best_action = 0;
    double best_score = -std::numeric_limits<double>::infinity();
    int total_visits = std::accumulate(node->visit_count.begin(), node->visit_count.end(), 0);

    double c_puct = 2;

    for (int i = 1; i < node->num_actions; ++i) {
        double Q = (node->visit_count[i] == 0) ? 0.0 :
                   node->total_value[i] / node->visit_count[i];
        double U = c_puct * node->prior[i] * std::sqrt(total_visits + 1) / (1 + node->visit_count[i]);
        double score = Q + U;

        if (score > best_score) {
            best_score = score;
            best_action = i;
        }
    }

    return best_action;
}

void MCTSAgent::simulate(MCTSNode* node, double fold_p) {
    int action = select_action(node);

    if (!node->children[action]) {
        node->children[action] = new MCTSNode(node->state, node->hand,
            node->num_actions, node->pot_size, node->call_amount, node->invested);
    }

    double value = rollout(node->children[action], action, fold_p);
    node->visit_count[action]++;
    node->total_value[action] += value;
}

double MCTSAgent::rollout(MCTSNode* node, int action, double fold_p) {
    if (action == 0) {  // fold
        return 0.0;  
    }
    simulation_count_this_round++;
    const double equity = evaluate_equity(node->hand);
    // std::cout << "Hand " << node->hand << " equity: " << equity << std::endl;
    if (action == 1) {  // call / check
        double pot = static_cast<double>(node->pot_size);
        double call = static_cast<double>(node->call_amount);
        return equity * (pot + call) - call - (1 - equity) * double(node->invested);
    }

    // raise
    const int raise_base = (node->state.length() < 11) ? 20 : 40;
    const int raise_total = node->call_amount + raise_base;
    const double final_pot = static_cast<double>(node->pot_size + raise_total);

    // Apply fold probability logic
    double fold_reward = static_cast<double>(node->pot_size);
    double call_reward = equity * final_pot - raise_total - (1 - equity) * double(node->invested);
    return fold_p * fold_reward + (1.0 - fold_p) * call_reward;
}

double MCTSAgent::evaluate_equity(const std::string& hand_str) {
    omp::Hand self_card = omp::Hand::empty();
    omp::Hand board = omp::Hand::empty();
    std::unordered_set<int> used;

    // Parse player hand (first 4 chars)
    for (int i = 0; i < 4; i += 2) {
        char suit = std::tolower(hand_str[i]);
        char rank = hand_str[i + 1];
        int r = (rank >= '2' && rank <= '9') ? rank - '2' :
                (rank == 'T') ? 8 : (rank == 'J') ? 9 :
                (rank == 'Q') ? 10 : (rank == 'K') ? 11 : 12;
        int s = (suit == 'c') ? 0 : (suit == 'd') ? 1 : (suit == 'h') ? 2 : 3;
        int idx = s * 13 + r;
        self_card += omp::Hand(idx);
        used.insert(idx);
    }

    // Parse board
    for (size_t i = 4; i < hand_str.size(); i += 2) {
        char suit = std::tolower(hand_str[i]);
        char rank = hand_str[i + 1];
        int r = (rank >= '2' && rank <= '9') ? rank - '2' :
                (rank == 'T') ? 8 : (rank == 'J') ? 9 :
                (rank == 'Q') ? 10 : (rank == 'K') ? 11 : 12;
        int s = (suit == 'c') ? 0 : (suit == 'd') ? 1 : (suit == 'h') ? 2 : 3;
        int idx = s * 13 + r;
        board += omp::Hand(idx);
        used.insert(idx);
    }

    int board_cards = (hand_str.size() - 4) / 2;
    int wins = 0, ties = 0, samples = 1000;
    int self_rank = evaluator.evaluate(self_card + board);

    
    for (int i = 0; i < samples; ++i) {
        // Build fresh deck
        std::vector<int> deck;
        for (int j = 0; j < 52; ++j)
            if (!used.count(j)) deck.push_back(j);

        std::shuffle(deck.begin(), deck.end(), rng);

        omp::Hand sampled_board = board;

        // Sample river card if still on turn
        if (board_cards == 4) {
            int river_card = deck[2];  // avoid collision with opp cards
            sampled_board += omp::Hand(river_card);
            self_rank = evaluator.evaluate(self_card + sampled_board);
        }

        omp::Hand opp_hand = omp::Hand(deck[0]) + omp::Hand(deck[1]);

        
        int opp_rank = evaluator.evaluate(opp_hand + sampled_board);

        if (self_rank > opp_rank) ++wins;
        else if (self_rank == opp_rank) ++ties;
    }

    return (wins + 0.5 * ties) / samples;
}

int MCTSAgent::get_simulation_count() const {
    return simulation_count_this_round;
}
