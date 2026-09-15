#pragma once

#include "../core/move.hpp"
#include "../core/types.hpp"
#include "kingsafety.hpp"
class Position;

namespace MoveGen {

// every piece of color attacking square for occupancy
U64 attackers_to(const Position& pos, int square, Color color, U64 occupancy);
U64 attackers_to(const Position& pos, int square, Color color);

// every square color attacks occupancy
U64 attack_map(const Position& pos, Color color, U64 occupancy);

bool is_square_attacked(const Position& pos, int square, Color attacker_color);
bool is_square_attacked(const Position& pos, int square, Color attacker_color,
                        U64 custom_occ);

// direct checks only
bool gives_check(const Position& pos, Move move);

// pos is not constant because of en passant make unmake
int generate_legal_moves(Position& pos, Move* moves);
int generate_legal_moves(Position& pos, const KingSafety& safety, Move* moves);
int generate_legal_captures(Position& pos, Move* moves);
int generate_legal_captures(Position& pos, const KingSafety& safety,
                            Move* moves);

// pseudo-legal generation even though genlegal generates directly
// this can be useful for perft later
int generate_pseudo_legal_moves(const Position& pos, Move* moves);

}  // namespace MoveGen

namespace {
int add_pawn_promotions(Move* moves, int count, int from_sq, int to_sq,
                        int flags);
}

void init_all();
