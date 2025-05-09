#ifndef STRATEGY_PARSER_H
#define STRATEGY_PARSER_H

#include <string>
#include <memory>
#include "mcts_agent.h"

class StrategyParser {
public:
    StrategyParser(const std::string& strategy_file, const std::string& pos_table_file);

    int parse(const std::string& state, const std::string& hand, int num_actions,
              bool use_mcts, int pot_size, int call_amount, int invested,
              bool verbose);

private:
    std::unique_ptr<MCTSAgent> mcts_agent;
};

#endif // STRATEGY_PARSER_H
