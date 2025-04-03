#include <fstream>
#include <vector>
#include <numeric>
#include "card_abstraction.hpp"

extern "C" {
#include "hand-isomorphism/src/hand_index.h"
}

class BucketingAbstraction : public CardAbstraction {
public:

  BucketingAbstraction( const Game *game ) {
    m_num_buckets[0] = 169;  m_num_buckets[1] = 5000;
    m_num_buckets[2] = 5000; m_num_buckets[3] = 5000;
    uint8_t cards_per_round[] = {2, 3, 1, 1};
    for (int i = 0; i < 4; i++) {
      hand_indexer_init(i + 1, cards_per_round, &indexers[i]);
    }
    bucket_tables.resize(4);
    bucket_tables[0].resize(169);
    std::iota(bucket_tables[0].begin(), bucket_tables[0].end(), 0);
    {
      std::ifstream in("/mnt/nfs/work1/ameli/yanqichen/flop_bucket_table.bin", std::ios::binary);
      size_t size = 1286792;
      bucket_tables[1].resize(size);
      in.read((char*)bucket_tables[1].data(), size * sizeof(int));
    }
    {
      std::ifstream in("/mnt/nfs/work1/ameli/yanqichen/turn_bucket_table.bin", std::ios::binary);
      size_t size = 55190538;
      bucket_tables[2].resize(size);
      in.read((char*)bucket_tables[2].data(), size * sizeof(int));
    }
    {
      std::ifstream in("/mnt/nfs/work1/ameli/yanqichen/river_bucket_table.bin", std::ios::binary);
      size_t size = 2428287420;
      bucket_tables[3].resize(size);
      in.read((char*)bucket_tables[3].data(), size * sizeof(int));
    }
  }
  virtual ~BucketingAbstraction() {}

  virtual int num_buckets( const Game *game, const BettingNode *node ) const {
    return m_num_buckets[node->get_round()];
  }
  virtual int num_buckets( const Game *game, const State &state ) const {
    return m_num_buckets[state.round];
  }
  virtual int get_bucket( const Game *game,
			  const BettingNode *node,
			  const uint8_t board_cards[ MAX_BOARD_CARDS ],
			  const uint8_t hole_cards[ MAX_PURE_CFR_PLAYERS ]
			  [ MAX_HOLE_CARDS ] ) const {
    uint8_t cards[7];
    memcpy(cards, hole_cards[node->get_player()], 2 * sizeof(uint8_t));
    memcpy(cards + 2, board_cards, 5 * sizeof(uint8_t));
    size_t index = index_hand(cards, node->get_round());
    return bucket_tables[node->get_round()][index];
  }
  virtual bool can_precompute_buckets( ) const { return false; }
  virtual void precompute_buckets( const Game *game,
				   hand_t &hand ) const {};

protected:
  virtual size_t index_hand(const uint8_t cards[], const int round) const {
    return hand_index_last(&indexers[round], cards);
  }

  size_t m_num_buckets[MAX_ROUNDS];
  hand_indexer_t indexers[MAX_ROUNDS];
  std::vector<std::vector<int>> bucket_tables;
};