#pragma once

#include <iostream>
#include <vector>

#include "../core/types.hpp"

class Position;

U64 perft(int depth, Position& pos);

void perft_divide(int depth, Position& pos);