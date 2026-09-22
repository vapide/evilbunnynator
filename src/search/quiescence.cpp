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

  // likely to be the only initialization of best score that isnt -INF_SCORE
  // this is because node has score already, and stm may decline the capture.
  best_score = evaluate(pos);

  if (best_score >= beta) {
    return best_score;
  } else if (best_score > alpha) {
    alpha = best_score;
  }

  return best_score;
}