// Cheks is this straight
#include "../core.hpp"
#include <array>
#include <vector>

std::vector<std::array<const Card *, 5>> find_straight(
    const std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> &hand,
    std::size_t threshold = 5) {
  std::vector<std::array<const Card *, 5>> result;
  std::array<const Card *, 5> straight{};
  std::size_t count = 0;

  // The last iteration closes a straight that ends at Ace.
  for (size_t i = 0; i <= hand.size(); ++i) {
    if (i < hand.size() && !hand[i].empty()) {
      if (count < straight.size()) {
        straight[count] = hand[i][0];
      }
      ++count;
      continue;
    }

    if (count >= threshold) {
      result.push_back(straight);
    }

    straight = {};
    count = 0;
  }

  return result;
}
