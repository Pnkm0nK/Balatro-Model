#include "core.hpp"
#include "types.hpp"
#include <vector>

Deck::Deck() {
  deck.reserve(ALL_SUITS.size() * ALL_CARD_RANKS.size());
  for (Suit suit : ALL_SUITS) {
    for (CardRank rank : ALL_CARD_RANKS) {
      deck.emplace_back(suit, rank);
    }
  }
}
