#pragma once

#include "../core/types.hpp"

namespace Magics {

struct SquareEntry {
  U64 mask;
  U64 magic;
  int shift;
  int offset;
};

// extern because the magics are initialized at runtime
// only the 128 multipliers are considered data
extern SquareEntry ROOK[64];
extern SquareEntry BISHOP[64];
extern U64 ROOK_POOL[102400];
extern U64 BISHOP_POOL[5248];

void init();

inline U64 rook_attacks(int square, U64 occupancy) {
  const SquareEntry& e = ROOK[square];
  return ROOK_POOL[e.offset + (((occupancy & e.mask) * e.magic) >> e.shift)];
}

inline U64 bishop_attacks(int square, U64 occupancy) {
  const SquareEntry& e = BISHOP[square];
  return BISHOP_POOL[e.offset + (((occupancy & e.mask) * e.magic) >> e.shift)];
}

}  // namespace Magics
