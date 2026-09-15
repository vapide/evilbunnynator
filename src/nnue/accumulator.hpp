#pragma once

#include <cstdint>

#include "../core/position_state.hpp"

// lazy accumulator thats one ply etc etc the move which reaches this ply will
// be recorded as a dirty piece (what left the board and what got on the board)
// and the values for the accumulator will be filled in by parent to child in
// nnue.cpp (not implemented yet) move adds 2 pieces at most and will remove two
// at most ex. mover + capture or castling rook ex. rook + king, or promoted
// piece
struct Accumulator {
  static constexpr int L1 = 1024;

  // 64 aligned no padding
  alignas(64) int16_t acc[2][L1];  // [WHITE] / [BLACK] perspectives
  bool computed = false;

  // dirty pieces for the move
  // will 'materialize acc[] from the already computed accumulator'
  DirtyPiece removed[2];
  DirtyPiece added[2];
  int8_t removed_count = 0;
  int8_t added_count = 0;
};
