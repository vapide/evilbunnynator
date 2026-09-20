#include "engine.hpp"

#include <iostream>

#include "../movegen/movegen.hpp"

void Engine::join_search_thread() {
  if (search_thread.joinable()) search_thread.join();
}

void Engine::stop_search() {
  stop_requested.store(true, std::memory_order_release);
  {
    // without this, in theory, a stop command in the window will be lost and go
    // infinite will never return
    std::lock_guard<std::mutex> lock(wait_mutex);
  }
  wait_cv.notify_all();
}

void Engine::think_async(const SearchLimits& limits,
                         std::function<void(Move)> on_complete) {
  // one at a time
  if (is_searching()) return;

  join_search_thread();
  stop_requested.store(false, std::memory_order_release);
  searching.store(true, std::memory_order_release);

  search_thread =
      std::thread(&Engine::think, this, limits, std::move(on_complete));
}

void Engine::think(SearchLimits limits, std::function<void(Move)> on_complete) {
  Move chosen = MOVE_NONE;

  // temporary placeholder which returns a random move until I implement search
  // fully or at least partially
  Move moves[MAX_MOVES];
  int count = MoveGen::generate_legal_moves(board, moves);
  rng.set_seed(std::chrono::steady_clock::now().time_since_epoch().count());
  if (count != 0) {
    U64 num = rng.rand64() % count;
    chosen = moves[num];
  } else {
    chosen = 0000;  // static_cast<Move>(0000);
  }

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