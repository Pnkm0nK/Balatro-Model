#pragma once
#include <array>
#include <cstdint>

enum class CardRank : uint8_t {
  TWO,
  THREE,
  FOUR,
  FIVE,
  SIX,
  SEVEN,
  EIGHT,
  NINE,
  TEN,
  JACK,
  QUEEN,
  KING,
  ACE
};

enum class HandType : uint8_t {
  HIGH_CARD,
  PAIR,
  TWO_PAIR,
  THREE_OF_A_KIND,
  STRAIGHT,
  FLUSH,
  FULL_HOUSE,
  FOUR_OF_A_KIND,
  STRAIGHT_FLUSH, // straight + flush
  ROYAL_FLUSH,    // ace high straight flush
  FIVE_OF_A_KIND,
  FLUSH_HOUSE, // Single suit. 3 cards are same rank and other 2 are same rank.
  FLUSH_FIVE   // All cards are the same
};

enum class Edition : uint8_t { BASE, FOIL, POLYCHROME, HOLOGRAPHIC, NEGATIVE };

enum class Enhancement : uint8_t {
  NONE,
  BONUS,
  MULT,
  WILD,
  STEEL,
  GOLD,
  STONE,
  LUCKY
};

enum class Seal : uint8_t { NONE, RED, GOLD, BLUE, PURPLE };

enum class Suit : uint8_t { HEARTS, DIAMONDS, CLUBS, SPADES };

inline constexpr std::array<CardRank, 13> ALL_CARD_RANKS = {
    CardRank::TWO, CardRank::THREE, CardRank::FOUR,  CardRank::FIVE,
    CardRank::SIX, CardRank::SEVEN, CardRank::EIGHT, CardRank::NINE,
    CardRank::TEN, CardRank::JACK,  CardRank::QUEEN, CardRank::KING,
    CardRank::ACE};

inline constexpr std::array<Suit, 4> ALL_SUITS = {Suit::HEARTS, Suit::DIAMONDS,
                                                  Suit::CLUBS, Suit::SPADES};

enum class Blind : uint8_t { SMALL, BIG, BOSS };

inline constexpr std::array<Blind, 3> ALL_BLINDS = {Blind::SMALL, Blind::BIG,
                                                    Blind::BOSS};

inline Blind next_blind(Blind blind) {
  return static_cast<Blind>((static_cast<uint8_t>(blind) + 1) %
                            ALL_BLINDS.size());
}
