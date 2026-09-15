#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "../core/position_state.hpp"
#include "../core/types.hpp"
#include "accumulator.hpp"

class Position;

class NNUEWeights {
 public:
  static constexpr int INPUT_SIZE = 768;
  static constexpr int L1 = 1024;

 private:
  bool is_loaded = false;
};

class NNUE {
 public:
  static constexpr int L1 = NNUEWeights::L1;
  static constexpr int SCALE = 400;
  static constexpr int QA = 255;
  static constexpr int QB = 64;
  static constexpr int QAB = QA * QB;

  // max ply is 128 and its not too harmful to be generous
  static constexpr int STACK_SIZE = 144;

  // weights methods/functions
  // set_weights, weights, has_weights.

  // rebuilding of the accumulator
  // stack should be reset to 0
  void refresh_from_pos(const Position& pos);

  void push(const PositionState& state) {};
  void push_null() {};
  void pop() { --idx; }

  // non constant since it is used with older accumulators
  int evaluate(Color side_to_move);

 private:
  static int feature_index(int piece, int square, Color perspective) {
    if (perspective == BLACK) {
      square ^= 56;
      piece = piece < 6 ? piece + 6 : piece - 6;
    }
    return piece * 64 + square;
  }

  // compute stack[k].acc using stack[k-1].acc
  void materialize_one(int k);

  Accumulator stack[STACK_SIZE];
  int idx = 0;
  const NNUEWeights* net = nullptr;
};