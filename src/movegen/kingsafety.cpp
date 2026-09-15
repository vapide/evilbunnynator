#include "kingsafety.hpp"

#include "../core/bitboard.hpp"
#include "../core/position.hpp"
#include "magics.hpp"
#include "movegen.hpp"
#include "raytables.hpp"

namespace MoveGen {

namespace {

// modifies kingsafety obj to get pins pins iff one piece stands between the
// pinner and king same as walking 8 rays square-by-square, but more efficient
// (2 lookups for magic + one short loop over snipers)
void find_pins(const Position& pos, Color color, int king_sq, KingSafety& out) {
  out.pinned = 0;

  const int ebase = (color ^ 1) * 6;
  const U64 occ = pos.all_occ;
  const U64 own = color == WHITE ? pos.white_occ : pos.black_occ;

  // uses ray from king square to check if there are any pieces checking the
  // king splits queen into rook and bishop to avoid third and--not sure how
  // efficient that was
  U64 snipers = (Magics::rook_attacks(king_sq, 0) &
                 (pos.pieces[ebase + ROOK] | pos.pieces[ebase + QUEEN])) |
                (Magics::bishop_attacks(king_sq, 0) &
                 (pos.pieces[ebase + BISHOP] | pos.pieces[ebase + QUEEN]));

  // loops through snipers like in pseudolegalmovegen
  while (snipers) {
    const int sniper_sq = Bitboard::pop_lsb(snipers);
    const U64 between = Ray::BETWEEN[king_sq][sniper_sq] & occ;
    // exactly one blocker--one of our own. this only matters because we're
    // generating legal moves, not searching
    if (between && !(between & (between - 1)) && (between & own)) {
      const int blocker = Bitboard::lsb(between);
      out.pinned |= 1ULL << blocker;
      out.pin_masks[blocker] = Ray::LINE[king_sq][sniper_sq];
    }
  }
}

}  // namespace

// if known_checkers aren't given.
KingSafety analyze_king_safety(const Position& pos) {
  const int king_sq = pos.king_square(pos.side_to_move);
  return analyze_king_safety(
      pos,
      attackers_to(pos, king_sq, static_cast<Color>(pos.side_to_move ^ 1)));
  // attackers_to not yet implemented
}

KingSafety analyze_king_safety(const Position& pos, U64 known_checkers) {
  KingSafety safety;

  const Color color = pos.side_to_move;
  const int king_sq = pos.king_square(color);

  safety.king = king_sq;
  safety.checkers = known_checkers;

  find_pins(pos, color, king_sq, safety);

  const int checker_count = Bitboard::popcount(safety.checkers);
  if (checker_count == 1) {
    const int checker_sq = Bitboard::lsb(safety.checkers);
    safety.evasion = Ray::BETWEEN[king_sq][checker_sq] | (1ULL << checker_sq);
  } else if (checker_count > 1) {
    safety.evasion = 0;  // double check: only king moves
  } else {
    safety.evasion = ~0ULL;
  }

  return safety;
}

}  // namespace MoveGen