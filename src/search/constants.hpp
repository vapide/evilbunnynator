#pragma once

#include <climits>

constexpr int INF_SCORE = 32000;

constexpr int CHECKMATE = 31000;
constexpr int CHECKMATE_THRESHOLD = CHECKMATE - 256;

constexpr int DRAW_HALFMOVE_LIMIT = 100;

constexpr int DRAW = 0;

constexpr int MAX_PLY = 128;

constexpr int EVAL_NONE = INT_MIN;

// difference in material before a draw becomes being worth zero
constexpr int DRAW_MDIFF_BIAS = 100;

// what the draw is worth then
constexpr int DRAW_BIAS_SCORE = 50;

// flooring like in python (to -inf, not to 0 like in c++)
constexpr int floordiv(int a, int b) {
  const int q = a / b;
  return (a % b != 0 && (a < 0) != (b < 0)) ? q - 1 : q;
}