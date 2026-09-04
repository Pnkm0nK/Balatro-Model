#include "Deck.cpp"
#include "types.cpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

constexpr std::array<int, 12> hand_chips = {5,  10, 20,  30,  30,  35,
                                            40, 60, 100, 120, 140, 160};
constexpr std::array<int, 13> hand_mult = {1, 2, 2, 3,  4,  4, 4,
                                           7, 8, 8, 12, 14, 16};
class GameState {
public:
  Deck deck;
  int ante = 1;
  int money;
  int round = 0;
  int max_cards_in_hand = 8;
  int max_cards_played = 5;
  int hands_left;
  int discards_left;
  std::array<int, 9> ante_base_chips = {100,   300,   800,   2000, 5000,
                                        11000, 20000, 35000, 50000};
  std::array<double, 3> blind_multipliers = {1.0, 1.5, 2.0};
  Blind cur_blind = Blind::SMALL;
  int round_score = 0;
  std::array<double, 3> blind_score_reqs;
  double cur_blind_score_req;

  GameState(int money = 4, int hands_left = 4, int discards_left = 4)
      : money(money), hands_left(hands_left), discards_left(discards_left) {
    for (size_t i = 0; i < ALL_BLINDS.size(); ++i) {
      blind_score_reqs[i] = ante_base_chips[ante] * blind_multipliers[i];
    }
    cur_blind_score_req = blind_score_reqs[static_cast<size_t>(cur_blind)];
  }

  std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()>
  get_cards_by_rank(const std::vector<Card> &hand) {
    std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> ranked_hand;
    for (const Card &card : hand) {
      ranked_hand[static_cast<size_t>(card.rank)].push_back(&card);
    }
    return ranked_hand;
  }

  std::array<std::vector<const Card *>, ALL_SUITS.size()>
  get_cards_by_suit(const std::vector<Card> &hand) {
    std::array<std::vector<const Card *>, ALL_SUITS.size()> suit_hand;
    for (const Card &card : hand) {
      suit_hand[static_cast<size_t>(card.suit)].push_back(&card);
    }
    return suit_hand;
  }

  void play_hand(const std::vector<Card> &hand) {
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
  }
};

int main() {
  // deck.deal();
}
