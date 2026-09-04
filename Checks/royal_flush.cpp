#include "../checks.hpp"
#include <algorithm>
#include <array>
#include <vector>

std::vector<std::array<const Card *, 5>> find_royal_flush(
    const std::array<std::vector<const Card *>, ALL_SUITS.size()> &hand) {
  std::vector<std::array<const Card *, 5>> result;
  constexpr auto first_royal_rank = static_cast<std::size_t>(CardRank::TEN);

  for (const auto &suited_cards : hand) {
    std::array<const Card *, 5> royal_flush{};

    for (const Card *card : suited_cards) {
      const auto rank = static_cast<std::size_t>(card->rank);
      if (rank >= first_royal_rank && rank < first_royal_rank + royal_flush.size()) {
        royal_flush[rank - first_royal_rank] = card;
      }
    }

    if (std::all_of(royal_flush.begin(), royal_flush.end(),
                    [](const Card *card) { return card != nullptr; })) {
      result.push_back(royal_flush);
    }
  }

  return result;
}
