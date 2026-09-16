
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
    // id
    write("id name bunnynator");
    write("id author superbunnylover58 on instagram");

    // uci option list

    // ok response
    write("uciok");
  } else if (command == "isready") {
    write("readyok");
  } else if (command == "quit") {
    return false;
  }

  return true;
}

void UCI::write(const std::string& text) { std::cout << text << std::endl; }