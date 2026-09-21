#include <algorithm>

#include "../movegen/movegen.hpp"
#include "search.hpp"

int Search::negamax(Position& pos, int depth, int alpha, int beta, int ply) {
  if (stop.load(std::memory_order_relaxed)) return 0;
  ++nodes;

  if (ply >= MAX_PLY - 1) {
    return evaluate(pos);
  }

  Move moves[MAX_MOVES];
  const int count = MoveGen::generate_legal_moves(pos, moves);

  // checkmate or stalemate
  if (count == 0) {
    if (pos.in_check(pos.side_to_move)) {
      return -CHECKMATE + ply;
    }
    return DRAW;
  }

  if (depth <= 0) {
    return evaluate(pos);
  }

  int best_score = -INF_SCORE;

  for (int i = 0; i < count; ++i) {
    do_move(pos, moves[i]);
    // swapped by negation
    const int score = -negamax(pos, depth - 1, -beta, -alpha, ply + 1);
    undo_move(pos);

    if (stop.load(std::memory_order_relaxed)) {
      return best_score > -INF_SCORE ? best_score : 0;
    }

    if (score > best_score) {
      best_score = score;
    }
    if (score > alpha) {
      alpha = score;
    }
    if (alpha >= beta) {
      break;
    }
  }
  return best_score;
}