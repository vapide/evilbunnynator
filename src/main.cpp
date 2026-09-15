#include <chrono>
#include <iostream>
#include <string>

#include "core/move.hpp"
#include "core/position.hpp"
#include "movegen/movegen.hpp"

int main() {
  auto start = std::chrono::high_resolution_clock::now();

  init_all();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "init_all() took: " << duration.count() << " ms" << std::endl;

  Position pos = Position::from_fen(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  pos.pretty_print();
  Move moves[MAX_MOVES];

  int count = MoveGen::generate_pseudo_legal_moves(pos, moves);

  for (int i = 0; i < count; ++i) {
    std::cout << to_uci(moves[i]) << std::endl;  // to_uci(move) << std::endl;
  }
  return 0;
}