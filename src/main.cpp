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
      // startpos
      //"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

      // kiwipete
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"

      // position 3
      //"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1"  // position 3

      // position 5
      //"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8");
  );
  pos.pretty_print();
  Move moves[MAX_MOVES];

  int count = MoveGen::generate_pseudo_legal_moves(pos, moves);

  for (int i = 0; i < count; ++i) {
    std::cout << to_uci(moves[i]) << std::endl;  // to_uci(move) << std::endl;
  }

  for (int i = 0; i < 7; ++i) {
    std::cout << "perft depth: " << i;
    auto start = std::chrono::high_resolution_clock::now();
    perft_divide(i, pos);
    // std::cout << perft(i, pos);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    std::cout << " time: " << duration.count() << " ms" << std::endl;
  }

  /*
  Move movese2e3[MAX_MOVES];

  Move movesg2g3[MAX_MOVES];

  Position pose2e3 =
      Position::from_fen("8/2p5/3p4/KP5r/1R3p1k/4P3/6P1/8 b - - 0 1");

  Position posg2g3 =
      Position::from_fen("8/2p5/3p4/KP5r/1R3p1k/6P1/4P3/8 b - - 0 1");

  int counte2e3 = MoveGen::generate_pseudo_legal_moves(pose2e3, movese2e3);

  std::cout << "\ne2e3 moves:\n";

  for (int i = 0; i < counte2e3; ++i) {
    std::cout << to_uci(movese2e3[i])
              << std::endl;  // to_uci(move) << std::endl;
  }

  int countg2g3 = MoveGen::generate_pseudo_legal_moves(posg2g3, movesg2g3);

  std::cout << "\ng2g3 moves:\n";

  for (int i = 0; i < countg2g3; ++i) {
    std::cout << to_uci(movesg2g3[i])
              << std::endl;  // to_uci(move) << std::endl;
  }

  */
  return 0;
}