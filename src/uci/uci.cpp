
#include "uci.hpp"

#include <iostream>
#include <sstream>

#include "../movegen/movegen.hpp"

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

//
Move UCI::legal_move_from_uci(Position& pos, const std::string& text) {
  const Move parsed = from_uci(text);
  if (parsed == MOVE_NONE) return MOVE_NONE;

  Move moves[MAX_MOVES];
  const int count = MoveGen::generate_legal_moves(pos, moves);
  for (int i = 0; i < count; ++i)
    if (get_from(moves[i]) == get_from(parsed) &&
        get_to(moves[i]) == get_to(parsed) &&
        get_promotion(moves[i]) == get_promotion(parsed))
      return moves[i];
  return MOVE_NONE;
}

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
  } else if (command == "position") {
    handle_position(args);
  } else if (command == "isready") {
    write("readyok");
  } else if (command == "quit") {
    return false;
  }

  return true;
}

// clang-format off
bool valid_piece_char(char c)
{
    switch (c) {
        case 'P': case 'N': case 'B': case 'R': case 'Q': case 'K':
        case 'p': case 'n': case 'b': case 'r': case 'q': case 'k':
            return true;
        default:
            return false;
    }
}
// clang-format on

bool valid_placement(const std::string& placement) {
  int rank = 7, file = 0, kings[2] = {0, 0};
  for (const char c : placement) {
    if (c == '/') {
      if (file != 8 || rank == 0)
        return false;  // rank too short, too long, or too many ranks
      --rank;
      file = 0;
    } else if (c >= '1' && c <= '8') {
      file += c - '0';
      if (file > 8) return false;
    } else if (valid_piece_char(c)) {
      if (file > 7) return false;  // the write that would run off the rank
      if (c == 'K') ++kings[0];
      if (c == 'k') ++kings[1];
      ++file;
    } else {
      return false;  // char which is NO_PIECE
    }
  }
  if (rank != 0 || file != 8) return false;
  // there should be 1 king per side
  return kings[0] == 1 && kings[1] == 1;
}

bool valid_fen(const std::vector<std::string>& f) {
  if (f.size() != 6) return false;
  if (!valid_placement(f[0])) return false;
  if (f[1] != "w" && f[1] != "b") return false;

  // castling
  if (f[2] != "-") {
    if (f[2].size() > 4) return false;
    for (const char c : f[2])
      if (c != 'K' && c != 'Q' && c != 'k' && c != 'q') return false;
  }

  if (f[3] != "-") {
    if (f[3].size() != 2) return false;
    if (f[3][0] < 'a' || f[3][0] > 'h') return false;
    if (f[3][1] != '3' && f[3][1] != '6') return false;
  }

  int64_t halfmove = 0, fullmove = 0;
  if (!parse_int(f[4], halfmove) || halfmove < 0) return false;
  if (!parse_int(f[5], fullmove) || fullmove < 1) return false;
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
    std::vector<std::string> fenvec = args;
    fenvec.erase(fenvec.begin());
    fenvec.erase(fenvec.begin() + move_idx - 1, fenvec.end());

    /*
    for (int i = 0; i < fenvec.size(); ++i) {
      std::cout << fenvec[i] << "\n";
    }
    */

    std::string fen;

    // write("info string validating FEN");

    if (valid_fen(fenvec)) {
      // move_idx - 1 = 6 in a correct fen.
      for (size_t i = 0; i < move_idx - 2; ++i) {
        fen += fenvec[i] + ' ';
      }

      engine.board = Position::from_fen(fen);
      write("info string FEN loaded and validated");

      for (size_t i = move_idx + 1; i < args.size(); ++i) {
        engine.board.make_move(
            static_cast<Move>(legal_move_from_uci(engine.board, args[i])));
        // engine.board.pretty_print();
        // std::cout << engine.board.to_fen() << std::endl;
      }

      // engine.board.pretty_print();
      write("info string FEN: " + engine.board.to_fen());

    } else {
      write("info string Invalid FEN");
    }
    return;
  } else {
    return;
  }

  for (size_t i = move_idx + 1; i < args.size(); ++i) {
    const Move move = legal_move_from_uci(engine.board, args[i]);
    if (move == MOVE_NONE) {
      write("info string Failed to parse or process move: " + args[i]);
      return;
    }
    engine.board.make_move(move);
  }

  write("info string " + engine.board.to_fen());
}

void UCI::write(const std::string& text) { std::cout << text << std::endl; }

void UCI::handle_uci() {
  write("id name bunnynator");
  write("id author superbunnylover58 on instagram");
  write("uciok");
}