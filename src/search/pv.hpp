#pragma once

#include <iostream>

#include "../core/move.hpp"
#include "constants.hpp"

// triangular principal variation
class PrincipalVariation {
 public:
  void clear() {
    for (int ply = 0; ply <= MAX_PLY; ++ply) length[ply] = 0;
  }

  void update(int ply, Move move) {
    if (ply >= MAX_PLY) return;

    const int child_length = length[ply + 1];

    lines[ply][0] = move;

    for (int i = 0; i < child_length; ++i) {
      lines[ply][i + 1] = lines[ply + 1][i];
    }

    length[ply] = child_length + 1;
  }

  void reset_line(int ply) {
    if (ply <= MAX_PLY) {
      length[ply] = 0;
    }
  }

  int line_length() const { return length[0]; }
  Move line_move(int i) const { return lines[0][i]; }

  std::string to_string() {
    std::string final = "";
    for (int i = 0; i < line_length(); ++i) {
      final += to_uci(line_move(i)) + " ";
    }
    return final;
  }

 private:
  Move lines[MAX_PLY + 2][MAX_PLY + 2];
  int length[MAX_PLY + 2] = {};
};
