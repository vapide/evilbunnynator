
#include "uci.hpp"

#include <iostream>
#include <sstream>

namespace {

// "e2e4" & " e2e4 " & "e2e4/r" (/n is omitted when using getline)
std::vector<std::string> split(const std::string& line) {
  std::vector<std::string> parts;
  std::istringstream ss{line};
  std::string token;

  while (ss >> token) {
    parts.push_back(token);
  }

  return parts;
}

bool parse_int(const std::string& s, int64_t& out) {
  try {
    out = std::stoll(s);
    return true;
  } catch (const std::exception&) {
    return false;
  }
}

}  // namespace

void UCI::loop() {
  std::string line;
  while (std::getline(std::cin, line))
    if (!handle_command(line)) break;
}

bool UCI::handle_command(const std::string& line) {
  const std::vector<std::string> parts = split(line);
  if (parts.empty()) return true;  // blank line != error

  const std::string& command = parts[0];
  const std::vector<std::string> args(parts.begin() + 1, parts.end());

  if (command == "uci") {
    handle_uci();
  } else if (command == "isready") {
    write("readyok");
  } else if (command == "quit") {
    return false;
  }

  return true;
}

void UCI::handle_position(const std::vector<std::string>& args) {
  if (args.empty()) return;

  size_t move_idx = args.size();
  for (int i = 0; i < move_idx; ++i) {
    if (args[i] == "moves") {
      move_idx = i;
      break;
    }
  }

  if (args[0] == "startpos") {
    engine.board = Position::startpos();
  } else if (args[0] == "fen") {
    if (move_idx - 1 != 6) {
      write("wrong fen size");
      return;
    }
    std::string fen;
    for (size_t i = 1; i < move_idx; ++i) {
      if (i > 1) fen += ' ';
      fen += args[i];
    }
    engine.board = Position::from_fen(fen);
  } else {
    return;
  }

  for (size_t i = move_idx + 1; i < args.size(); ++i) {
    const Move move = legal_move_from_uci(args[i]);
    if (move == MOVE_NONE) {
      write("info string Failed to parse or process move: " + args[i]);
      return;
    }
    engine.board.make_move(move);
  }
}

void UCI::write(const std::string& text) { std::cout << text << std::endl; }

void UCI::handle_uci() {
  write("id name bunnynator");
  write("id author superbunnylover58 on instagram");
  write("uciok");
}