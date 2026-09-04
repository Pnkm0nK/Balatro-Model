#include "../checks.hpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <sstream>
#include <string>
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

std::string format_card(const Card *card) {
  if (card == nullptr) {
    return "null";
  }

  constexpr std::array rank_names = {"2", "3", "4", "5", "6", "7", "8",
                                     "9", "10", "J", "Q", "K", "A"};
  constexpr std::array suit_names = {"H", "D", "C", "S"};
  return std::string(rank_names[static_cast<std::size_t>(card->rank)]) +
         suit_names[static_cast<std::size_t>(card->suit)];
}

std::string format_input(const std::vector<Card> &cards) {
  std::ostringstream output;
  output << "[";
  for (std::size_t i = 0; i < cards.size(); ++i) {
    if (i != 0) {
      output << ", ";
    }
    output << format_card(&cards[i]);
  }
  return output.str() + "]";
}

template <std::size_t N>
std::string format_output(const std::vector<std::array<const Card *, N>> &hands) {
  std::ostringstream output;
  output << "[";
  for (std::size_t hand_index = 0; hand_index < hands.size(); ++hand_index) {
    if (hand_index != 0) {
      output << ", ";
    }
    output << "[";
    for (std::size_t card_index = 0; card_index < N; ++card_index) {
      if (card_index != 0) {
        output << ", ";
      }
      output << format_card(hands[hand_index][card_index]);
    }
    output << "]";
  }
  return output.str() + "]";
}

template <std::size_t N>
void print_test_result(const std::string &name, const std::vector<Card> &input,
                       const std::vector<std::array<const Card *, N>> &output,
                       const std::vector<std::array<const Card *, N>> &expected,
                       bool passed) {
  std::cout << "========================================\n"
            << "Test name: " << name << "\n"
            << "Input: " << format_input(input) << "\n"
            << "Output: " << format_output(output) << "\n"
            << "Correct output: " << format_output(expected) << "\n"
            << "Verdict: " << (passed ? "PASS" : "FAILED") << "\n";
}

void test_pairs() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO},
      {Suit::CLUBS, CardRank::TWO},
      {Suit::SPADES, CardRank::NINE},
  };

  const auto pairs = find_pairs(group_by_rank(cards));
  const std::vector<std::array<const Card *, 2>> expected = {
      {&cards[0], &cards[1]}};
  const bool passed = pairs == expected;

  print_test_result("pairs", cards, pairs, expected, passed);
  assert(passed);
}

void test_two_pairs() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO},
      {Suit::CLUBS, CardRank::TWO},
      {Suit::SPADES, CardRank::KING},
      {Suit::DIAMONDS, CardRank::KING},
      {Suit::HEARTS, CardRank::NINE},
  };

  const auto pairs = find_pairs(group_by_rank(cards));
  const std::vector<std::array<const Card *, 2>> expected = {
      {&cards[0], &cards[1]}, {&cards[2], &cards[3]}};
  const bool passed = pairs == expected;

  print_test_result("two pairs", cards, pairs, expected, passed);
  assert(passed);
}

void test_three_of_a_kind() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::KING},
      {Suit::CLUBS, CardRank::KING},
      {Suit::SPADES, CardRank::KING},
      {Suit::DIAMONDS, CardRank::TWO},
  };

  const auto three_of_a_kind = find_three_of_a_kind(group_by_rank(cards));
  const std::vector<std::array<const Card *, 3>> expected = {
      {&cards[0], &cards[1], &cards[2]}};
  const bool passed = three_of_a_kind == expected;

  print_test_result("three of a kind", cards, three_of_a_kind, expected, passed);
  assert(passed);
}

void test_four_of_a_kind() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::ACE},
      {Suit::CLUBS, CardRank::ACE},
      {Suit::SPADES, CardRank::ACE},
      {Suit::DIAMONDS, CardRank::ACE},
      {Suit::HEARTS, CardRank::TWO},
  };

  const auto four_of_a_kind = find_four_of_a_kind(group_by_rank(cards));
  const std::vector<std::array<const Card *, 4>> expected = {
      {&cards[0], &cards[1], &cards[2], &cards[3]}};
  const bool passed = four_of_a_kind == expected;

  print_test_result("four of a kind", cards, four_of_a_kind, expected, passed);
  assert(passed);
}

