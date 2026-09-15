#include "bitboard.hpp"

#include <iostream>

namespace Bitboard {

void pretty_print(U64 bb) {
  for (int rank = 7; rank >= 0; --rank) {
    for (int file = 0; file < 8; ++file) {
      int square = rank * 8 + file;

      if (bb & (1ULL << square))
        std::cout << "1 ";
      else
        std::cout << ". ";
    }

    std::cout << '\n';
  }

  std::cout << '\n';
}

}  // namespace Bitboard