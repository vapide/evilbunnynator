#pragma once

#include "../core/bitboard.hpp"
#include "../core/types.hpp"
#include "magics.hpp"

// pseudo-legal
namespace Attacks {

extern U64 KNIGHT_ATTACKS[64];
extern U64 KING_ATTACKS[64];
extern U64 PAWN_ATTACKS[2][64];   // [color][square]

void init();

inline U64 knight(int square) { return KNIGHT_ATTACKS[square]; }
inline U64 king(int square)   { return KING_ATTACKS[square]; }
inline U64 pawn(int square, Color color) { return PAWN_ATTACKS[color][square]; }

// TODO: sliding attacks, can do ray attacks before magic bitboards/numbers

}