void test_four_card_flush() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO}, {Suit::HEARTS, CardRank::FOUR},
      {Suit::HEARTS, CardRank::SIX}, {Suit::HEARTS, CardRank::EIGHT},
      {Suit::CLUBS, CardRank::TEN},
  };

  const auto flushes = find_flush(group_by_suit(cards), 4);
  const std::vector<std::array<const Card *, 5>> expected = {
      {&cards[0], &cards[1], &cards[2], &cards[3], nullptr}};
  const bool passed =
      flushes == expected && find_flush(group_by_suit(cards), 5).empty();

  print_test_result("four-card flush", cards, flushes, expected, passed);
  assert(passed);
}

void test_five_card_flush() {
  const std::vector<Card> cards = {
      {Suit::SPADES, CardRank::TWO}, {Suit::SPADES, CardRank::FOUR},
      {Suit::SPADES, CardRank::SIX}, {Suit::SPADES, CardRank::EIGHT},
      {Suit::SPADES, CardRank::TEN},
  };

  const auto flushes = find_flush(group_by_suit(cards));
  const std::vector<std::array<const Card *, 5>> expected = {
      {&cards[0], &cards[1], &cards[2], &cards[3], &cards[4]}};
  const bool passed = flushes == expected;

  print_test_result("five-card flush", cards, flushes, expected, passed);
  assert(passed);
}

void test_royal_flush() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::ACE},   {Suit::HEARTS, CardRank::TEN},
      {Suit::HEARTS, CardRank::KING},  {Suit::HEARTS, CardRank::JACK},
      {Suit::HEARTS, CardRank::QUEEN}, {Suit::CLUBS, CardRank::ACE},
  };

  const auto royal_flushes = find_royal_flush(group_by_suit(cards));
  const std::vector<std::array<const Card *, 5>> expected = {
      {&cards[1], &cards[3], &cards[4], &cards[2], &cards[0]}};
  const bool passed = royal_flushes == expected;

  print_test_result("royal flush", cards, royal_flushes, expected, passed);
  assert(passed);
}

void test_four_card_straight() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO},
      {Suit::CLUBS, CardRank::THREE},
      {Suit::SPADES, CardRank::FOUR},
      {Suit::DIAMONDS, CardRank::FIVE},
  };

  const auto straights = find_straight(group_by_rank(cards), 4);
  const std::vector<std::array<const Card *, 5>> expected = {
      {&cards[0], &cards[1], &cards[2], &cards[3], nullptr}};
  const bool passed = straights == expected &&
                      find_straight(group_by_rank(cards), 5).empty();

  print_test_result("four-card straight", cards, straights, expected, passed);
  assert(passed);
}

void test_five_card_straight_ending_at_ace() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TEN},   {Suit::CLUBS, CardRank::JACK},
      {Suit::SPADES, CardRank::QUEEN}, {Suit::DIAMONDS, CardRank::KING},
      {Suit::HEARTS, CardRank::ACE},
  };

  const auto straights = find_straight(group_by_rank(cards));
  const std::vector<std::array<const Card *, 5>> expected = {
      {&cards[0], &cards[1], &cards[2], &cards[3], &cards[4]}};
  const bool passed = straights == expected;

  print_test_result("five-card straight ending at ace", cards, straights, expected,
                    passed);
  assert(passed);
}

void test_full_house() {
  const std::vector<Card> cards = {
      {Suit::HEARTS, CardRank::TWO},  {Suit::CLUBS, CardRank::TWO},
      {Suit::HEARTS, CardRank::KING}, {Suit::CLUBS, CardRank::KING},
      {Suit::SPADES, CardRank::KING},
  };
  const auto hand = group_by_rank(cards);
  const auto full_houses =
      find_full_house(find_pairs(hand), find_three_of_a_kind(hand));
  const std::vector<std::array<const Card *, 5>> expected = {
      {&cards[0], &cards[1], &cards[2], &cards[3], &cards[4]}};
  const bool passed = full_houses == expected;

  print_test_result("full house", cards, full_houses, expected, passed);
  assert(passed);
}

int main() {
  test_pairs();
  test_two_pairs();
  test_three_of_a_kind();
  test_four_of_a_kind();
  test_four_card_flush();
  test_five_card_flush();
  test_royal_flush();
  test_four_card_straight();
  test_five_card_straight_ending_at_ace();
  test_full_house();

  std::cout << "All checks tests passed.\n";
}
