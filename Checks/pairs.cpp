// Finds pairs
#include "../core.hpp"
#include <array>
#include <vector>

std::vector<std::array<const Card *, 2>>
find_pairs(const std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()>
               &hand_by_rank) {
  std::vector<std::array<const Card *, 2>> result;

  for (size_t i = 0; i < hand_by_rank.size(); ++i) {
    if (hand_by_rank[i].size() == 2) {
      result.push_back({hand_by_rank[i][0], hand_by_rank[i][1]});
    }
  }

  return result;
}
