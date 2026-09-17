#include "position.hpp"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "bitboard.hpp"
#include "position.hpp"

// requires the following methods for position:
//

Position::Position() {
  for (int sq = 0; sq < 64; ++sq) mailbox[sq] = NO_PIECE;
}

Position Position::from_fen(const std::string& fen) {
  Position position;
  std::istringstream ss(fen);
  std::string placement, stm, castling, ep, halfmove = "0", fullmove = "1";
  ss >> placement >> stm >> castling >> ep >> halfmove >> fullmove;

  auto piece_from_char = [](char c) -> Piece {
    switch (c) {
      case 'P':
        return WP;
      case 'N':
        return WN;
      case 'B':
        return WB;
      case 'R':
        return WR;
      case 'Q':
        return WQ;
      case 'K':
        return WK;
      case 'p':
        return BP;
      case 'n':
        return BN;
      case 'b':
        return BB;
      case 'r':
        return BR;
      case 'q':
        return BQ;
      case 'k':
        return BK;
    }
    return NO_PIECE;
  };

  int rank = 7, file = 0;
  for (char c : placement) {
    if (c == '/') {
      --rank;
      file = 0;
    } else if (c >= '1' && c <= '8') {
      file += c - '0';
    } else {
      position.add_piece(piece_from_char(c), rank * 8 + file);
      ++file;
    }
  }

  position.side_to_move = (stm == "w") ? WHITE : BLACK;

  position.castling_rights = 0;
  if (castling.find('K') != std::string::npos)
    position.castling_rights |= WK_CASTLE;
  if (castling.find('Q') != std::string::npos)
    position.castling_rights |= WQ_CASTLE;
  if (castling.find('k') != std::string::npos)
    position.castling_rights |= BK_CASTLE;
  if (castling.find('q') != std::string::npos)
    position.castling_rights |= BQ_CASTLE;

  if (ep == "-") {
    position.ep_square = -1;
  } else {
    int ep_file = ep[0] - 'a';
    int ep_rank = ep[1] - '1';
    position.ep_square = ep_rank * 8 + ep_file;
  }

  position.halfmove_clock = std::atoi(halfmove.c_str());
  position.fullmove_number = std::atoi(fullmove.c_str());

  position.update_occupancy();
  // ok to recompute since we're building the board from scratch
  position.zobrist.hash = position.zobrist.compute(position);

  return position;
}

