#include "material.hpp"

#include "../core/bitboard.hpp"
#include "../core/position.hpp"

// side to move perspective, just for now.
int material_eval(const Position& pos) {
  int material_popcount;
  int material_count = 0;
  int stm = pos.side_to_move;
  for (int i = PAWN; i <= QUEEN; ++i) {
    // TODO: can move ? : to negation in return statement later
    material_popcount = (stm == WHITE ? (Bitboard::popcount(pos.pieces[i]) -
                                         Bitboard::popcount(pos.pieces[i + 6]))
                                      : (Bitboard::popcount(pos.pieces[i + 6]) -
                                         Bitboard::popcount(pos.pieces[i])));
    // difference in piece counts x material value e.g. distributive property
    material_count += material_popcount * MATERIAL_VALUES[i];
  }
  return material_popcount;
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

/*
    // chose to do stm == BLACK instead of base == 6 to not hard code values
    material_popcount =
        Bitboard::popcount(pos.pieces[i + base]) -
        Bitboard::popcount(stm == BLACK ? pos.pieces[i] : pos.pieces[i - base]);
*/