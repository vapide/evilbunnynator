#pragma once

#include "../core/types.hpp"

// forward declaration
class Position;

// different from see static exchange evaluation
// just a base for testing search cheaply
constexpr int MATERIAL_VALUES[5] = {100, 300, 300, 500, 900};

// exchange scale, not heuristic scale
// decided to make bishops slightly more than knights
// king is considered 0
constexpr int PIECE_VALUES[6] = {85, 270, 290, 450, 820, 0};

// side to move perspective, just for now.
int material_eval(const Position& pos);

// total non king material on the board from both sides.
int total_material(const Position& pos);