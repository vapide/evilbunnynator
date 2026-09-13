#include "attacks.hpp"

namespace Attacks {

U64 KNIGHT_ATTACKS[64];
U64 KING_ATTACKS[64];
U64 PAWN_ATTACKS[2][64];

// computing at startup, unlike python because in c++ its faster
// and is more convenient, and smaller file size, and 
// its "cheap"
static U64 knight_mask(int square) {
    U64 b = 1ULL << square;
    U64 attacks = 0;

    attacks |= (b & Bitboard::NOT_H) << 17;
    attacks |= (b & Bitboard::NOT_A) << 15;
    attacks |= (b & Bitboard::NOT_GH) << 10;
    attacks |= (b & Bitboard::NOT_AB) << 6;
    attacks |= (b & Bitboard::NOT_H) >> 15;
    attacks |= (b & Bitboard::NOT_A) >> 17;
    attacks |= (b & Bitboard::NOT_GH) >> 6;
    attacks |= (b & Bitboard::NOT_AB) >> 10;

    return attacks;
}

static U64 king_mask(int square) {
    U64 b = 1ULL << square;
    U64 attacks = 0;

    attacks |= b << 8;
    attacks |= b >> 8;
    attacks |= (b << 1) & Bitboard::NOT_A;
    attacks |= (b >> 1) & Bitboard::NOT_H;
    attacks |= (b << 9) & Bitboard::NOT_A;
    attacks |= (b << 7) & Bitboard::NOT_H;
    attacks |= (b >> 9) & Bitboard::NOT_H;
    attacks |= (b >> 7) & Bitboard::NOT_A;

    return attacks;
}

static U64 pawn_mask(int square, Color color) {
    U64 b = 1ULL << square;
    if (color == WHITE)
        return ((b << 7) & Bitboard::NOT_H) | ((b << 9) & Bitboard::NOT_A);
    return ((b >> 7) & Bitboard::NOT_A) | ((b >> 9) & Bitboard::NOT_H);
}

void init() {
    for (int sq = 0; sq < 64; ++sq) {
        KNIGHT_ATTACKS[sq] = knight_mask(sq);
        KING_ATTACKS[sq] = king_mask(sq);
        PAWN_ATTACKS[WHITE][sq] = pawn_mask(sq, WHITE);
        PAWN_ATTACKS[BLACK][sq] = pawn_mask(sq, BLACK);
    }
}

}
