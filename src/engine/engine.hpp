#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <thread>

#include "../core/position.hpp"
#include "../search/limits.hpp"
#include "../search/search.hpp"

class Engine {
 public:
  Engine();

  ~Engine();

  std::unique_ptr<Search> search;

  uint64_t perft(int depth);
  uint64_t perft_divide(int depth);

  void new_game();

  Position board;

  void set_info_sink(std::function<void(const SearchInfo&)> sink);

  void set_position(const Position& p);

  void think_async(const SearchLimits& limits,
                   std::function<void(Move)> on_complete);
  bool is_searching() const {
    return searching.load(std::memory_order_acquire);
  }
  void stop_search();

 private:
  std::function<void(const SearchInfo&)> search_on_info;

  void think(SearchLimits limits, std::function<void(Move)> on_complete);

  void join_search_thread();
  std::thread search_thread;
  std::atomic<bool> searching{false};
  std::atomic<bool> stop_requested{false};

  std::mutex wait_mutex;
  std::condition_variable wait_cv;
};
