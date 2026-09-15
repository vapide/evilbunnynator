#include "movegen.hpp"

#include <cstdlib>

#include "../core/bitboard.hpp"
#include "../core/position.hpp"
#include "attacks.hpp"
#include "magics.hpp"
#include "raytables.hpp"

namespace MoveGen {

U64 attackers_to(const Position& pos, int square, Color color, U64 occupancy) {
  U64 attackers = 0;

  // pawn of Color color attacks a Square square iff a pawn of the opposite
  // color is standing on Square square would attack the pawn's square
  // ex.: white pawn on e4 is attacking black knight on d5, if the knight were a
  // pawn, then the black knight-now-pawn is attacking the e4 pawn
  if (color == WHITE)
    attackers |= Attacks::pawn(square, BLACK) & pos.pieces[WP];
  else
    attackers |= Attacks::pawn(square, WHITE) & pos.pieces[BP];

  attackers |= Attacks::knight(square) & pos.pieces[color * 6 + KNIGHT];

  const U64 diagonal = Attacks::bishop(square, occupancy);
  attackers |= diagonal &
               (pos.pieces[color * 6 + BISHOP] | pos.pieces[color * 6 + QUEEN]);

  const U64 straight = Attacks::rook(square, occupancy);
  attackers |=
      straight & (pos.pieces[color * 6 + ROOK] | pos.pieces[color * 6 + QUEEN]);

  attackers |= Attacks::king(square) & pos.pieces[color * 6 + KING];

  return attackers;
}

U64 attackers_to(const Position& pos, int square, Color color) {
  return attackers_to(pos, square, color, pos.all_occ);
}

// bitboard of all attacked squares
U64 attack_map(const Position& pos, Color color, U64 occupancy) {
  U64 map = 0;
  const int start = color == WHITE ? 0 : 6;

  U64 bb = pos.pieces[start + PAWN];
  while (bb) map |= Attacks::pawn(Bitboard::pop_lsb(bb), color);

  bb = pos.pieces[start + KNIGHT];
  while (bb) map |= Attacks::knight(Bitboard::pop_lsb(bb));

  bb = pos.pieces[start + BISHOP];
  while (bb) map |= Attacks::bishop(Bitboard::pop_lsb(bb), occupancy);

  bb = pos.pieces[start + ROOK];
  while (bb) map |= Attacks::rook(Bitboard::pop_lsb(bb), occupancy);

  bb = pos.pieces[start + QUEEN];
  while (bb) map |= Attacks::queen(Bitboard::pop_lsb(bb), occupancy);

  bb = pos.pieces[start + KING];
  while (bb) map |= Attacks::king(Bitboard::pop_lsb(bb));

  return map;
}

bool is_square_attacked(const Position& pos, int square, Color attacker_color,
                        U64 occ) {
  const int base = attacker_color * 6;

  const Color pawn_lookup = static_cast<Color>(attacker_color ^ 1);
  // const Color pawn_lookup = attacker_color == WHITE ? BLACK : WHITE;

  if (Attacks::knight(square) & pos.pieces[base + KNIGHT]) return true;
  if (Attacks::king(square) & pos.pieces[base + KING]) return true;
  if (Attacks::pawn(square, pawn_lookup) & pos.pieces[base + PAWN]) return true;
  if (Attacks::bishop(square, occ) &
      (pos.pieces[base + BISHOP] | pos.pieces[base + QUEEN]))
    return true;
  if (Attacks::rook(square, occ) &
      (pos.pieces[base + ROOK] | pos.pieces[base + QUEEN]))
    return true;
  return false;
}

bool is_square_attacked(const Position& pos, int square, Color attacker_color) {
  return is_square_attacked(pos, square, attacker_color, pos.all_occ);
}

// TODO: implement
bool gives_check(const Position& pos, Move move) {
  const int from_sq = get_from(move);
  const int to_sq = get_to(move);
  const int promotion = get_promotion(move);
  const int flags = get_flags(move);

  const Piece moving_piece = pos.piece_at(from_sq);

  const Color enemy = pos.side_to_move == WHITE ? BLACK : WHITE;
  const int enemy_king_sq = pos.king_square(enemy);

  U64 occ = pos.all_occ;
  occ &= ~(1ULL << from_sq);

  if (flags & CAPTURE) {
    int captured_sq = to_sq;
    if (flags & EN_PASSANT)
      captured_sq = pos.side_to_move == WHITE ? to_sq - 8 : to_sq + 8;
    occ &= ~(1ULL << captured_sq);
  }
  occ |= 1ULL << to_sq;

  const int piece_type = promotion ? promotion : moving_piece % 6;
  const U64 king_bb = 1ULL << enemy_king_sq;

  bool direct = false;
  switch (piece_type) {
    case PAWN:
      direct = (Attacks::pawn(to_sq, pos.side_to_move) & king_bb) != 0;
      break;
    case KNIGHT:
      direct = (Attacks::knight(to_sq) & king_bb) != 0;
      break;
    case BISHOP:
      direct = (Attacks::bishop(to_sq, occ) & king_bb) != 0;
      break;
    case ROOK:
      direct = (Attacks::rook(to_sq, occ) & king_bb) != 0;
      break;
    case QUEEN:
      direct = (Attacks::queen(to_sq, occ) & king_bb) != 0;
      break;
    case KING:
      direct = (Attacks::king(to_sq) & king_bb) != 0;
      break;
  }
  if (direct) return true;

  // check discovered checks
  return false;
}

namespace {

int generate_king_moves(const Position& pos, const KingSafety& safety,
                        Move* moves, bool captures_only = false) {
  int count = 0;
  const int king_sq = safety.king;
  const Color color = pos.side_to_move;
  const Color enemy = color == WHITE ? BLACK : WHITE;

  const U64 friendly_occ = color == WHITE ? pos.white_occ : pos.black_occ;
  const U64 enemy_occ = color == WHITE ? pos.black_occ : pos.white_occ;

  U64 attacks = Attacks::king(king_sq) & ~friendly_occ;
  if (captures_only) attacks &= enemy_occ;

  // destination is legal iff enemy doesn't attack it once the king has left his
  // square only a slider ray can pass through old square which means it sees
  // the king which means we're in check so king removed occ changes when in
  // check. Testing each square is a better idea when compared to full enemy
  // attack maps
  const U64 occ =
      safety.checkers ? pos.all_occ & ~(1ULL << king_sq) : pos.all_occ;

  while (attacks) {
    const int to_sq = Bitboard::pop_lsb(attacks);
    if (is_square_attacked(pos, to_sq, enemy, occ)) continue;
    const int flags = (enemy_occ & (1ULL << to_sq)) ? CAPTURE : 0;
    moves[count++] = encode_move(king_sq, to_sq, flags);
  }
  return count;
}

int generate_castling_moves(const Position& pos, Move* moves) {
  int count = 0;

  if (pos.side_to_move == WHITE) {
    if ((pos.castling_rights & WK_CASTLE) && pos.piece_at(H1) == WR) {
      if (!(pos.all_occ & ((1ULL << F1) | (1ULL << G1))) &&
          !is_square_attacked(pos, E1, BLACK) &&
          !is_square_attacked(pos, F1, BLACK) &&
          !is_square_attacked(pos, G1, BLACK))
        moves[count++] = encode_move(E1, G1, CASTLE);
    }
    if ((pos.castling_rights & WQ_CASTLE) && pos.piece_at(A1) == WR) {
      if (!(pos.all_occ & ((1ULL << B1) | (1ULL << C1) | (1ULL << D1))) &&
          !is_square_attacked(pos, E1, BLACK) &&
          !is_square_attacked(pos, D1, BLACK) &&
          !is_square_attacked(pos, C1, BLACK))
        moves[count++] = encode_move(E1, C1, CASTLE);
    }
  } else {
    if ((pos.castling_rights & BK_CASTLE) && pos.piece_at(H8) == BR) {
      if (!(pos.all_occ & ((1ULL << F8) | (1ULL << G8))) &&
          !is_square_attacked(pos, E8, WHITE) &&
          !is_square_attacked(pos, F8, WHITE) &&
          !is_square_attacked(pos, G8, WHITE))
        moves[count++] = encode_move(E8, G8, CASTLE);
    }
    if ((pos.castling_rights & BQ_CASTLE) && pos.piece_at(A8) == BR) {
      if (!(pos.all_occ & ((1ULL << B8) | (1ULL << C8) | (1ULL << D8))) &&
          !is_square_attacked(pos, E8, WHITE) &&
          !is_square_attacked(pos, D8, WHITE) &&
          !is_square_attacked(pos, C8, WHITE))
        moves[count++] = encode_move(E8, C8, CASTLE);
    }
  }
  return count;
}

int add_pawn_promotions(Move* moves, int count, int from_sq, int to_sq,
                        int flags) {
  moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_QUEEN);
  moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_ROOK);
  moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_BISHOP);
  moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_KNIGHT);
  return count;
}

