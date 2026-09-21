#include "engine.hpp"

#include <iostream>

#include "../movegen/perft.hpp"

Engine::Engine() : search(std::make_unique<Search>()) {
  board = Position::startpos();
}

Engine::~Engine() {
  stop_search();
  join_search_thread();
}

void Engine::join_search_thread() {
  if (search_thread.joinable()) search_thread.join();
}

void Engine::stop_search() {
  stop_requested.store(true, std::memory_order_release);
  search->stop_search();
  {
    // without this, in theory, a stop command in the window will be lost and go
    // infinite will never return
    std::lock_guard<std::mutex> lock(wait_mutex);
  }
  wait_cv.notify_all();
}

void Engine::set_info_sink(std::function<void(const SearchInfo&)> sink) {
  search->on_info = std::move(sink);
}

void Engine::think_async(const SearchLimits& limits,
                         std::function<void(Move)> on_complete) {
  // one at a time
  if (is_searching()) return;

  join_search_thread();
  stop_requested.store(false, std::memory_order_release);
  search->reset();
  searching.store(true, std::memory_order_release);

  search_thread =
      std::thread(&Engine::think, this, limits, std::move(on_complete));
}

void Engine::new_game() {
  stop_search();
  join_search_thread();
  board = Position::startpos();
  search->reset();
}

uint64_t Engine::perft(int depth) {
  if (depth < 0) return 0;
  return Perft::perft(board, depth);
}

uint64_t Engine::perft_divide(int depth) {
  if (depth < 0) return 0;
  return Perft::divide(board, depth);
}

void Engine::set_position(const Position& p) {
  stop_search();
  join_search_thread();
  board = p;
}

void Engine::think(SearchLimits limits, std::function<void(Move)> on_complete) {
  const Move chosen = search->find_best_move(board, limits);

  // inifinite waits for a stop, movetime waits
  //  until time or until stop-whichever is first. anything else returns at once
  //  because there is no depth or node count implemented by me yet.
  if (limits.infinite) {
    std::unique_lock<std::mutex> lock(wait_mutex);
    wait_cv.wait(lock, [this] {
      return stop_requested.load(std::memory_order_acquire);
    });
  } else if (limits.movetime > 0) {
    std::unique_lock<std::mutex> lock(wait_mutex);
    wait_cv.wait_for(lock, std::chrono::milliseconds(limits.movetime), [this] {
      return stop_requested.load(std::memory_order_acquire);
    });
  }
  // clear flag before callback because there have been known issues with
  // guis sending "go" immediately after it reads bestmove.
  searching.store(false, std::memory_order_release);
  on_complete(chosen);
}