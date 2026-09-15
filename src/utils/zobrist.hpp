#pragma once

#include <array>
#include <cstdint>

#include "../core/types.hpp"

class Position;

// 781 keys total, all static, and generated once at STARTUP
// zobrist::init()
// key layout
// [piece * 64 + square] for piece 0-11
// [768] side to move
// [769-772] castling rights (one key per right bit)
// [773-780] en passant file
class Zobrist {
 public:
  static constexpr int SIDE_KEY = 768;
  static constexpr int CASTLING_START = 769;
  static constexpr int EP_START = 773;
  static constexpr int NUM_KEYS = 781;

  static void init(uint64_t seed = 67676767ULL);

  static U64 key_at(int i) { return keys[i]; }

  void toggle_piece(Piece piece, Square sq) {
    U64 key = keys[piece * 64 + sq];
    hash ^= key;

    if (piece % 6 == PAWN) {
      pawn_hash ^= key;
    } else {
      Color color = piece < 6 ? WHITE : BLACK;
      non_pawn_hash[color] ^= key;
    }
  }

  void toggle_castling(int right_index) {
    hash ^= keys[CASTLING_START + right_index];
  }

  void toggle_ep(Square square) {
    if (square != SQ_NONE) hash ^= keys[EP_START + square % 8];
  }

  void flip_side() { hash ^= keys[SIDE_KEY]; }

  // full recompute from the board, this also refreshes pawn/non-pawn hashes
  U64 compute(const Position& position);

  U64 hash = 0;
  U64 pawn_hash = 0;
  std::array<U64, 2> non_pawn_hash{0, 0};

 private:
  static std::array<U64, NUM_KEYS> keys;
};
