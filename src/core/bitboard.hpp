#pragma once

#include <bit>

#include "types.hpp"

// namespace bitboard since BB can be confused with Black Bishop
namespace Bitboard {

constexpr U64 DIAG_A1H8 = 0x8040201008040201ULL;
constexpr U64 DIAG_H1A8 = 0x0102040810204080ULL;

constexpr U64 FILE_A = 0x0101010101010101ULL;
constexpr U64 FILE_H = 0x8080808080808080ULL;

constexpr U64 RANK_1 = 0x00000000000000FFULL;
constexpr U64 RANK_2 = 0x000000000000FF00ULL;
constexpr U64 RANK_7 = 0x00FF000000000000ULL;
constexpr U64 RANK_8 = 0xFF00000000000000ULL;

constexpr U64 LIGHT = 0x55AA55AA55AA55AAULL;
constexpr U64 DARK  = 0xAA55AA55AA55AA55ULL;

// attack masks
constexpr U64 FILE_B = 0x0202020202020202ULL;
constexpr U64 FILE_G = 0x4040404040404040ULL;

constexpr U64 NOT_A  = ~FILE_A;
constexpr U64 NOT_B  = ~FILE_B;
constexpr U64 NOT_G  = ~FILE_G;
constexpr U64 NOT_H  = ~FILE_H;

constexpr U64 NOT_AB = ~(FILE_A | FILE_B);
constexpr U64 NOT_GH = ~(FILE_G | FILE_H);

//consider using <bit> for popcount, lsb, msb, etc.

inline Square pop_lsb(U64& bb)
{
    Square sq = static_cast<Square>(std::countr_zero(bb));
    bb &= bb - 1;
    return sq;
}

// number of 1s
inline int popcount(U64 bb)
{
    return std::popcount(bb);
}

constexpr U64 square_bb(Square sq)
{
    return 1ULL << static_cast<int>(sq);
}

// gets first set bit
inline Square lsb(U64 bb)
{
    return static_cast<Square>(std::countr_zero(bb));
}

// gets highest set bit
inline Square msb(U64 bb)
{
    return static_cast<Square>(63 - std::countl_zero(bb));
}

void pretty_print(U64 bb);

}