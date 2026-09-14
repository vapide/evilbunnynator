#include "movegen.hpp"

#include <cstdlib>

#include "../core/bitboard.hpp"
#include "../core/position.hpp"
#include "attacks.hpp"
#include "magics.hpp"
#include "raytables.hpp"


void init_all() {
    Zobrist::init();
    Magics::init();
    Attacks::init();
    Ray::init();
}

