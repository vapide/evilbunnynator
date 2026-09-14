#include <iostream>
#include <chrono>
#include <string>

#include "movegen/movegen.hpp"
#include "core/position.hpp"
#include "core/move.hpp"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    init_all();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << "init_all() took: " << duration.count() << " ms" << std::endl;

    Position pos = Position::from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Move moves[MAX_MOVES];

    MoveGen::generate_pseudo_legal_moves(pos, moves);

    for(int move : moves) {
        std::cout << to_uci(move) << std::endl;
    }
    return 0;
}