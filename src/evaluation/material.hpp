#pragma once

#include "../core/types.hpp"

// forward declaration
class Position;

// will be different from see static exchange evaluation
// just a base for testing search cheaply
constexpr int MATERIAL_VALUES[5] = {100, 300, 300, 500, 900};

// side to move perspective, just for now.
int material_eval(const Position& pos);

// total non king material on the board from both sides.
int total_material(const Position& pos);
