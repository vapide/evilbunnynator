#pragma once

#include "constants.hpp"

struct PlyRecord {
  int m_diff = 0;

  void reset() { m_diff = 0; }
};