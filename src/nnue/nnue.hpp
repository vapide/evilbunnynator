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

  static constexpr std::size_t FILE_SIZE =
      sizeof(int16_t) *
      (static_cast<std::size_t>(INPUT_SIZE) * L1  // ft_weights
       + L1                                       // ft_biases
       + 2 * L1                                   // out_weights
       + 1);                                      // out_bias

  enum class LoadStatus {
    Ok,
    NotFound,       // doesn't exist or can't open
    WrongSize,      // does exist, but is the wrong size
    ReadError,      // right size, but reading had issues
    UnsafeWeights,  // parsed, but has issues with weights
  };

  LoadStatus load_checked(const std::string& path);
  LoadStatus load_from_memory(const uint8_t* data, std::size_t size);

  bool load(const std::string& path) {
    return load_checked(path) == LoadStatus::Ok;
  }
  bool loaded() const { return is_loaded; }

  static const char* status_text(LoadStatus s);

  alignas(64) int16_t ft_weights[INPUT_SIZE * L1] = {};
  alignas(64) int16_t ft_biases[L1] = {};
  alignas(64) int16_t out_weights[2 * L1] = {};  // [0..L1) stm, [L1..2L1) opp
  int16_t out_bias = 0;

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

  void set_weights(const NNUEWeights* w) { net = w; }
  const NNUEWeights* weights() const { return net; }
  bool has_weights() const { return net && net->loaded(); }

  // rebuilding of the accumulator
  // stack should be reset to 0
  void refresh_from_pos(const Position& pos);

  void push(const PositionState& state) {
    Accumulator& e = stack[++idx];
    e.removed[0] = state.removed[0];
    e.removed[1] = state.removed[1];
    e.added[0] = state.added[0];
    e.added[1] = state.added[1];
    e.removed_count = static_cast<int8_t>(state.removed_count);
    e.added_count = static_cast<int8_t>(state.added_count);
    e.computed = false;
  }
  void push_null() {
    Accumulator& e = stack[++idx];
    e.removed_count = 0;
    e.added_count = 0;
    e.computed = false;
  }
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