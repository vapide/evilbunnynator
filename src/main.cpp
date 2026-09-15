#include <chrono>
#include <iostream>
#include <string>

#include "core/move.hpp"
#include "core/position.hpp"
#include "movegen/movegen.hpp"
#include "test/perft.hpp"

int main() {
  auto start = std::chrono::high_resolution_clock::now();

  init_all();

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> duration = end - start;
  std::cout << "init_all() took: " << duration.count() << " ms" << std::endl;

  Position pos = Position::from_fen(
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

  pos.pretty_print();
  Move moves[MAX_MOVES];

  int count = MoveGen::generate_pseudo_legal_moves(pos, moves);

  for (int i = 0; i < count; ++i) {
    std::cout << to_uci(moves[i]) << std::endl;  // to_uci(move) << std::endl;
  }

  for (int i = 0; i < 6; ++i) {
    std::cout << "perft depth: " << i << " nodes: ";
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << perft(i, pos);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << " time: " << duration.count() << " ms" << std::endl;
  }
  return 0;
}