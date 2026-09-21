#include <algorithm>

#include "../movegen/movegen.hpp"
#include "search.hpp"

int Search::negamax(Position& pos, int depth, int alpha, int beta, int ply) {
  Move moves[MAX_PLY];

  if (stop.load(std::memory_order_relaxed)) {
    return 0;
  }
  ++nodes;
  if (depth <= 0) {
    return evaluate(pos);
  }
  int count = MoveGen::generate_legal_moves(pos, moves);
  int best_score = -INF_SCORE;
  for (int i = 0; i < count; ++i) {
    do_move(pos, moves[i]);
    const int score =
        std::max(best_score, -negamax(pos, depth - 1, -alpha, -beta, ply + 1));
    undo_move(pos);

    if (stop.load(std::memory_order_relaxed)) {
      return best_score != -INF_SCORE ? best_score : 0;
    }

    if (score > best_score) {
      best_score = score;
    }
  }
}