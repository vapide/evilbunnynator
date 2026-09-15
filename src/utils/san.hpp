#pragma once

#include <string>

#include "../core/move.hpp"

class Position;

// algebriac notation for a legal move in pos
// check/mate suffix detection uses make and uses unmake, making Position
// non-const
std::string move_to_san(Position& pos, Move move);
