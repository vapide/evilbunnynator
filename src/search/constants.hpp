#pragma once

#include <climits>

constexpr int INF_SCORE = 32000;

constexpr int CHECKMATE = 31000;
constexpr int CHECKMATE_THRESHOLD = CHECKMATE - 256;

constexpr int DRAW_HALFMOVE_LIMIT = 100;

constexpr int DRAW = 0;

constexpr int MAX_PLY = 128;

constexpr int EVAL_NONE = INT_MIN;