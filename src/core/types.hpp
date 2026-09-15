// clang-format off
#pragma once

#include <cstdint>

using U64 = uint64_t;

// unscoped enums for integer bitwise arithmetic
// ex.: color ^ 1, piece % 6, pieces[piece],
// don't need to cast it over and over again

enum Color : int {
    WHITE = 0,
    BLACK = 1
};


// order also allows for comparing piece values directly to decide cheapness

enum PieceType : int {
    PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5
};


enum Piece : int {
    WP = 0, WN = 1, WB = 2, WR = 3, WQ = 4, WK = 5,
    BP = 6, BN = 7, BB = 8, BR = 9, BQ = 10, BK = 11,
    NO_PIECE = 12
};

enum Square : int {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8,
    SQ_NONE = -1
};

// move flags bitflags packed into the move in bits 12-15
enum MoveFlag : int {
    QUIET            = 0,
    CAPTURE          = 1 << 0,
    DOUBLE_PAWN_PUSH = 1 << 1,
    EN_PASSANT       = 1 << 2,
    CASTLE           = 1 << 3
};

// promotion flags bits 16-19 of the move
enum PromoFlag : int {
    PROMO_NONE   = 0,
    PROMO_KNIGHT = 1,
    PROMO_BISHOP = 2,
    PROMO_ROOK   = 3,
    PROMO_QUEEN  = 4
};

// position castling-rights bits
enum CastleRight : int {
    WK_CASTLE = 1 << 0,
    WQ_CASTLE = 1 << 1,
    BK_CASTLE = 1 << 2,
    BQ_CASTLE = 1 << 3
};

constexpr char FILES[] = "abcdefgh";

// castling_rights &= CASTLING_MASKS[sq] for every move and its from_sq,
// as well as cap_sq. moving/capturing a rook or king home square clears the respective
// rights, ported from core/types.py. 
constexpr int CASTLING_MASKS[64] = {
    13, 15, 15, 15, 12, 15, 15, 14,   // a1 ... e1 ... h1
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
     7, 15, 15, 15,  3, 15, 15, 11    // a8 ... e8 ... h8
};

// legal position cannot exceed 256 moves
// 218 is the max but its a safe buffer in case of
// miscounting or the rules of chess change
constexpr int MAX_MOVES = 256;
