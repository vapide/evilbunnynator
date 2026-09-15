#pragma once

#include <cstdint>

// Bob Jenkins'PRNG RKISS
class RKISS {
 public:
  explicit RKISS(uint64_t seed = 1) { set_seed(seed); }

  void set_seed(uint64_t seed) {
    a = 0xF1EA5EEDULL;
    b = c = d = seed;
    for (int i = 0; i < 20; ++i) rand64();
  }

  uint64_t rand64() {
    uint64_t e = a - rotate_left(b, 7);
    a = b ^ rotate_left(c, 13);
    b = c + rotate_left(d, 37);
    c = d + e;
    d = e + a;
    return d;
  }

  uint64_t rand64_sparse() { return rand64() & rand64() & rand64(); }

 private:
  static constexpr uint64_t rotate_left(uint64_t val, int shift) {
    return (val << shift) | (val >> (64 - shift));
  }

  uint64_t a, b, c, d;
};
