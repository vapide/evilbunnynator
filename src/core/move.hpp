#pragma once

#include <string>

#include "types.hpp"

// same as python:
// bits 0-5   from square
// bits 6-11  to square
// bits 12-15 flags (MoveFlag bitmask)
// bits 16-19 promotion (PromoFlag)
using Move = uint32_t;

constexpr Move MOVE_NONE = 0;

constexpr Move encode_move(int from_sq, int to_sq, int flags = 0,
                           int promotion = 0) {
  return static_cast<Move>(from_sq | (to_sq << 6) | (flags << 12) |
                           (promotion << 16));
}

constexpr Square get_from(Move m) { return static_cast<Square>(m & 0x3F); }
constexpr Square get_to(Move m) { return static_cast<Square>((m >> 6) & 0x3F); }
constexpr int get_flags(Move m) { return (m >> 12) & 0xF; }
constexpr int get_promotion(Move m) { return (m >> 16) & 0xF; }

// 16 bit move from(6) | to(6) | promo(4)
// flag parts are dropped, transposition table moves are used after matching
// against the legal list which makes a hash collision harmless, a foreign
// move16 simply matches nothing
using Move16 = uint16_t;
constexpr Move16 MOVE16_NONE = 0;

constexpr Move16 to_move16(Move m) {
  return static_cast<Move16>((m & 0xFFF) | (get_promotion(m) << 12));
}

// string conversions defined in move.cpp
std::string square_name(int square);
int square_index(const std::string& name);  // -1 on invalid input
Move from_uci(const std::string& uci);      // MOVE_NONE on invalid input
std::string to_uci(Move m);
