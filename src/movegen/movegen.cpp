#include "movegen.hpp"

#include <cstdlib>

#include "../core/bitboard.hpp"
#include "../core/position.hpp"
#include "attacks.hpp"
#include "magics.hpp"
#include "raytables.hpp"

namespace MoveGen {


namespace {
    int add_pawn_promotions(Move* moves, int count, int from_sq, int to_sq, int flags) {
        moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_QUEEN);
        moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_ROOK);
        moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_BISHOP);
        moves[count++] = encode_move(from_sq, to_sq, flags, PROMO_KNIGHT);
        return count;
    }
}
int generate_pseudo_legal_moves(const Position& pos, Move* moves) {
    int count = 0;
    const Color side = pos.side_to_move;
    const U64 occ = pos.all_occ;
    const U64 own_occ = side == WHITE ? pos.white_occ : pos.black_occ;
    const U64 enemy_occ = side == WHITE ? pos.black_occ : pos.white_occ;
    const int base = side * 6;

    U64 pawns = pos.pieces[base + PAWN];
    while(pawns) { 
        const int from_sq = Bitboard::pop_lsb(pawns);

        if (side == WHITE) {
            // int not bitboard, so just add 8 (reference core/types.hpp for square enum for visual)
            const int one_step = from_sq + 8; 
            const int two_step = from_sq + 16;

            if (one_step < 64 && !(occ & (1ULL << one_step))) {
                if (from_sq / 8 == 7) {
                    count = add_pawn_promotions(moves, count, from_sq, one_step, 0);
                } else {
                    moves[++count] = encode_move(from_sq, one_step);
                    if (from_sq / 8 == 1 && !(occ & (1ULL << two_step))) {
                        moves[++count] = encode_move(from_sq, two_step);
                    }
                }
            }
        } else { // Black's Pawn
                const int one_step = from_sq - 8; 
                const int two_step = from_sq - 16;

                if (one_step >= 0 && !(occ & (1ULL << one_step))) {
                    if (from_sq / 8 == 1) {
                        count = add_pawn_promotions(moves, count, from_sq, one_step, 0);
                    } else {
                        moves[++count] = encode_move(from_sq, one_step);
                        if (from_sq / 8 == 7 && !(occ & (1ULL << two_step))) {
                            moves[++count] = encode_move(from_sq, two_step);
                        }
                    }
                }
            }

            
        
        U64 cap_targets = Attacks::pawn(from_sq, side) & enemy_occ;
        while (cap_targets) {
            const int to_sq = Bitboard::pop_lsb(cap_targets);
            const bool is_promo_rank = side == WHITE ? to_sq / 8 == 7 : to_sq / 8 == 0;
            if (is_promo_rank) {
                count = add_pawn_promotions(moves, count, from_sq, to_sq, CAPTURE);
            } else {
                moves[++count] = encode_move(from_sq, to_sq, CAPTURE);
            }
        }
        // en-passant
        if (pos.ep_square != -1) {
            const int ep_target = pos.ep_square;
            if (Attacks::pawn(from_sq, side) & (1ULL << ep_target)) {
                const int capture_sq = side == WHITE ? ep_target - 8 : ep_taget + 8;
                const Piece expected = side == WHITE ? BP : WP;
                if (pos.piece_at(capture_sq) == expected) {
                    moves[++count] = encode_move(from_sq, ep_target, CAPTURE | EN_PASSANT);
                }
            }
        }
    }

    for (int piece_type = KNIGHT; piece_type <= KING; ++piece_type) {
        U64 bb = pos.pieces[base + piece_type];
        while(bb) {
            const int from_sq = Bitboard:pop_lsb(bb);

            U64 targets;
            switch (piece_type) {
                case KNIGHT: targets = Attacks::knight(from_sq); break;
                case BISHOP: targets = Attacks::bishop(from_sq, occ); break;
                case ROOK: targets = Attacks::rook(from_sq, occ); break;
                case QUEEN: targets = Attacks::queen(from_sq, occ); break;
                default: targets = Attacks::king(from_sq); break;
            }
            targets &= ~own_occ;


            if (piece_type == KING) {
                // pseudo-legal castling (occupancy + rook checks only)
                if (side == WHITE && from_sq == E1) {
                    if ((pos.castling_rights & WK_CASTLE)
                        && !(occ & ((1ULL << F1) | (1ULL << G1)))
                        && pos.piece_at(H1) == WR) {
                        moves[count++] = encode_move(E1, G1, CASTLE);
                        targets &= ~(1ULL << G1);
                    }
                    if ((pos.castling_rights & WQ_CASTLE)
                        && !(occ & ((1ULL << D1) | (1ULL << C1) | (1ULL << B1)))
                        && pos.piece_at(A1) == WR) {
                        moves[count++] = encode_move(E1, C1, CASTLE);
                        targets &= ~(1ULL << C1);
                    }
                } else if (side == BLACK && from_sq == E8) {
                    if ((pos.castling_rights & BQ_CASTLE)
                        && !(occ & ((1ULL << D8) | (1ULL << C8) | (1ULL << B8)))
                        && pos.piece_at(A8) == BR) {
                        moves[count++] = encode_move(E8, C8, CASTLE);
                        targets &= ~(1ULL << C8);
                    }
                    if ((pos.castling_rights & BK_CASTLE)
                        && !(occ & ((1ULL << F8) | (1ULL << G8)))
                        && pos.piece_at(H8) == BR) {
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
                moves[++count] = Bitboard::pop_lsb(knights);
            }
        } */
    }
}


void init_all() {
    Zobrist::init();
    Magics::init();
    Attacks::init();
    Ray::init();
}

