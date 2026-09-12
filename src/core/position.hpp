#pragma once

#include <string>

#include "../utils/zobrist.hpp"
#include "move.hpp"
#include "position_state.hpp"
#include "types.hpp"

// generous pre-allocation
constexpr int MAX_HISTORY = 1024;

class Position {
public:
    U64 pieces[12] = {};
    // O(1) piece at lookup
    Piece mailbox[64];

    U64 white_occ = 0;
    U64 black_occ = 0;
    U64 all_occ = 0;

    Color side_to_move = WHITE;
    int castling_rights = WK_CASTLE | WQ_CASTLE | BK_CASTLE | BQ_CASTLE;
    int ep_square = -1;
    int halfmove_clock = 0;
    int fullmove_number = 1;

    Zobrist zobrist;

    int ply = 0;
    PositionState history[MAX_HISTORY];
    // copy of history[k].hash for repetition detection
    // packing 8 per line makes the scan optimized
    U64 hash_ring[MAX_HISTORY];

    Position();

    static Position from_fen(const std::string& fen);
    static Position startpos();
    std::string to_fen() const;

    Piece piece_at(int square) const { return mailbox[square]; }

    U64 occupancy(Color c) const { return c == WHITE ? white_occ : black_occ; }

    void add_piece(Piece piece, int square);
    void remove_piece(Piece piece, int square);
    void move_piece(Piece piece, int from_sq, int to_sq);

    Square king_square(Color color) const;
    bool has_non_pawn_material(Color color) const;

    void make_move(Move move);
    void unmake_move();
    void make_null_move();
    void unmake_null_move();

    // state of the last move made this is for dirty pieces for NNUE later on
    const PositionState& last_state() const { return history[ply - 1]; }

    bool is_repetition() const;
    bool is_insufficient_material() const;

    // will be defined in movegen.cpp
    // last two non const because legal movegen make/unmakes en passant candidates
    bool in_check(Color color) const;
    bool is_checkmate(Color color);
    bool is_game_over();

    void update_occupancy();
    void pretty_print() const;

private:
    void assert_board_consistent() const;   // invariant checks
};
