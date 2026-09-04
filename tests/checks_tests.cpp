#include "../checks.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

using CardsByRank =
    std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()>;
using CardsBySuit = std::array<std::vector<const Card *>, ALL_SUITS.size()>;

CardsByRank group_by_rank(const std::vector<Card> &cards) {
  CardsByRank hand;

  for (const Card &card : cards) {
    hand[static_cast<std::size_t>(card.rank)].push_back(&card);
  }

  return hand;
}

CardsBySuit group_by_suit(const std::vector<Card> &cards) {
  CardsBySuit hand;

  for (const Card &card : cards) {
    hand[static_cast<std::size_t>(card.suit)].push_back(&card);
  }

  return hand;
}

void test_pairs() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO},
      {Suit::CLUBS, CardRank::TWO},
      {Suit::SPADES, CardRank::NINE},
  };

  const auto pairs = find_pairs(group_by_rank(cards));

  assert(pairs.size() == 1);
  assert(pairs[0][0] == &cards[0]);
  assert(pairs[0][1] == &cards[1]);
}

void test_triples() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::KING},
      {Suit::CLUBS, CardRank::KING},
      {Suit::SPADES, CardRank::KING},
      {Suit::DIAMONDS, CardRank::TWO},
  };

  const auto triples = find_triples(group_by_rank(cards));

  assert(triples.size() == 1);
  assert(triples[0][0] == &cards[0]);
  assert(triples[0][1] == &cards[1]);
  assert(triples[0][2] == &cards[2]);
}

void test_four_card_flush() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO}, {Suit::HEARTS, CardRank::FOUR},
      {Suit::HEARTS, CardRank::SIX}, {Suit::HEARTS, CardRank::EIGHT},
      {Suit::CLUBS, CardRank::TEN},
  };

  const auto flushes = find_flush(group_by_suit(cards), 4);

  assert(flushes.size() == 1);
  for (std::size_t i = 0; i < 4; ++i) {
    assert(flushes[0][i] == &cards[i]);
  }
  assert(flushes[0][4] == nullptr);
  assert(find_flush(group_by_suit(cards), 5).empty());
}

void test_five_card_flush() {
  const std::vector<Card> cards = {
      {Suit::SPADES, CardRank::TWO}, {Suit::SPADES, CardRank::FOUR},
      {Suit::SPADES, CardRank::SIX}, {Suit::SPADES, CardRank::EIGHT},
      {Suit::SPADES, CardRank::TEN},
  };

  const auto flushes = find_flush(group_by_suit(cards));

  assert(flushes.size() == 1);
  for (std::size_t i = 0; i < 5; ++i) {
    assert(flushes[0][i] == &cards[i]);
  }
}

void test_four_card_straight() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO},
      {Suit::CLUBS, CardRank::THREE},
      {Suit::SPADES, CardRank::FOUR},
      {Suit::DIAMONDS, CardRank::FIVE},
  };

  const auto straights = find_straight(group_by_rank(cards), 4);

  assert(straights.size() == 1);
  for (std::size_t i = 0; i < 4; ++i) {
    assert(straights[0][i] == &cards[i]);
  }
  assert(straights[0][4] == nullptr);
  assert(find_straight(group_by_rank(cards), 5).empty());
}

void test_five_card_straight_ending_at_ace() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TEN},   {Suit::CLUBS, CardRank::JACK},
      {Suit::SPADES, CardRank::QUEEN}, {Suit::DIAMONDS, CardRank::KING},
      {Suit::HEARTS, CardRank::ACE},
  };

  const auto straights = find_straight(group_by_rank(cards));

  assert(straights.size() == 1);
  for (std::size_t i = 0; i < 5; ++i) {
    assert(straights[0][i] == &cards[i]);
  }
}

void test_full_house() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO},  {Suit::CLUBS, CardRank::TWO},
      {Suit::HEARTS, CardRank::KING}, {Suit::CLUBS, CardRank::KING},
      {Suit::SPADES, CardRank::KING},
  };
  const auto hand = group_by_rank(cards);
  const auto full_houses =
      find_full_house(find_pairs(hand), find_triples(hand));

  assert(full_houses.size() == 1);
  for (std::size_t i = 0; i < 5; ++i) {
    assert(full_houses[0][i] == &cards[i]);
  }
}

int main() {
  test_pairs();
  test_triples();
  test_four_card_flush();
  test_five_card_flush();
  test_four_card_straight();
  test_five_card_straight_ending_at_ace();
  test_full_house();

  std::cout << "All checks tests passed.\n";
}
