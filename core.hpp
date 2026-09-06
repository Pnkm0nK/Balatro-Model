#pragma once
#include "types.hpp"
#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

inline constexpr std::array<int, 13> base_chips = {2, 3,  4,  5,  6,  7, 8,
                                                   9, 10, 10, 10, 10, 11};
// Royal flush uses the same base chips as straight flush.
inline constexpr std::array<int, 12> base_hand_chips = {
    5, 10, 20, 30, 30, 35, 40, 60, 100, 120, 140, 160};
inline constexpr std::array<int, 12> base_hand_mult = {1, 2, 2, 3,  4,  4,
                                                       4, 7, 8, 12, 14, 16};
inline constexpr std::array<int, 4> gold_per_blind = {3, 4, 5, 8};
inline constexpr std::array<int, 9> ante_base_chips = {
    100, 300, 800, 2000, 5000, 11000, 20000, 35000, 50000};
inline constexpr std::array<double, 3> blind_multipliers = {1.0, 1.5, 2.0};

inline constexpr std::array<int, 12> planet_chip_additions = {
    10, 15, 20, 20, 30, 15, 25, 30, 40, 35, 40, 50};
inline constexpr std::array<int, 12> planet_mult_additions = {1, 1, 1, 2, 3, 2,
                                                              2, 3, 4, 3, 4, 3};

class GameState;

enum class Context {
  ROUND,
  SHOP,
};

class Item {
public:
  std::string name;
  int buy_cost;
  int sell_price;

  virtual void activate(GameState &state);

  virtual ~Item() = default;

  // Non-copyable item types return nullptr until they implement cloning.
  virtual std::unique_ptr<Item> clone() const { return nullptr; }
};

enum class TarotType;

class TarotCard : public Item {
public:
  TarotType name;

  TarotCard(TarotType name, int buy_cost, int sell_price);

  void set_sell_price(int price);

  void activate(GameState &state) override;

  std::unique_ptr<Item> clone() const override;
};

class PlanetCard : public Item {
public:
  Planet planet_type;
  int chips_to_add;
  int mult_to_add;

  PlanetCard(Planet planet_type);

  void activate(GameState &game_state) override;

  std::unique_ptr<Item> clone() const override;
};

enum class JokerRarity { COMMON, UNCOMMON, RARE };

class Card : public Item {
public:
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

class Voucher {
public:
  int buy_cost;
  virtual ~Voucher() = default;
  virtual void activate(GameState &state) = 0;
};

struct ScoringContext {
  const HandEval &hand_eval;
  unsigned long long chips = 0;
  unsigned long long mult = 0;
};

class Joker : public Item {
public:
  JokerRarity rarity;

  virtual ~Joker() = 0;

  virtual void on_pre_score(const std::vector<Card> &played_cards,
                            HandEval &hand_eval, ScoringContext &ctx,
                            GameState &state) {}

  virtual void on_card_scored(const Card &card, ScoringContext &ctx,
                              GameState &state) {}

  virtual void on_card_in_hand(const Card &card, ScoringContext &ctx,
                               GameState &state) {}

  virtual void on_joker_score(ScoringContext &ctx, GameState &state) {}

  virtual void on_discard(const std::vector<Card> &discarded,
                          GameState &state) {}

  virtual void on_sale(Item &sold_item, GameState &state) {}

  virtual void on_round_start(GameState &state) {}
  virtual void on_round_end(GameState &state) {}
};

inline Joker::~Joker() = default;

class Shop {
public:
  int max_item_slots;
  int max_voucher_slots;
  int max_pack_slots;
  std::vector<std::unique_ptr<Item>> available_items;
  std::vector<std::unique_ptr<Voucher>> available_vouchers;

  Shop(int max_item_slots, int max_voucher_slots, int max_pack_slots);
};

class GameState {
public:
  Context context;
  Deck deck;
  int ante;
  int money;
  Blind cur_blind;
  int round;
  int max_cards_in_hand;
  int max_cards_played;
  int hands;
  int max_joker_slots;
  int max_inventory_slots;
  int discards;
  int hands_left;
  int discards_left;

  std::array<int, 12> hand_chips = base_hand_chips;
  std::array<int, 12> hand_mult = base_hand_mult;

  std::array<int, 13> hand_levels;

  std::vector<Joker *> jokers;
  std::vector<std::unique_ptr<Item>> inventory;
  // Independent snapshot of the last successfully used Tarot/Planet, not The
  // Fool.
  std::unique_ptr<Item> last_used_card;
  std::unordered_map<HandType, int> hand_play_counts;

  unsigned long long round_score;
  // Extremely high total scores may overflow unsigned long long and wrap
  // around.
  unsigned long long total_score = 0;
  unsigned long long best_hand_score = 0;
  std::size_t hands_played = 0;
  std::array<double, 3> blind_score_reqs;
  double cur_blind_score_req;

  GameState(int money = 4, int hands = 4, int discards = 4,
            int max_joker_slots = 5, int max_inventory_slots = 2);

  std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()>
  get_cards_by_rank(const std::vector<Card> &hand);

  std::array<std::vector<const Card *>, ALL_SUITS.size()>
  get_cards_by_suit(const std::vector<Card> &hand);

  HandEval evaluate_hand(const std::vector<Card> &hand);

  void play_hand(const std::vector<Card> &hand);

  void use_item(std::size_t index);

  std::vector<Card> discard(std::vector<Card> hand,
                            std::vector<std::size_t> chosen_card_indices);

  bool lose_round();

  bool start_new_round();

  void win_round();

  void game_loop();
};
