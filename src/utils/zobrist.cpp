#include "zobrist.hpp"

#include "../core/bitboard.hpp"
#include "../core/position.hpp"
#include "rkiss.hpp"

std::array<U64, Zobrist::NUM_KEYS> Zobrist::keys{};

void Zobrist::init(uint64_t seed) {
    RKISS r(seed);
    for (auto& k : keys)
        k = r.rand64();
}

U64 Zobrist::compute(const Position& position) {
    U64 h = 0;
    U64 pawn_h = 0;
    U64 non_pawn_h[2] = {0, 0};

    for (int piece = 0; piece < 12; ++piece) {
        U64 bb = position.pieces[piece];
        while (bb) {
            Square sq = Bitboard::pop_lsb(bb);
            U64 key = keys[piece * 64 + sq];
            h ^= key;
            if (piece % 6 == PAWN) {
                pawn_h ^= key;
            } else {
                Color color = piece < 6 ? WHITE : BLACK;
                non_pawn_h[color] ^= key;
            }
        }
    }

    if (position.side_to_move == BLACK)
        h ^= keys[SIDE_KEY];

    for (int i = 0; i < 4; ++i)
        if (position.castling_rights & (1 << i))
            h ^= keys[CASTLING_START + i];

    if (position.ep_square != SQ_NONE)
        h ^= keys[EP_START + position.ep_square % 8];

    pawn_hash = pawn_h;
    non_pawn_hash[0] = non_pawn_h[0];
    non_pawn_hash[1] = non_pawn_h[1];

    return h;
}
