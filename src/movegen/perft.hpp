#pragma once

#include "../core/types.hpp"

class Position;

namespace Perft {

uint64_t perft(Position& pos, int depth);

uint64_t divide(Position& pos, int depth);

}  // namespace Perft