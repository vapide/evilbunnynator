#pragma once

#include "move.hpp"
#include "types.hpp"

// based off of Stockfish's DirtyPiece, but has a more compact representation
// move removes at most 2 pieces, adds at most 2, and is relevant to NNUE
// incremental updates
struct DirtyPiece {
  Piece piece;
  Square square;
};

// whatever make_move destroys, that unmake_move cannot recompute, must be saved
// and not derived. there is one entry per ply which is in Position::history,
// similar to core/position.py, without the accumulator field this is because
// the NNUE stack will own that
struct PositionState {
  Move move = MOVE_NONE;
  Piece moved_piece = NO_PIECE;
  Piece captured_piece = NO_PIECE;
  int captured_square = -1;
  Piece promotion_piece = NO_PIECE;
  int flags = 0;

  int castling_rights = 0;
  int ep_square = -1;
  int halfmove_clock = 0;
  int fullmove_number = 1;

  U64 hash = 0;
  U64 pawn_hash = 0;
  U64 non_pawn_hash_white = 0;
  U64 non_pawn_hash_black = 0;

  // dirty-piece lists for the incremental update of this move for the NNUE
  DirtyPiece removed[2];
  DirtyPiece added[2];
  int removed_count = 0;
  int added_count = 0;
};
