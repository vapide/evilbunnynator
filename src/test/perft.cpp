#include "perft.hpp"

#include <iostream>
#include <vector>

#include "../core/position.hpp"
#include "../movegen/movegen.hpp"

U64 perft(int depth, Position& pos) {
  if (depth == 0) {
    return 1ULL;
  }

  U64 nodes = 0;

  Move moves[MAX_MOVES];
  const int move_count = MoveGen::generate_legal_moves(pos, moves);

  if (depth == 1) {
    return move_count;
  }

  for (int i = 0; i < move_count; ++i) {
    const Move& move = moves[i];
    pos.make_move(move);
    nodes += perft(depth - 1, pos);
    pos.unmake_move();
  }

  return nodes;
}

void perft_divide(int depth, Position& pos) {
  if (depth == 0) {
    std::cout << "Total Nodes: 1" << std::endl;
    return;
  }

  U64 total_nodes = 0;
  Move moves[MAX_MOVES];
  const int move_count = MoveGen::generate_legal_moves(pos, moves);

  std::cout << "\nPERFT DIV DEPTH: " << depth << std::endl;

  for (int i = 0; i < move_count; ++i) {
    const Move& move = moves[i];
    pos.make_move(move);
    U64 nodes_for_move = perft(depth - 1, pos);
    pos.unmake_move();

    std::string text = to_uci(move);

    std::cout << text << ": " << nodes_for_move << std::endl;
    total_nodes += nodes_for_move;
  }

  std::cout << "\nTotal Root Moves: " << move_count << std::endl;
  std::cout << "Total Leaf Nodes: " << total_nodes << std::endl;
}