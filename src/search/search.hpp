#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>

#include "../core/move.hpp"
#include "../core/position.hpp"
#include "../evaluation/material.hpp"
#include "constants.hpp"
#include "limits.hpp"
#include "pv.hpp"
#include "stack.hpp"

class Position;

class Search {
 public:
  static constexpr int DEFAULT_DEPTH = 5;
  static constexpr int STACK_SIZE = MAX_PLY + 4;

  Move find_best_move(Position& pos, const SearchLimits& search_limits);

  void reset();

  void stop_search() { stop.store(true, std::memory_order_relaxed); }

  int64_t nodes = 0;
  int best_score = EVAL_NONE;

  std::vector<Move> last_pv;

  int seldepth = 0;

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

  bool is_draw(const Position& pos) const;

  int draw_score(int m_diff) const;

  SearchLimits limits;
  std::atomic<bool> stop{false};
  PlyRecord stack[STACK_SIZE];
  PrincipalVariation pv;
};