#include "position.hpp"

#include <cassert>
#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include "bitboard.hpp"

// requires the following methods for position:
// 

Position::Position() {
    for (int sq = 0; sq < 64; ++sq)
        mailbox[sq] = NO_PIECE;
}

// unfinished
Position Position::from_fen(const std::string& fen) { 
    Position position;

    std::istringstream ss(fen);
    std::string placement, stm, castling, ep, halfmove = "0", fullmove = "1";
    ss >> placement >> stm >> castling >> ep >> halfmove >> fullmove;

    return position;
}

Position Position::startpos() {
    return from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

void Position::update_occupancy() {
    white_occ = 0;
    black_occ = 0;
    for (int piece = 0; piece < 6; ++piece)
        white_occ |= pieces[piece];
    for (int piece = 6; piece < 12; ++piece)
        black_occ |= pieces[piece];
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

void Position::make_move(Move move) {};

void Position::unmake_move() {};

void Position::make_null_move() {};

void Position::unmake_null_move() {};

bool Position::is_repetition() const {};

bool Position::is_insufficient_material() const {};

void Position::pretty_print() const {
    for (int rank = 7; rank >= 0; --rank) {
        std::string line = std::to_string(rank + 1) + " ";
        for (int file = 0; file < 8; ++file) {
            Piece piece = piece_at(rank * 8 + file);
            if (piece == NO_PIECE)
                line += ". ";
            else {
                line += (piece < 6 ? "PNBRQK" : "pnbrqk")[piece % 6];
                line += ' ';
            }
        }
        std::cout << line << '\n';
    }
    std::cout << "  a b c d e f g h\n";
}

void Position::assert_board_consistent() const {
#ifndef NDEBUG
// stuff
#endif
}