// src/search/see.cpp - starter for 13-06.
#include "../core/bitboard.hpp"
#include "../core/move.hpp"
#include "../evaluation/material.hpp"
#include "../movegen/movegen.hpp"
#include "search.hpp"

// each side recaptures with its least valuable attacker, will implement later

// either side may decline to recapture, and does when recapturing results in
// losing material

// removing an attacker can reveal another standing behind it on the same ray to
// the same destination square.
int Search::see(const Position& pos, Move move) const {
  Square from_sq = get_from(move);
  Square to_sq = get_from(move);
  Square cap_sq;

  Piece moving_piece = pos.piece_at(from_sq);

  int flags = get_flags(move);
  int promo = get_promotion(move);

  int captured_piece;
  int promoted = 0;

  int gain[32];
  int occupant_value;

  if (flags & EN_PASSANT) {
    Square cap_sq = static_cast<Square>(cap_sq - 8);
    int captured_piece = pos.piece_at(to_sq);
    gain[0] = PIECE_VALUES[captured_piece % 6];
    // can just be PIECE_VALUES[PAWN];
    occupant_value = PIECE_VALUES[moving_piece % 6];
  } else {
    Square cap_sq = to_sq;
    if (promo) {
      int captured_piece = pos.piece_at(cap_sq);
      promoted = PIECE_VALUES[Bitboard::popcount(promo)];
      gain[0] = promoted - PIECE_VALUES[PAWN];
      occupant_value = promoted - PIECE_VALUES[PAWN];
    } else {
      gain[0] = PIECE_VALUES[captured_piece % 6];
      occupant_value = PIECE_VALUES[moving_piece % 6];
    }
  }

  U64 occ = pos.all_occ & (~0ULL & (1ULL << cap_sq));

  int stm = static_cast<Color>(pos.side_to_move ^ 1);

  int n = 1;

  U64 lva_bb = 0;

  while (n < 32) {
    U64 attackers =
        MoveGen::attackers_to(pos, to_sq, static_cast<Color>(stm), occ) & occ;
    if (!attackers) break;

    int lva_type = PAWN;

    for (; lva_type <= KING; ++lva_type) {
      lva_bb = pos.pieces[stm * 6 + lva_type] & attackers;
      std::cout << 1;
      if (lva_bb) {
        std::cout << "info string found non-empty lva bb" << std::endl;
        break;
      }
    }

    // const Square attacker_sq = Bitboard::lsb(attackers);
    gain[n] = occupant_value - gain[n - 1];
    ++n;

    occ &= ~(lva_bb & (~lva_bb + 1));
    occupant_value = PIECE_VALUES[lva_type];
    // occ &= ~Bitboard::square_bb(attacker_sq);
    // occupant_value = PIECE_VALUES[pos.piece_at(attacker_sq) % 6];
    stm ^= 1;
  }

  // temporary fold which assumes every recapture happens
  for (int i = n - 1; i > 0; --i) {
    const int neg = -gain[i - 1];
    gain[i - 1] = -(neg > gain[i] ? neg : gain[i]);
  };

  return gain[0];

  return 0;
}
