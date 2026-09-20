#include "perft.hpp"

#include <cstdio>

#include "../core/move.hpp"
#include "../core/position.hpp"
#include "movegen.hpp"

namespace Perft {

uint64_t perft(Position& pos, int depth) {
  if (depth == 0) return 1;

  Move moves[MAX_MOVES];
  const int count = MoveGen::generate_legal_moves(pos, moves);

  // bulk counting: depth 1 number of legal moves is how many moves there are
  // skips one layer of making/unmaking
  if (depth == 1) return count;

  uint64_t nodes = 0;
  for (int i = 0; i < count; ++i) {
    pos.make_move(moves[i]);
    nodes += perft(pos, depth - 1);
    pos.unmake_move();
  }
  return nodes;
}

// changed from perft_divide because of new namespace
uint64_t divide(Position& pos, int depth) {
  Move moves[MAX_MOVES];
  const int count = MoveGen::generate_legal_moves(pos, moves);

  uint64_t total = 0;
  for (int i = 0; i < count; ++i) {
    pos.make_move(moves[i]);
    const uint64_t nodes = depth > 1 ? perft(pos, depth - 1) : 1;
    pos.unmake_move();
    std::printf("%s: %llu\n", to_uci(moves[i]).c_str(),
                (unsigned long long)nodes);
    total += nodes;
  }
  // already have total in main perft function in engine.cpp
  // std::printf("total: %llu\n", (unsigned long long)total);
  return total;
}

}  // namespace Perft