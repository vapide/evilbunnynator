#include "../movegen/movegen.hpp"
#include "search.hpp"

// no depth to avoid horizon effect, which is what quiescence focuses on
// eliminating.
// constraints: search Position pos, do not modify it; returned score is pov of
// stm in pov, just likenegamax; alpha and beta are the same as in negamax; ply
// is distance from root, not depth, and it keeps counting through the boundary
// set--the qnode below a negamax node at ply 4 is ply 5.

int Search::quiescence(Position& pos, int alpha, int beta, int ply) {
  if (stop.load(std::memory_order_relaxed)) return 0;

  pv.reset_line(ply);

  ++nodes;
  ++qnodes;

  if (ply > seldepth) seldepth = ply;

  // same as negamax, but with a nuance-there is no depth counter that runs out,
  // it returns an eval instead of a mate or draw score, because the search ceil
  // is not making a claim, its more like giving up
  if (ply >= MAX_PLY - 1) return evaluate(pos);

  const bool in_check = pos.in_check(pos.side_to_move);

  int best_score;
  Move moves[MAX_MOVES];
  int count;

  if (in_check) {
    count = MoveGen::generate_legal_moves(pos, moves);
    if (count == 0) return -CHECKMATE + ply;
    best_score = -CHECKMATE;
  } else {
    best_score = evaluate(pos);
    count = MoveGen::generate_legal_captures(pos, moves);
  }
  // likely to be the only initialization of best score that isnt -INF_SCORE
  // this is because node has score already, and stm may decline the capture.

  if (best_score >= beta) return best_score;

  if (best_score > alpha) alpha = best_score;

  for (int i = 0; i < count; ++i) {
    do_move(pos, moves[i]);
    const int score = -quiescence(pos, -beta, -alpha, ply + 1);
    undo_move(pos);
    if (stop.load(std::memory_order_relaxed)) {
      // ok here because we already have a valid score, not -INF_SCORE;
      return best_score;
    }

    if (score > best_score) {
      best_score = score;
    }

    if (score > alpha) {
      alpha = score;
    }

    if (score >= beta) {
      return score;
    }
  }

  return best_score;
}