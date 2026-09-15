#pragma once

#include "../core/types.hpp"

class Position;

struct KingSafety {
  int king;
  U64 checkers;
  U64 pinned;
  U64 evasion;  // ~0 when not in check, otherwise it will be 0 on double check
  U64 pin_masks[64];
};

namespace MoveGen {

KingSafety analyze_king_safety(const Position& pos);

// when attackers_to(king_sq) is already known skip it
KingSafety analyze_king_safety(const Position& pos, U64 known_checkers);

}  // namespace MoveGen
