// mcts_test.cpp — stand‑alone harness for your Monte‑Carlo tree search
// ---------------------------------------------------------------------
// Build example (adjust paths to your sources):
//   g++ -std=c++17 -O2 -march=native -fopenmp \
//       mcts_test.cpp mcts_agent.cpp ../omp/HandEvaluator.cpp \
//       -I../include -I../omp -o mcts_test
//
// Run:
//   ./mcts_test  [<time‑limit‑ms>]
//
// The program drives MCTSAgent directly, prints the chosen action, the
// number of simulations carried out, and the wall‑clock time actually
// spent inside MCTS.  Nothing is routed through pybind11 or pypokerengine,
// so you can step through this with gdb/lldb or attach Valgrind without
// the Python layer in the way.

#include "mcts_agent.h"
#include <chrono>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    // --- 1.  Dummy game state ------------------------------------------------
    // You can of course substitute real values captured from the Python
    // side; this minimal string keeps the parser happy.
    std::string state = "/";          // pre‑flop root
    std::string hand  = "SASKS2D2H3";        // "Ace spades, Ace clubs" (2‑card string)
                                        // NB: evaluator only looks at ranks.
    constexpr int NUM_ACTIONS = 3;      // 0=fold, 1=call, 2=raise

    double pot_size   = 30.0;           // small dummy pot
    double call_amt   = 10.0;           // amount to call
    double invested   = 0.0;            // hero has not invested yet

    // Uniform priors — use whatever StrategyParser gives you in production
    std::vector<double> prior(NUM_ACTIONS, 1.0 / NUM_ACTIONS);

    // --- 2.  Create MCTS agent ----------------------------------------------
    int time_budget_ms = (argc == 2 ? std::atoi(argv[1]) : 95);
    MCTSAgent mcts(/*epsilon=*/0.1, time_budget_ms);

    // We use steady_clock because it cannot go backwards.
    auto t0 = std::chrono::steady_clock::now();
    int best_action = mcts.search(state, hand, NUM_ACTIONS,
                                  pot_size, call_amt, invested, prior);
    auto t1 = std::chrono::steady_clock::now();

    // --- 3.  Report ----------------------------------------------------------
    std::cout << "Chosen action : " << best_action << "\n"
              << "Simulations   : " << mcts.get_simulation_count() << "\n"
              << "Wall‑time ms  : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << "\n" << std::flush;
    return 0;
}
