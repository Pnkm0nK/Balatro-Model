#include "../core.hpp"

std::vector<std::array<const Card *, 5>>
find_flush_five(const std::vector<Card> hand) {
  std::vector<std::array<const Card *, 5>> flush_fives;
  for (int i = 0; i < 4; i++) {
    if (hand[i].suit != hand[i + 1].suit || hand[i].rank != hand[i + 1].rank) {
      return flush_fives;
    }
  }
  flush_fives.push_back({&hand[0], &hand[1], &hand[2], &hand[3], &hand[4]});
  return flush_fives;
}