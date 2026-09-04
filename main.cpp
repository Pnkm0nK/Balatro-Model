#include "checks.hpp"
#include "core.hpp"
#include "types.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

GameState::GameState(int money, int hands_left, int discards_left)
    : ante(1), money(money), round(0), max_cards_in_hand(8),
      max_cards_played(5), hands_left(hands_left), discards_left(discards_left),
      cur_blind(Blind::SMALL), round_score(0) {
  for (size_t i = 0; i < ALL_BLINDS.size(); ++i) {
    blind_score_reqs[i] = ante_base_chips[ante] * blind_multipliers[i];
  }
  cur_blind_score_req = blind_score_reqs[static_cast<size_t>(cur_blind)];
}

std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()>
GameState::get_cards_by_rank(const std::vector<Card> &hand) {
  std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> ranked_hand;
  for (const Card &card : hand) {
    ranked_hand[static_cast<size_t>(card.rank)].push_back(&card);
  }
  return ranked_hand;
}

std::array<std::vector<const Card *>, ALL_SUITS.size()>
GameState::get_cards_by_suit(const std::vector<Card> &hand) {
  std::array<std::vector<const Card *>, ALL_SUITS.size()> suit_hand;
  for (const Card &card : hand) {
    suit_hand[static_cast<size_t>(card.suit)].push_back(&card);
  }
  return suit_hand;
}

void GameState::play_hand(const std::vector<Card> &hand) {
  /*
  1. Pre-scoring
  Jokers that have hand modifiers get triggered from left to right.
  So, if you have a Vampire, this is where it will suck up the effects of the
  cards, or if you have a Runner and a Straight gets detected, increases its
  chips. Same goes with other Jokers like Ride the Bus, Square Joker, Green
  Joker, etc.
  */
  /*
  2. Dealt hand scoring
  our dealt hand is analysed left to right,
  adding the chips, bonus effects, multiplier effects, triggering glass cards,
  etc. Specific order:
  - 1. Adding Base Card Chips

  - 2. Triggering Own Card Effects: +Chips (either from Bonus as +30 chips, or
  from Hiker), Mult card (+4 Mult), Lucky (Chances of +20 mult or $20).

  - 3. Triggering Card Editions: Foil (+50 chips), Holographic (+10 Mult)
   or Polychrome (1.5 xMult).

  - 4. Triggering Joker Effects: Fibonacci, Photograph, Smiley.
  Greedy Joker adds +4 Mult on Diamonds, etc.

  - 5. Gold Seal, if card has one. It will give $3 after played.

  3. Effects in hand
  4. Joker scoring
  */
  int score = 0;
  HandType hand_type = HandType::HIGH_CARD;
  std::vector<std::vector<Card>> ranked_hand(ALL_CARD_RANKS.size());
  for (const auto &card : hand) {
    ranked_hand[static_cast<size_t>(card.rank)].push_back(card);
  }
  // find max number of cards with same rank
  auto max_it = std::max_element(
      ranked_hand.begin(), ranked_hand.end(),
      [](const std::vector<Card> &a, const std::vector<Card> &b) {
        return a.size() < b.size();
      });
  size_t max_index = std::distance(ranked_hand.begin(), max_it);
  int max_val = max_it->size();
  auto cards_by_rank = get_cards_by_rank(hand);
  auto cards_by_suit = get_cards_by_suit(hand);
  auto pairs = find_pairs(cards_by_rank);
  auto triples = find_triples(cards_by_rank);
  if (!pairs.empty()) {
    auto two_pair = find_two_pair(pairs) auto full_house =
        find_full_house(pairs, triples)
  }
}

int main() {
  // deck.deal();
}
