#include "../core/bitboard.hpp"
#include "../evaluation/material.hpp"
#include "search.hpp"

// side to move perspective, just for now.
int material_eval(const Position& pos) {
  int material_popcount;
  int material_count = 0;
  int stm = pos.side_to_move;
  for (int i = PAWN; i <= QUEEN; ++i) {
    // whites perspective
    material_popcount = Bitboard::popcount(pos.pieces[i]) -
                        Bitboard::popcount(pos.pieces[i + 6]);
    // difference in piece counts x material value e.g. distributive property
    material_count += material_popcount * MATERIAL_VALUES[i];
  }
  return stm == WHITE ? material_count : -material_count;
}

// total non king material on the board from both sides.
int total_material(const Position& pos) {
  int total = 0;
  for (int i = PAWN; i <= QUEEN; ++i) {
    total += MATERIAL_VALUES[i] * (Bitboard::popcount(pos.pieces[i]) +
                                   Bitboard::popcount(pos.pieces[i + 6]));
  }
  return total;
}

bool Search::is_draw(const Position& pos) const {
  // ordered by cost
  return pos.halfmove_clock >= DRAW_HALFMOVE_LIMIT ||
         pos.is_insufficient_material() || pos.is_repetition();
}

int Search::draw_score(int m_diff) const {
  int score = 1 - static_cast<int>(nodes & 2);
  if (m_diff < -DRAW_MDIFF_BIAS) {
    score += DRAW_BIAS_SCORE;
  } else if (m_diff > DRAW_MDIFF_BIAS) {
    score -= DRAW_BIAS_SCORE;
  }
  return score;
}

// test nps here
/*
  if (pos.halfmove_clock >= DRAW_HALFMOVE_LIMIT) {
return true;
} else if (pos.is_insufficient_material()) {
return true;
} else if (pos.is_repetition()) {
return true;
}*/