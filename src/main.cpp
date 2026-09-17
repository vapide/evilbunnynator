#include <chrono>
#include <iostream>
#include <string>

#include "engine/engine.hpp"
#include "movegen/movegen.hpp"
#include "uci/uci.hpp"
#include "utils/paths.hpp"

int main(int argc, char** argv) {
  set_executable_dir(argv[0]);
  init_all();

  UCI uci;
  uci.loop();
  return 0;
}