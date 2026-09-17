#pragma once

#include <string>
#include <vector>

#include "../engine/engine.hpp"

// whole point of this is to read lines, decide what they mean, change the
// engine, then print replies. no chess involved here, and engine has no
// protocol strings, only uci.
class UCI {
 public:
  // while std::cin basically
  void loop();

  // returns false when session should end, this is public for testing
  bool handle_command(const std::string& line);

 private:
  void handle_position(const std::vector<std::string>& args);
  void handle_go(const std::vector<std::string>& args);
  void handle_uci();
  void handle_help(const std::vector<std::string>& args);
  Move legal_move_from_uci(Position& pos, const std::string& text);
  void write(const std::string& text);
  Engine engine;
};
