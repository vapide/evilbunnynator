#include "search.hpp"

#include <cassert>

#include "../movegen/movegen.hpp"

Move Search::find_best_move(Position& pos, const SearchLimits& search_limits) {
  SearchLimits limits;
  limits = search_limits;

  clear_for_search();

  if (limits.depth <= 0) {
    limits.depth = DEFAULT_DEPTH;
  } else {
    limits.depth = std::clamp(limits.depth, 0, MAX_PLY - 2);
  }

  const int entry_ply = pos.ply;

  Move root_moves[MAX_PLY];
  int best_score;

  int count = MoveGen::generate_legal_moves(pos, root_moves);

  if (count == 0) {
    if (pos.in_check(pos.side_to_move) == true) {
      best_score = -INF_SCORE;
    } else {
      best_score = DRAW;
    }
    return MOVE_NONE;
  }

  Move best_move = root_moves[0];
  int best = -INF_SCORE;

  // root loop
  for (int i = 0; i < count; ++i) {
    do_move(pos, root_moves[i]);
    const int score = -negamax(pos, limits.depth - 1, -INF_SCORE, INF_SCORE, 1);
    undo_move(pos);
    if (stop.load(std::memory_order_relaxed)) {
      break;
    } else if (score > best) {
      best = score;
      best_move = root_moves[i];
    }
  }

  assert(pos.ply = entry_ply);
  // for NDEBUG builds
  (void)entry_ply;

  if (best > -INF_SCORE) {
    best_score = best;
  }

  return best_move;
}

void Search::reset() {
  clear_for_search();
  stop.store(false, std::memory_order_relaxed);
}

void Search::clear_for_search() {
  nodes = 0;
  best_score = EVAL_NONE;
}
