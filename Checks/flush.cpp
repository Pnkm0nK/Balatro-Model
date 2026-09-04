// Cheks is this flush
#include "../core.hpp"
#include <algorithm>
#include <array>
#include <vector>

std::vector<std::array<const Card *, 5>>
find_flush(const std::array<std::vector<const Card *>, ALL_SUITS.size()> &hand,
           std::size_t threshold = 5) {
  std::vector<std::array<const Card *, 5>> result;

  for (const auto &suited_cards : hand) {
    if (suited_cards.size() < threshold) {
      continue;
    }

    const auto flush_size = std::min<std::size_t>(suited_cards.size(), 5);

    std::array<const Card *, 5> flush{};
    std::copy_n(suited_cards.begin(), flush_size, flush.begin());
    result.push_back(flush);
  }

  return result;
}
