#pragma once

#include <algorithm>
#include <atomic>
#include <cstdint>

#include "../core/move.hpp"
#include "../core/position.hpp"
#include "../evaluation/material.hpp"
#include "constants.hpp"
#include "limits.hpp"

class Position;

class Search {
 public:
  static constexpr int DEFAULT_DEPTH = 5;

  Move find_best_move(Position& pos, const SearchLimits& search_limits);

  void reset();

  void stop_search() { stop.store(true, std::memory_order_relaxed); }

  int64_t nodes = 0;
  int best_score = EVAL_NONE;

  inline int evaluate(const Position& pos) {
    int CLAMP_LIMIT = CHECKMATE_THRESHOLD - 1;
    int material_score = material_eval(pos);

    return std::clamp(material_score, -CLAMP_LIMIT, CLAMP_LIMIT);
  }

 private:
  void clear_for_search();

  int negamax(Position& pos, int depth, int alpha, int beta, int ply);

  // will hold accumulator modifiers later
  void do_move(Position& pos, Move move) { pos.make_move(move); }
  void undo_move(Position& pos) { pos.unmake_move(); }

  SearchLimits limits;
  std::atomic<bool> stop{false};
};