// not safe or ready-doesn't check if a move is legal. legality should be
// checked in the generator here. for en passant, make and unmake move to check
// for checks.
int generate_non_king_moves(Position& pos, const KingSafety& safety,
                            Move* moves, bool captures_only = false) {
  int count = 0;
  const Color side = pos.side_to_move;
  const U64 occ = pos.all_occ;
  const U64 own_occ = side == WHITE ? pos.white_occ : pos.black_occ;
  const U64 enemy_occ = side == WHITE ? pos.black_occ : pos.white_occ;
  const U64 enemy_king_bb = pos.pieces[side == WHITE ? BK : WK];
  const U64 capture_targets = enemy_occ & ~enemy_king_bb;
  const int base = side * 6;

  // pawn code from pseudo legal
  U64 pawns = pos.pieces[base + PAWN];
  while (pawns) {
    const int from_sq = Bitboard::pop_lsb(pawns);

    // pinned pawn
    U64 allowed = safety.evasion;
    if (safety.pinned & (1ULL << from_sq)) allowed &= safety.pin_masks[from_sq];

    if (side == WHITE) {
      // int not bitboard, so just add 8 (reference core/types.hpp for square
      // enum for visual)
      const int one_step = from_sq + 8;
      const int two_step = from_sq + 16;

      if (one_step < 64 && !(occ & (1ULL << one_step))) {
        if (one_step / 8 == 7) {  // destination not current position (63/8 = 7)
          if (allowed & (1ULL << one_step))
            count = add_pawn_promotions(moves, count, from_sq, one_step, 0);
        } else if (!captures_only) {
          if (allowed & (1ULL << one_step))
            moves[count++] = encode_move(from_sq, one_step);
          if (from_sq / 8 == 1 && !(occ & (1ULL << two_step)) &&
              (allowed & (1ULL << two_step)))
            moves[count++] = encode_move(from_sq, two_step, DOUBLE_PAWN_PUSH);
        }
      }
    } else {  // Black's Pawn
      const int one_step = from_sq - 8;
      const int two_step = from_sq - 16;

      if (one_step >= 0 && !(occ & (1ULL << one_step))) {
        if (one_step / 8 == 0) {
          if (allowed & (1ULL << one_step))
            count = add_pawn_promotions(moves, count, from_sq, one_step, 0);
        } else if (!captures_only) {
          if (allowed & (1ULL << one_step))
            moves[count++] = encode_move(from_sq, one_step);
          if (from_sq / 8 == 6 && !(occ & (1ULL << two_step)) &&
              (allowed & (1ULL << two_step)))
            moves[count++] = encode_move(from_sq, two_step, DOUBLE_PAWN_PUSH);
        }
      }
    }

    U64 cap_targets = Attacks::pawn(from_sq, side) & enemy_occ & allowed;
    while (cap_targets) {
      const int to_sq = Bitboard::pop_lsb(cap_targets);
      const bool is_promo_rank =
          side == WHITE ? to_sq / 8 == 7 : to_sq / 8 == 0;
      if (is_promo_rank) {
        count = add_pawn_promotions(moves, count, from_sq, to_sq, CAPTURE);
      } else {
        moves[count++] = encode_move(from_sq, to_sq, CAPTURE);
      }
    }
    // en-passant
    if (pos.ep_square != -1) {
      const int ep_target = pos.ep_square;
      if (Attacks::pawn(from_sq, side) & (1ULL << ep_target)) {
        const int capture_sq = side == WHITE ? ep_target - 8 : ep_target + 8;
        const Piece expected = side == WHITE ? BP : WP;
        if (pos.piece_at(capture_sq) == expected) {
          // make and unmake to check if its pinned (easier this way)
          const Move candidate =
              encode_move(from_sq, ep_target, CAPTURE | EN_PASSANT);
          pos.make_move(candidate);
          const bool leaves_king_in_check = pos.in_check(side);
          pos.unmake_move();
          if (!leaves_king_in_check) moves[count++] = candidate;
        }
      }
    }
  }
  // other pieces, for loop differs because this is non king moves
  for (int piece_type = KNIGHT; piece_type <= QUEEN; ++piece_type) {
    U64 bb = pos.pieces[base + piece_type];
    while (bb) {
      const int from_sq = Bitboard::pop_lsb(bb);

      U64 targets;
      switch (piece_type) {
        case KNIGHT:
          targets = Attacks::knight(from_sq);
          break;
        case BISHOP:
          targets = Attacks::bishop(from_sq, occ);
          break;
        case ROOK:
          targets = Attacks::rook(from_sq, occ);
          break;
        default:
          targets = Attacks::queen(from_sq, occ);
          break;
      }
      targets &= ~(own_occ | enemy_king_bb);
      if (captures_only) targets &= enemy_occ;

      if (safety.pinned & (1ULL << from_sq))
        targets &= safety.pin_masks[from_sq];
      targets &= safety.evasion;

      while (targets) {
        const int to_sq = Bitboard::pop_lsb(targets);
        const int flags = (enemy_occ & (1ULL << to_sq)) ? CAPTURE : 0;
        moves[count++] = encode_move(from_sq, to_sq, flags);
      }
    }
  }

  return count;
}

}  // namespace

