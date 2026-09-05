#pragma once

#include "types.hpp"
#include <array>
#include <cstddef>
#include <vector>

inline constexpr std::array<int, 13> base_chips = {2, 3,  4,  5,  6,  7, 8,
                                                   9, 10, 10, 10, 10, 11};
inline constexpr std::array<int, 12> hand_chips = {5,  10, 20,  30,  30,  35,
                                                   40, 60, 100, 120, 140, 160};
inline constexpr std::array<int, 13> hand_mult = {1, 2, 2, 3,  4,  4, 4,
                                                  7, 8, 8, 12, 14, 16};
inline constexpr std::array<int, 4> gold_per_blind = {3, 4, 5, 8};
inline constexpr std::array<int, 9> ante_base_chips = {
    100, 300, 800, 2000, 5000, 11000, 20000, 35000, 50000};
inline constexpr std::array<double, 3> blind_multipliers = {1.0, 1.5, 2.0};

struct Card {
  Suit suit;
  CardRank rank;
  Edition edition;
  Enhancement enhancement;
  Seal seal;
  int chips;

  Card(Suit suit, CardRank rank, Edition edition = Edition::BASE,
       Enhancement enhancement = Enhancement::NONE, Seal seal = Seal::NONE);
};

class Deck {
public:
  std::vector<Card> deck;

  Deck();

  void shuffle();

  std::vector<Card> deal(int cards_in_hand, int max_cards_in_hand);
};

struct HandEval {
  std::vector<const Card *> cards_scored;
  HandType hand_type;
};

class GameState {
public:
  Deck deck;
  int ante;
  int money;
  int round;
  int max_cards_in_hand;
  int max_cards_played;
  int hands;
  int discards;
  int hands_left;
  int discards_left;
  Blind cur_blind;
  unsigned long long round_score;
  std::array<double, 3> blind_score_reqs;
  double cur_blind_score_req;

  GameState(int money = 4, int hands = 4, int discards = 4);

  std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()>
  get_cards_by_rank(const std::vector<Card> &hand);

  std::array<std::vector<const Card *>, ALL_SUITS.size()>
  get_cards_by_suit(const std::vector<Card> &hand);

  HandEval evaluate_hand(const std::vector<Card> &hand);

  void play_hand(const std::vector<Card> &hand);

  std::vector<Card> discard(std::vector<Card> hand,
                            std::vector<std::size_t> chosen_card_indices);

  bool lose_round();

  void start_new_round();

  void win_round();
};
