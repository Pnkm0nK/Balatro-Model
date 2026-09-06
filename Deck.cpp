#include "core.hpp"
#include "types.hpp"
#include <algorithm>
#include <random>
#include <vector>

Deck::Deck() {
  deck.reserve(ALL_SUITS.size() * ALL_CARD_RANKS.size());
  for (Suit suit : ALL_SUITS) {
    for (CardRank rank : ALL_CARD_RANKS) {
      deck.emplace_back(suit, rank);
    }
  }
  shuffle();
}

void Deck::shuffle() {
  static std::mt19937 generator(std::random_device{}());
  std::shuffle(deck.begin(), deck.end(), generator);
}

std::vector<Card> Deck::deal(int cards_in_hand, int max_cards_in_hand) {
  shuffle();

  int cards_to_deal = max_cards_in_hand - cards_in_hand;
  std::vector<Card> dealt_cards;
  while (cards_to_deal > 0 && deck.size() > 0) {
    dealt_cards.push_back(deck.back());
    deck.pop_back();
    cards_to_deal--;
  }
  return dealt_cards;
}

void Deck::collect(const Card &card) {
  used_cards.push_back(card);
}

void Deck::finish_round(std::vector<Card> &hand) {
  // Return the current card values, including permanent Tarot changes.
  deck.insert(deck.end(), used_cards.begin(), used_cards.end());
  deck.insert(deck.end(), hand.begin(), hand.end());
  used_cards.clear();
  hand.clear();
  shuffle();
}
