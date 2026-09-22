#include <algorithm>
#include <iostream>

#include "../evaluation/material.hpp"
#include "../movegen/movegen.hpp"
#include "search.hpp"

int Search::negamax(Position& pos, int depth, int alpha, int beta, int ply) {
  if (stop.load(std::memory_order_relaxed)) return 0;

  pv.reset_line(ply);

  PlyRecord& rec = stack[ply];

  if (ply > seldepth) {
    seldepth = ply;
    // std::cout << "info string selfdepth " << seldepth << std::endl;
  }

  if (ply >= MAX_PLY - 1) {
    return evaluate(pos);
  }

  if (depth <= 0) {
    // same position not child so alpha beta are the same.
    // simply handed to a different function.
    return Search::quiescence(pos, alpha, beta, ply);
  }

  ++nodes;

  const int mate_distance = CHECKMATE - ply;
  if (mate_distance < beta) {
    beta = mate_distance;
    if (alpha >= beta) return beta;
  }

  Move moves[MAX_MOVES];
  const int count = MoveGen::generate_legal_moves(pos, moves);

  rec.m_diff = material_eval(pos);

  // if (ply > 0 && is_draw(pos)) return DRAW;
  if (ply > 0 && is_draw(pos)) return draw_score(rec.m_diff);

  // checkmate or stalemate
  if (count == 0) {
    if (pos.in_check(pos.side_to_move)) {
      return -CHECKMATE + ply;
    }
    return DRAW;
  }

  int best_score = -INF_SCORE;

  const int og_alpha = alpha;

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

      if (score > alpha) {
        alpha = score;
        pv.update(ply, moves[i]);

        if (alpha >= beta) {
          break;
        }
      }
    }
  }
  return best_score;
}