int generate_legal_moves(Position& pos, const KingSafety& safety, Move* moves) {
  const int checker_count = Bitboard::popcount(safety.checkers);

  int count = generate_king_moves(pos, safety, moves);

  if (checker_count > 1) return count;  // double check: king moves only

  if (checker_count == 0) count += generate_castling_moves(pos, moves + count);

  count += generate_non_king_moves(pos, safety, moves + count);

  return count;
}

int generate_legal_moves(Position& pos, Move* moves) {
  return generate_legal_moves(pos, analyze_king_safety(pos), moves);
}

int generate_legal_captures(Position& pos, const KingSafety& safety,
                            Move* moves) {
  const int checker_count = Bitboard::popcount(safety.checkers);

  // captures_only = true
  int count = generate_king_moves(pos, safety, moves, true);

  if (checker_count > 1) return count;  // double check: king moves only

  // captures_only = true
  count += generate_non_king_moves(pos, safety, moves + count, true);

  return count;
}

int generate_legal_captures(Position& pos, Move* moves) {
  return generate_legal_captures(pos, analyze_king_safety(pos), moves);
}

int generate_pseudo_legal_moves(const Position& pos, Move* moves) {
  int count = 0;
  const Color side = pos.side_to_move;
  const U64 occ = pos.all_occ;
  const U64 own_occ = side == WHITE ? pos.white_occ : pos.black_occ;
  const U64 enemy_occ = side == WHITE ? pos.black_occ : pos.white_occ;
  const int base = side * 6;

  U64 pawns = pos.pieces[base + PAWN];
  while (pawns) {
    const int from_sq = Bitboard::pop_lsb(pawns);

    if (side == WHITE) {
      // int not bitboard, so just add 8 (reference core/types.hpp for square
      // enum for visual)
      const int one_step = from_sq + 8;
      const int two_step = from_sq + 16;

      if (one_step < 64 && !(occ & (1ULL << one_step))) {
        if (from_sq / 8 == 7) {
          count = add_pawn_promotions(moves, count, from_sq, one_step, 0);
        } else {
          moves[count++] = encode_move(from_sq, one_step);
          if (from_sq / 8 == 1 && !(occ & (1ULL << two_step))) {
            moves[count++] = encode_move(from_sq, two_step);
          }
        }
      }
    } else {  // Black's Pawn
      const int one_step = from_sq - 8;
      const int two_step = from_sq - 16;

      if (one_step >= 0 && !(occ & (1ULL << one_step))) {
        if (from_sq / 8 == 1) {
          count = add_pawn_promotions(moves, count, from_sq, one_step, 0);
        } else {
          moves[count++] = encode_move(from_sq, one_step);
          if (from_sq / 8 == 7 && !(occ & (1ULL << two_step))) {
            moves[count++] = encode_move(from_sq, two_step);
          }
        }
      }
    }

    U64 cap_targets = Attacks::pawn(from_sq, side) & enemy_occ;
    while (cap_targets) {
      const int to_sq = Bitboard::pop_lsb(cap_targets);
      const bool is_promo_rank =
          side == WHITE ? to_sq / 8 == 7 : to_sq / 8 == 0;
      if (is_promo_rank) {
        count = add_pawn_promotions(moves, count, from_sq, to_sq, CAPTURE);
      } else {
        moves[count++] = encode_move(from_sq, to_sq, CAPTURE);
      }
    }
    // en-passant
    if (pos.ep_square != -1) {
      const int ep_target = pos.ep_square;
      if (Attacks::pawn(from_sq, side) & (1ULL << ep_target)) {
        const int capture_sq = side == WHITE ? ep_target - 8 : ep_target + 8;
        const Piece expected = side == WHITE ? BP : WP;
        if (pos.piece_at(capture_sq) == expected) {
          moves[count++] =
              encode_move(from_sq, ep_target, CAPTURE | EN_PASSANT);
        }
      }
    }
  }

  for (int piece_type = KNIGHT; piece_type <= KING; ++piece_type) {
    U64 bb = pos.pieces[base + piece_type];
    while (bb) {
      const int from_sq = Bitboard::pop_lsb(bb);

      U64 targets;
      switch (piece_type) {
        case KNIGHT:
          targets = Attacks::knight(from_sq);
          break;
        case BISHOP:
          targets = Attacks::bishop(from_sq, occ);
          break;
        case ROOK:
          targets = Attacks::rook(from_sq, occ);
          break;
        case QUEEN:
          targets = Attacks::queen(from_sq, occ);
          break;
        default:
          targets = Attacks::king(from_sq);
          break;
      }
      targets &= ~own_occ;

      if (piece_type == KING) {
        // pseudo-legal castling (occupancy + rook checks only)
        if (side == WHITE && from_sq == E1) {
          if ((pos.castling_rights & WK_CASTLE) &&
              !(occ & ((1ULL << F1) | (1ULL << G1))) &&
              pos.piece_at(H1) == WR) {
            moves[count++] = encode_move(E1, G1, CASTLE);
            targets &= ~(1ULL << G1);
          }
          if ((pos.castling_rights & WQ_CASTLE) &&
              !(occ & ((1ULL << D1) | (1ULL << C1) | (1ULL << B1))) &&
              pos.piece_at(A1) == WR) {
            moves[count++] = encode_move(E1, C1, CASTLE);
            targets &= ~(1ULL << C1);
          }
        } else if (side == BLACK && from_sq == E8) {
          if ((pos.castling_rights & BQ_CASTLE) &&
              !(occ & ((1ULL << D8) | (1ULL << C8) | (1ULL << B8))) &&
              pos.piece_at(A8) == BR) {
            moves[count++] = encode_move(E8, C8, CASTLE);
            targets &= ~(1ULL << C8);
          }
          if ((pos.castling_rights & BK_CASTLE) &&
              !(occ & ((1ULL << F8) | (1ULL << G8))) &&
              pos.piece_at(H8) == BR) {
            moves[count++] = encode_move(E8, G8, CASTLE);
            targets &= ~(1ULL << G8);
          }
        }
      }

      while (targets) {
        const int to_sq = Bitboard::pop_lsb(targets);
        const int flags = (enemy_occ & (1ULL << to_sq)) ? CAPTURE : 0;
        moves[count++] = encode_move(from_sq, to_sq, flags);
      }
    }

    /*
    U64 knights = pos.pieces[base * side + 1];
    while (knights) {
        const int from_sq = Bitboard::pop_lsb(knights);
        const int knight_moves = Attacks::knight(from_sq) & own_occ;

        while (knight_moves) {
            moves[count++] = Bitboard::pop_lsb(knights);
        }
    } */
  }
  return count;
}
}  // namespace MoveGen

// methods in Position that need the movegen methods

bool Position::in_check(Color color) const {
  return MoveGen::is_square_attacked(*this, king_square(color),
                                     color == WHITE ? BLACK : WHITE);
}

bool Position::is_checkmate(Color color) {
  if (!in_check(color)) return false;
  Move moves[MAX_MOVES];
  return MoveGen::generate_legal_moves(*this, moves) == 0;
}

bool Position::is_game_over() {
  Move moves[MAX_MOVES];
  return MoveGen::generate_legal_moves(*this, moves) == 0;
}

void init_all() {
  Zobrist::init();
  Magics::init();
  Attacks::init();
  Ray::init();
}
