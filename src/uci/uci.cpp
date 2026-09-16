
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

void UCI::write(const std::string& text) { std::cout << text << std::endl; }

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
    std::cout << "id name bunnynator" << std::endl;
    std::cout << "id author superbunnylover58 on instagram" << std::endl;
    std::cout << std::endl;

    // uci option list

    // ok response
    std::cout << "uciok";
  } else if (command == "isready") {
    std::cout << "readyok" << std::endl;
  } else if (command == "quit") {
    return false;
  }

  return true;
}
