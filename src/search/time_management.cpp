#include <algorithm>

#include "search.hpp"

// how long will this search take
void Search::compute_time_limits(const SearchLimits& lims) {
  hard_limit_ms = -1;
  if (lims.movetime >= 0) {
    hard_limit_ms = static_cast<double>(
        std::max<int64_t>(1, lims.movetime - MOVE_OVERHEAD_MS));
  }
}

// at every single individual node, has it taken compute_time_limmits long yet?
bool Search::check_time() {
  if (stop.load(std::memory_order_relaxed)) return true;
  if (hard_limit_ms >= 0 && elapsed_ms() >= hard_limit_ms) {
    stop.store(true, std::memory_order_relaxed);
    return true;
  }
  return false;
}
