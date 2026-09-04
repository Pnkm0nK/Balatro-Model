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

std::vector<Card> Deck::deal(int cards_in_hand, int max_cards_in_hand) {
  int cards_to_deal = max_cards_in_hand - cards_in_hand;
  std::vector<Card> dealt_cards;
  while (cards_to_deal > 0 && deck.size() > 0) {
    dealt_cards.push_back(deck.back());
    deck.pop_back();
    cards_to_deal--;
  }
  return dealt_cards;
}
