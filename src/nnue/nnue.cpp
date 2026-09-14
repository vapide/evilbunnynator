#include "nnue.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <system_error>

#ifdef __AVX2__
#include <immintrin.h>
#endif

#include "../core/bitboard.hpp"
#include "../core/position.hpp"

// I think this will be necessary later since python's floor division differs from c++ by it going to -infinity
static inline int64_t floor_div(int64_t a, int64_t b) {
    int64_t q = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0)))
        --q;
    return q;
}