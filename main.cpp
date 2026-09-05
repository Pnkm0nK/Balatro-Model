#include "checks.hpp"
#include "core.hpp"
#include "types.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <stdexcept>
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

HandEval GameState::evaluate_hand(const std::vector<Card> &hand) {
  if (hand.empty()) {
    throw std::invalid_argument("Cannot evaluate hand: hand is empty");
  }

  // find all necessary combos
  auto cards_by_rank = get_cards_by_rank(hand);
  auto cards_by_suit = get_cards_by_suit(hand);
  auto pairs = find_pairs(cards_by_rank);
  auto triples = find_three_of_a_kind(cards_by_rank);
  auto fours = find_four_of_a_kind(cards_by_rank);
  auto fives = find_five_of_a_kind(cards_by_rank);
  auto flushes = find_flush(cards_by_suit);
  auto straights = find_straight(cards_by_rank);
  // cascade top-down for best hand and check interactions of combos
  // to decide on the hand type

  // flush five
  if (!flushes.empty() && !fives.empty()) {
    return {{fives[0].begin(), fives[0].end()}, HandType::FLUSH_FIVE};
  }
  auto full_houses = find_full_house(pairs, triples);

  // flush_house
  if (!flushes.empty() && !full_houses.empty()) {
    return {{full_houses[0].begin(), full_houses[0].end()},
            HandType::FLUSH_HOUSE};
  }

  // five of a kind
  if (!fives.empty()) {
    return {{fives[0].begin(), fives[0].end()}, HandType::FIVE_OF_A_KIND};
  }

  // royal flush
  auto royal_flushes = find_royal_flush(cards_by_suit);
  if (!royal_flushes.empty()) {
    return {{royal_flushes[0].begin(), royal_flushes[0].end()},
            HandType::ROYAL_FLUSH};
  }

  // straight flush
  if (!straights.empty() && !flushes.empty()) {
    return {{flushes[0].begin(), flushes[0].end()}, HandType::STRAIGHT_FLUSH};
  }

  // four of a kind
  if (!fours.empty()) {
    return {{fours[0].begin(), fours[0].end()}, HandType::FOUR_OF_A_KIND};
  }

  // full house
  if (!full_houses.empty()) {
    return {{full_houses[0].begin(), full_houses[0].end()},
            HandType::FULL_HOUSE};
  }

  // flush
  if (!flushes.empty()) {
    return {{flushes[0].begin(), flushes[0].end()}, HandType::FLUSH};
  }

  // straight
  if (!straights.empty()) {
    return {{straights[0].begin(), straights[0].end()}, HandType::STRAIGHT};
  }

  // three of a kind
  if (!triples.empty()) {
    return {{triples[0].begin(), triples[0].end()}, HandType::THREE_OF_A_KIND};
  }

  // two pair
  if (pairs.size() == 2) {
    return {{pairs[0][0], pairs[0][1], pairs[1][0], pairs[1][1]},
            HandType::TWO_PAIR};
  }

  // pair
  if (!pairs.empty()) {
    return {{pairs[0].begin(), pairs[0].end()}, HandType::PAIR};
  }

  // high card: scan from Ace down to Two
  for (int i = static_cast<int>(ALL_CARD_RANKS.size()) - 1; i >= 0; --i) {
    if (!cards_by_rank[i].empty()) {
      return {{cards_by_rank[i][0]}, HandType::HIGH_CARD};
    }
  }

  throw std::logic_error(
      "Internal error: high card not found in non-empty hand");
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
  const auto &[cards_scored, hand_type] = evaluate_hand(hand);
  uint chips = hand_chips[static_cast<size_t>(hand_type)];
  uint mult = hand_mult[static_cast<size_t>(hand_type)];
  for (const Card &card : hand) {
    bool is_scoring = std::find(cards_scored.begin(), cards_scored.end(),
                                &card) != cards_scored.end();
    if (is_scoring) {
      chips += card.chips;
    }
  }
  this->round_score += chips * mult;
  this->hands_left--;
}

std::vector<Card> GameState::discard(
    std::vector<Card> hand,
  std::vector<std::size_t> chosen_card_indices) {
  std::sort(chosen_card_indices.rbegin(), chosen_card_indices.rend());

  for (const std::size_t index : chosen_card_indices) {
    hand.erase(hand.begin() + index);
  }

  return hand;
}

    void GameState::start_new_round() {
  bool round_end = false;
  while (!round_end) {
    std::vector<Card> hand = deck.deal(0, max_cards_in_hand);
    // TODO: choose cards to play
    std::vector<Card> chosen_cards = {hand[0], hand[1], hand[2]};

    // TODO: deal with discards

    play_hand(chosen_cards);
    if (this->round_score >= cur_blind_score_req) {
      round_end = true;
      this->round++;
    }
  }

  int main() {
    // deck.deal();
    return 0;
  }
