#pragma once

#include "../core/position.hpp"

class Engine {
 public:
  Engine() { board = Position::startpos(); }

  Position board;
};