Position Position::startpos() {
  return from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

std::string Position::to_fen() const {
  std::string fen;
  // top to bottom
  for (int rank = 7; rank >= 0; --rank) {
    int empty = 0;
    for (int file = 0; file <= 7; ++file) {
      Piece p = piece_at(rank * 8 + file);
      if (p == NO_PIECE) {
        ++empty;
      } else {
        if (empty > 0) {
          fen += char('0' + empty);
          empty = 0;
          fen += (p < 6 ? "PNBRQK" : "pnbrqk")[p % 6];
        }
      }
    }
    if (empty > 0) {
      fen += char('0' + empty);
      // std::cout << "empty char: " << empty << std::endl;
    }
    if (rank > 0) {
      fen += '/';
    }
  }

  // stm, castling, en passant, halfmove, fullmove
  fen += ' ';
  fen += ((side_to_move == WHITE) ? 'w' : 'b');

  std::string castlingrights;
  for (int i = 0; i < 4; ++i) {
    if (1 << i & castling_rights) {
      castlingrights += "KQkq"[i];
    }
  }

  fen += ' ';

  fen += castlingrights.empty() ? "-" : castlingrights;

  fen += ' ';
  fen += ((ep_square != -1) ? square_name(ep_square) : "-");

  fen += ' ';
  fen += std::to_string(halfmove_clock);
  fen += ' ';
  fen += std::to_string(fullmove_number);

  return fen;
}

void Position::update_occupancy() {
  white_occ = 0;
  black_occ = 0;
  for (int piece = 0; piece < 6; ++piece) white_occ |= pieces[piece];
  for (int piece = 6; piece < 12; ++piece) black_occ |= pieces[piece];
  all_occ = white_occ | black_occ;
}

void Position::add_piece(Piece piece, int square) {
  U64 bb = 1ULL << square;
  pieces[piece] |= bb;
  mailbox[square] = piece;
  if (piece < 6)
    white_occ |= bb;
  else
    black_occ |= bb;
  all_occ |= bb;
}

void Position::remove_piece(Piece piece, int square) {
  U64 bb = 1ULL << square;
  pieces[piece] &= ~bb;
  mailbox[square] = NO_PIECE;
  if (piece < 6)
    white_occ &= ~bb;
  else
    black_occ &= ~bb;
  all_occ &= ~bb;
}

void Position::move_piece(Piece piece, int from_sq, int to_sq) {
  assert(mailbox[from_sq] == piece);
  assert(mailbox[to_sq] == NO_PIECE);

  U64 from_mask = ~(1ULL << from_sq);
  U64 to_bb = 1ULL << to_sq;
  pieces[piece] &= from_mask;
  pieces[piece] |= to_bb;
  mailbox[from_sq] = NO_PIECE;
  mailbox[to_sq] = piece;
  if (piece < 6) {
    white_occ &= from_mask;
    white_occ |= to_bb;
  } else {
    black_occ &= from_mask;
    black_occ |= to_bb;
  }
  all_occ &= from_mask;
  all_occ |= to_bb;
}

Square Position::king_square(Color color) const {
  return Bitboard::lsb(pieces[color == WHITE ? WK : BK]);
}

bool Position::has_non_pawn_material(Color color) const {
  // king and pawns don't count - check knights, bishops, rooks, queens
  if (color == WHITE)
    return (pieces[WN] | pieces[WB] | pieces[WR] | pieces[WQ]) != 0;
  return (pieces[BN] | pieces[BB] | pieces[BR] | pieces[BQ]) != 0;
}

// basically ported from position.py, with minor changes due to the nature of
// C++ and optimizations.
void Position::make_move(Move move) {
  assert(ply < MAX_HISTORY);
  PositionState& state = history[ply];
  state.hash = zobrist.hash;
  hash_ring[ply] = zobrist.hash;
  state.pawn_hash = zobrist.pawn_hash;
  state.non_pawn_hash_white = zobrist.non_pawn_hash[WHITE];
  state.non_pawn_hash_black = zobrist.non_pawn_hash[BLACK];

  const int from_sq = get_from(move);
  const int to_sq = get_to(move);
  const int flags = get_flags(move);
  const Piece moving_piece = piece_at(from_sq);
  assert(moving_piece != NO_PIECE);

  int captured_square = to_sq;
  Piece captured_piece = piece_at(to_sq);

  if (flags & EN_PASSANT) {
    captured_square = (moving_piece == WP) ? to_sq - 8 : to_sq + 8;
    captured_piece = piece_at(captured_square);
  }

  state.removed_count = 0;
  state.added_count = 0;

  state.captured_piece = captured_piece;
  state.captured_square = (captured_piece != NO_PIECE) ? captured_square : -1;

  state.move = move;
  state.moved_piece = moving_piece;
  state.castling_rights = castling_rights;
  state.ep_square = ep_square;
  state.halfmove_clock = halfmove_clock;
  state.fullmove_number = fullmove_number;
  state.promotion_piece = NO_PIECE;
  state.flags = flags;

  state.removed[state.removed_count++] = {moving_piece,
                                          static_cast<Square>(from_sq)};

  if (ep_square != -1) zobrist.toggle_ep(static_cast<Square>(ep_square));
  ep_square = -1;

  const int old_castling = castling_rights;
  castling_rights &= CASTLING_MASKS[from_sq];
  if (state.captured_square != -1)
    castling_rights &= CASTLING_MASKS[state.captured_square];

  const int changed_castling = old_castling ^ castling_rights;
  for (int i = 0; i < 4; ++i)
    if (changed_castling & (1 << i)) zobrist.toggle_castling(i);

  zobrist.toggle_piece(moving_piece, static_cast<Square>(from_sq));

  const bool is_pawn = (moving_piece == WP || moving_piece == BP);

  if (captured_piece != NO_PIECE) {
    zobrist.toggle_piece(captured_piece, static_cast<Square>(captured_square));
    remove_piece(captured_piece, captured_square);
    state.removed[state.removed_count++] = {
        captured_piece, static_cast<Square>(captured_square)};
  }

  move_piece(moving_piece, from_sq, to_sq);

  if ((moving_piece == WK || moving_piece == BK) &&
      std::abs(from_sq - to_sq) == 2) {
    if (moving_piece == WK) {
      if (to_sq == G1) {
        move_piece(WR, H1, F1);
        state.removed[state.removed_count++] = {WR, H1};
        state.added[state.added_count++] = {WR, F1};
        zobrist.toggle_piece(WR, H1);
        zobrist.toggle_piece(WR, F1);
      } else if (to_sq == C1) {
        move_piece(WR, A1, D1);
        state.removed[state.removed_count++] = {WR, A1};
        state.added[state.added_count++] = {WR, D1};
        zobrist.toggle_piece(WR, A1);
        zobrist.toggle_piece(WR, D1);
      }
    } else {
      if (to_sq == G8) {
        move_piece(BR, H8, F8);
        state.removed[state.removed_count++] = {BR, H8};
        state.added[state.added_count++] = {BR, F8};
        zobrist.toggle_piece(BR, H8);
        zobrist.toggle_piece(BR, F8);
      } else if (to_sq == C8) {
        move_piece(BR, A8, D8);
        state.removed[state.removed_count++] = {BR, A8};
        state.added[state.added_count++] = {BR, D8};
        zobrist.toggle_piece(BR, A8);
        zobrist.toggle_piece(BR, D8);
      }
    }
  }

  const int promotion = get_promotion(move);
  if (promotion != PROMO_NONE && is_pawn) {
    static constexpr PieceType promo_types[5] = {PAWN /*unused*/, KNIGHT,
                                                 BISHOP, ROOK, QUEEN};
    Piece promoted_piece = static_cast<Piece>(promo_types[promotion] +
                                              (moving_piece == WP ? 0 : 6));

    // not toggling pawn at to_sq, the key was already removed at from_sq and at
    // to_sq it was never hashed. python position.py had a toggle which leaves a
    // random pawn key xored into the after-promotion hash which corrupts
    // tt/repetition.

    remove_piece(moving_piece, to_sq);

    add_piece(promoted_piece, to_sq);
    zobrist.toggle_piece(promoted_piece, static_cast<Square>(to_sq));
    state.added[state.added_count++] = {promoted_piece,
                                        static_cast<Square>(to_sq)};
    state.promotion_piece = promoted_piece;
  } else {
    zobrist.toggle_piece(moving_piece, static_cast<Square>(to_sq));
    state.added[state.added_count++] = {moving_piece,
                                        static_cast<Square>(to_sq)};
  }

  if (is_pawn || captured_piece != NO_PIECE)
    halfmove_clock = 0;
  else
    ++halfmove_clock;

  if (side_to_move == BLACK) ++fullmove_number;

  if (is_pawn && std::abs(from_sq - to_sq) == 16) {
    ep_square = (from_sq + to_sq) / 2;
    zobrist.toggle_ep(static_cast<Square>(ep_square));
  }

  zobrist.flip_side();
  side_to_move = static_cast<Color>(side_to_move ^ 1);

  ++ply;

  assert_board_consistent();
}

void Position::unmake_move() {
  --ply;
  PositionState& state = history[ply];

  const Move move = state.move;
  const Piece moving_piece = state.moved_piece;
  assert(moving_piece != NO_PIECE);

  const int from_sq = get_from(move);
  const int to_sq = get_to(move);
  const int flags = get_flags(move);
  const int promotion = get_promotion(move);

  if (promotion != PROMO_NONE && state.promotion_piece != NO_PIECE) {
    remove_piece(state.promotion_piece, to_sq);
    add_piece(moving_piece, from_sq);

    if (state.captured_piece != NO_PIECE)
      add_piece(state.captured_piece, state.captured_square);

  } else if (flags & EN_PASSANT) {
    move_piece(moving_piece, to_sq, from_sq);
    add_piece(state.captured_piece, state.captured_square);

  } else if ((moving_piece == WK || moving_piece == BK) &&
             std::abs(from_sq - to_sq) == 2) {
    move_piece(moving_piece, to_sq, from_sq);

    if (moving_piece == WK) {
      if (to_sq == G1)
        move_piece(WR, F1, H1);
      else if (to_sq == C1)
        move_piece(WR, D1, A1);
    } else {
      if (to_sq == G8)
        move_piece(BR, F8, H8);
      else if (to_sq == C8)
        move_piece(BR, D8, A8);
    }

  } else {
    move_piece(moving_piece, to_sq, from_sq);

    if (state.captured_piece != NO_PIECE)
      add_piece(state.captured_piece, state.captured_square);
  }

  side_to_move = static_cast<Color>(side_to_move ^ 1);

  castling_rights = state.castling_rights;
  ep_square = state.ep_square;
  halfmove_clock = state.halfmove_clock;
  fullmove_number = state.fullmove_number;
  zobrist.hash = state.hash;
  // pawn hash included, which wasn't in python
  zobrist.pawn_hash = state.pawn_hash;
  zobrist.non_pawn_hash[WHITE] = state.non_pawn_hash_white;
  zobrist.non_pawn_hash[BLACK] = state.non_pawn_hash_black;

  assert_board_consistent();
}

void Position::make_null_move() {
  assert(ply < MAX_HISTORY);
  PositionState& state = history[ply];

  state.move = MOVE_NONE;
  state.moved_piece = NO_PIECE;
  state.castling_rights = castling_rights;
  state.ep_square = ep_square;
  state.halfmove_clock = halfmove_clock;
  state.fullmove_number = fullmove_number;
  state.hash = zobrist.hash;
  hash_ring[ply] = zobrist.hash;
  state.pawn_hash = zobrist.pawn_hash;
  state.non_pawn_hash_white = zobrist.non_pawn_hash[WHITE];
  state.non_pawn_hash_black = zobrist.non_pawn_hash[BLACK];
  state.removed_count = 0;
  state.added_count = 0;

  if (ep_square != -1) zobrist.toggle_ep(static_cast<Square>(ep_square));
  zobrist.flip_side();
  ++ply;
  ep_square = -1;
  side_to_move = static_cast<Color>(side_to_move ^ 1);
}

void Position::unmake_null_move() {
  --ply;
  const PositionState& state = history[ply];

  castling_rights = state.castling_rights;
  ep_square = state.ep_square;
  halfmove_clock = state.halfmove_clock;
  fullmove_number = state.fullmove_number;
  zobrist.hash = state.hash;
  zobrist.pawn_hash = state.pawn_hash;
  zobrist.non_pawn_hash[WHITE] = state.non_pawn_hash_white;
  zobrist.non_pawn_hash[BLACK] = state.non_pawn_hash_black;

  side_to_move = static_cast<Color>(side_to_move ^ 1);
}

bool Position::is_repetition() const {
  const int limit = halfmove_clock < ply ? halfmove_clock : ply;
  int idx = ply - 2;
  int steps = 2;
  while (steps <= limit) {
    if (hash_ring[idx] == zobrist.hash) return true;
    idx -= 2;
    steps += 2;
  }
  return false;
}

bool Position::is_insufficient_material() const {
  // pawn, rook, and queen can mate
  if (pieces[WP] | pieces[BP] | pieces[WR] | pieces[BR] | pieces[WQ] |
      pieces[BQ])
    return false;

  const int wn = Bitboard::popcount(pieces[WN]);
  const int bn = Bitboard::popcount(pieces[BN]);
  const int wb = Bitboard::popcount(pieces[WB]);
  const int bb = Bitboard::popcount(pieces[BB]);
  const int minors = wn + bn + wb + bb;

  if (minors <= 1)
    return true;  // kvk or king vs minor can't mate (k vs k, k vs n, k vs b)

  if (minors == 2 && wb == 1 && bb == 1) {
    // K+B vs K+B is only a draw with same colored bishops.
    const int w_sq = Bitboard::lsb(pieces[WB]);
    const int b_sq = Bitboard::lsb(pieces[BB]);
    return (w_sq / 8 + w_sq % 8) % 2 == (b_sq / 8 + b_sq % 8) % 2;
  }

  return false;
}
void Position::pretty_print() const {
  for (int rank = 7; rank >= 0; --rank) {
    std::string line = std::to_string(rank + 1) + " ";
    for (int file = 0; file < 8; ++file) {
      Piece piece = piece_at(rank * 8 + file);
      if (piece == NO_PIECE)
        line += ". ";
      else {
        line += (piece < 6 ? "PNBRQK" : "pnbrqk")[piece % 6];
        line += " ";
      }
    }
    std::cout << line << '\n';
  }
  std::cout << "  a b c d e f g h\n";
}

void Position::assert_board_consistent() const {
#ifndef NDEBUG
  // occupancy mirrors piece bitboards
  U64 w = 0, b = 0;
  for (int piece = 0; piece < 6; ++piece) w |= pieces[piece];
  for (int piece = 6; piece < 12; ++piece) b |= pieces[piece];
  assert(w == white_occ);
  assert(b == black_occ);
  assert((w | b) == all_occ);
  assert((w & b) == 0);

  // exactly one king each
  assert(Bitboard::popcount(pieces[WK]) == 1);
  assert(Bitboard::popcount(pieces[BK]) == 1);

  // mailbox mirrors bitboards
  for (int sq = 0; sq < 64; ++sq) {
    Piece p = mailbox[sq];
    if (p == NO_PIECE)
      assert((all_occ & (1ULL << sq)) == 0);
    else
      assert((pieces[p] & (1ULL << sq)) != 0);
  }

  // incremental hash matches a from-scratch recompute
  Zobrist scratch;
  U64 h = scratch.compute(*this);
  assert(h == zobrist.hash);
  assert(scratch.pawn_hash == zobrist.pawn_hash);
  assert(scratch.non_pawn_hash[WHITE] == zobrist.non_pawn_hash[WHITE]);
  assert(scratch.non_pawn_hash[BLACK] == zobrist.non_pawn_hash[BLACK]);
#endif
}