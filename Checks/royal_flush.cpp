#include "../checks.hpp"
#include <array>
#include <vector>

std::vector<std::array<const Card *, 5>> find_royal_flush(
    const std::array<std::vector<const Card *>, ALL_SUITS.size()> &hand) {
  std::vector<std::array<const Card *, 5>> result;

  for (const auto &flush : find_flush(hand)) {
    std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> cards_by_rank;

    for (const Card *card : flush) {
      if (card != nullptr) {
        cards_by_rank[static_cast<std::size_t>(card->rank)].push_back(card);
      }
    }

    for (const auto &straight : find_straight(cards_by_rank)) {
      if (straight.back()->rank == CardRank::ACE) {
        result.push_back(straight);
      }
    }
  }

  return result;
}
