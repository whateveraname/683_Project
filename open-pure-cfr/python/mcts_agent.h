#pragma once
#include <string>
#include <vector>
#include <memory>
#include <random>
#include <cassert>
#include "../omp/HandEvaluator.h"

class MCTSAgent {
public:
    MCTSAgent(double epsilon, int time_limit_ms);

    int search(const std::string& state, const std::string& hand, int num_actions,
               int pot_size, int call_amount, int invested,
               const std::vector<double>& prior, double fold_p);

    int get_simulation_count() const;

private:
    double epsilon;
    int time_limit; // milliseconds
    omp::HandEvaluator evaluator;
    std::mt19937 rng;

    int simulation_count_this_round;

    struct MCTSNode {
        std::string state;
        std::string hand;
        int num_actions;
        int pot_size;
        int call_amount;
        int invested;

        std::vector<int> visit_count;
        std::vector<double> total_value;
        std::vector<MCTSNode*> children;
        std::vector<double> prior;

        MCTSNode(const std::string& s, const std::string& h, int a,
                 int pot, int call_amt, int inv)
            : state(s), hand(h), num_actions(a),
              pot_size(pot), call_amount(call_amt), invested(inv) {
            assert(num_actions > 0);
            visit_count.resize(num_actions, 0);
            total_value.resize(num_actions, 0.0);
            children.resize(num_actions, nullptr);
            prior.resize(num_actions, 1.0 / num_actions); // default uniform prior
        }

        ~MCTSNode() {
            for (auto* child : children) {
                delete child;
            }
        }
    };

    int select_action(MCTSNode* node);

    // Updated to match cpp implementation
    void simulate(MCTSNode* node, double fold_p);
    double rollout(MCTSNode* node, int action, double fold_p);

    double evaluate_equity(const std::string& hand);
};
