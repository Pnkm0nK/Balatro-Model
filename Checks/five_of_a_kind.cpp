#include "../core.hpp"

std::vector<std::array<const Card *, 5>> find_five_of_a_kind(
    const std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()>
        &hand_by_rank) {
  std::vector<std::array<const Card *, 5>> five_of_a_kinds;
  for (int i; i < ALL_CARD_RANKS.size(); i++) {
    auto ranked = hand_by_rank[i];
    if (hand_by_rank[i].size() == 5) {
      five_of_a_kinds.push_back({
          ranked[0],
          ranked[1],
          ranked[2],
          ranked[3],
          ranked[4],
      });
    }
  }
  return five_of_a_kinds;
}
