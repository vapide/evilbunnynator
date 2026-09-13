#include "magics.hpp"

#include <bit>

#include "magic_tables.hpp"

namespace Magics {

SquareEntry ROOK[64];
SquareEntry BISHOP[64];
U64 ROOK_POOL[102400];
U64 BISHOP_POOL[5248];

namespace {

constexpr int ROOK_DIRS[4][2]   = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
constexpr int BISHOP_DIRS[4][2] = {{1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

// ray walking, only used at init time to fill all of the pools
// should test how long this takes.
U64 slider_attacks(int square, U64 occupancy, const int dirs[4][2]) {
    U64 attacks = 0;
    const int sf = square % 8, sr = square / 8;
    for (int d = 0; d < 4; ++d) {
        int f = sf + dirs[d][0], r = sr + dirs[d][1];
        while (f >= 0 && f < 8 && r >= 0 && r < 8) {
            const int sq = r * 8 + f;
            attacks |= 1ULL << sq;
            if (occupancy & (1ULL << sq))
                break;
            f += dirs[d][0];
            r += dirs[d][1];
        }
    }
    return attacks;
}

// Carry-Rippler https://analog-hors.github.io/site/magic-bitboards/
U64 slider_mask(int square, const int dirs[4][2]) {
    U64 mask = 0;
    const int sf = square % 8, sr = square / 8;
    for (int d = 0; d < 4; ++d) {
        int f = sf + dirs[d][0], r = sr + dirs[d][1];
        while (true) {
            const int nf = f + dirs[d][0], nr = r + dirs[d][1];
            if (nf < 0 || nf >= 8 || nr < 0 || nr >= 8)
                break;   // (f,r) is the edge square and should be excluded from the mask
            mask |= 1ULL << (r * 8 + f);
            f = nf;
            r = nr;
        }
    }
    return mask;
}

void init_one(SquareEntry* entries, U64* pool, const uint64_t* magics,
              const int dirs[4][2]) {
    int offset = 0;
    for (int sq = 0; sq < 64; ++sq) {
        SquareEntry& e = entries[sq];
        e.mask = slider_mask(sq, dirs);
        e.magic = magics[sq];
        e.shift = 64 - std::popcount(e.mask);
        e.offset = offset;

        // enumerate every subset that is of the mask
        U64 subset = 0;
        do {
            const U64 index = (subset * e.magic) >> e.shift;
            pool[e.offset + index] = slider_attacks(sq, subset, dirs);
            subset = (subset - e.mask) & e.mask;
        } while (subset);

        offset += 1 << std::popcount(e.mask);
    }
}

}  // namespace

void init() {
    init_one(ROOK, ROOK_POOL, ROOK_MAGICS, ROOK_DIRS);
    init_one(BISHOP, BISHOP_POOL, BISHOP_MAGICS, BISHOP_DIRS);
}

}
