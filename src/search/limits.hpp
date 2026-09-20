#pragma once

#include <cstdint>
#include <vector>

#include "../core/move.hpp"

// everything needed for the go command.
// -1 is not given, since no depth cannot be treated as being depth of 0.
// winc binc are exceptions because a missing increment will be zero
struct SearchLimits {
  int depth = -1;
  int64_t movetime = -1;
  int64_t nodes = -1;

  int64_t wtime = -1;
  int64_t btime = -1;
  int64_t winc = 0;
  int64_t binc = 0;

  int movestogo = -1;

  bool infinite = false;
  int mate = -1;

  bool ponder = false;
  int perft = -1;

  std::vector<Move> searchmoves;
};
