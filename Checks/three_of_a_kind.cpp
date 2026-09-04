// Finds three of a kind
#include "../core.hpp"
#include <array>
#include <vector>

std::vector<std::array<const Card *, 3>> find_three_of_a_kind(
    const std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> &hand) {
  std::vector<std::array<const Card *, 3>> result;

  for (size_t i = 0; i < hand.size(); ++i) {
    if (hand[i].size() == 3) {
      result.push_back({hand[i][0], hand[i][1], hand[i][2]});
    }
  }

  return result;
}
