#include "checks.hpp"
#include "core.hpp"
#include "types.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

const char *card_rank_name(CardRank rank) {
  constexpr std::array rank_names = {"2", "3",  "4", "5", "6", "7", "8",
                                     "9", "10", "J", "Q", "K", "A"};
  return rank_names[static_cast<std::size_t>(rank)];
}

const char *card_suit_name(Suit suit) {
  constexpr std::array suit_names = {"♥", "♦", "♣", "♠"};
  return suit_names[static_cast<std::size_t>(suit)];
}

const char *hand_type_name(HandType hand_type) {
  constexpr std::array hand_type_names = {
      "High card",      "Pair",        "Two pair",       "Three of a kind",
      "Straight",       "Flush",       "Full house",     "Four of a kind",
      "Straight flush", "Royal flush", "Five of a kind", "Flush house",
      "Flush five",
  };
  return hand_type_names[static_cast<std::size_t>(hand_type)];
}

const char *blind_name(Blind blind) {
  constexpr std::array blind_names = {"Small", "Big", "Boss"};
  return blind_names[static_cast<std::size_t>(blind)];
}

void print_hand(const std::vector<Card> &hand) {
  std::cout << "Cards in hand:\n";
  for (std::size_t index = 0; index < hand.size(); ++index) {
    const Card &card = hand[index];
    std::cout << "  [" << index << "] " << card_rank_name(card.rank)
              << card_suit_name(card.suit) << "\n";
  }
}

void print_cards(const char *label, const std::vector<Card> &cards) {
  std::cout << label << ": [";
  for (std::size_t index = 0; index < cards.size(); ++index) {
    if (index != 0) {
      std::cout << ", ";
    }
    std::cout << card_rank_name(cards[index].rank) << " "
              << card_suit_name(cards[index].suit);
  }
  std::cout << "]\n";
}

void print_round_status(const GameState &game) {
  std::cout << "\n========================================\n"
            << "Round: " << game.round + 1
            << " | Blind: " << blind_name(game.cur_blind) << "\n"
            << "Score: " << game.round_score << " / "
            << game.cur_blind_score_req << "\n"
            << "Hands left: " << game.hands_left
            << " | Discards left: " << game.discards_left << " | Money: $"
            << game.money << "\n"
            << "========================================\n";
}

} // namespace

GameState::GameState(int money, int hands, int discards, int max_joker_slots,
                     int max_inventory_slots)
    : ante(1), money(money), round(0), max_cards_in_hand(8),
      max_cards_played(5), max_joker_slots(max_joker_slots),
      max_inventory_slots(max_inventory_slots), hands(hands),
      discards(discards), hands_left(hands), discards_left(discards),
      cur_blind(Blind::SMALL), round_score(0) {
  context = Context::ROUND;
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
  HandEval eval = evaluate_hand(hand);
  ScoringContext ctx{.hand_eval = eval,
                     .chips = static_cast<unsigned long long>(
                         hand_chips[static_cast<size_t>(eval.hand_type)]),
                     .mult = static_cast<unsigned long long>(
                         hand_mult[static_cast<size_t>(eval.hand_type)])};

  const auto &[cards_scored, hand_type] = eval;

  // 1. Pre-scoring
  for (auto &joker : this->jokers) {
    joker->on_pre_score(hand, eval, ctx, *this);
  }

  // 2. Dealt hand scoring
  for (const Card &card : hand) {
    bool is_scoring = std::find(cards_scored.begin(), cards_scored.end(),
                                &card) != cards_scored.end();
    if (is_scoring) {
      ctx.chips += card.chips;
      // TODO: add counting editions, effects and seals
      for (auto &joker : this->jokers) {
        joker->on_card_scored(card, ctx, *this);
        joker->on_card_in_hand(card, ctx, *this);
      }
    }
  }

  for (auto &joker : this->jokers) {
    joker->on_joker_score(ctx, *this);
  }
  const auto hand_score = ctx.chips * ctx.mult;
  this->round_score += hand_score;
  this->total_score += hand_score;
  this->best_hand_score = std::max(this->best_hand_score, hand_score);
  ++this->hands_played;
  this->hands_left--;
}

void GameState::win_round() {
  std::cout << "\n========================================\n"
            << "              ROUND WON\n"
            << "========================================\n"
            << "Round reward:" << gold_per_blind[static_cast<size_t>(cur_blind)]
            << "$" << std::endl
            << hands_left << " hands left(1$ each):   " << "+" << hands_left
            << "$" << std::endl
            << "Cash out "
            << hands_left + gold_per_blind[static_cast<size_t>(cur_blind)]
            << "$ ?" << std::endl
            << "Press any key" << std::endl;
  char c;
  std::cin >> c;
  this->money +=
      this->hands_left + gold_per_blind[static_cast<size_t>(cur_blind)];
  this->round_score = 0;
  this->round++;
  if (round % 3 == 0) {
    this->ante++;
    for (size_t i = 0; i < ALL_BLINDS.size(); ++i) {
      blind_score_reqs[i] = ante_base_chips[ante] * blind_multipliers[i];
    }
  }
  this->cur_blind =
      static_cast<Blind>((static_cast<int>(this->cur_blind) + 1) % 3);
  this->cur_blind_score_req =
      this->blind_score_reqs[static_cast<size_t>(this->cur_blind)];
  this->hands_left = this->hands;
  this->discards_left = this->discards;
}

std::vector<Card>
GameState::discard(std::vector<Card> hand,
                   std::vector<std::size_t> chosen_card_indices) {
  std::sort(chosen_card_indices.rbegin(), chosen_card_indices.rend());

  for (const std::size_t index : chosen_card_indices) {
    hand.erase(hand.begin() + index);
  }

  return hand;
}

bool GameState::lose_round() {
  std::cout << "\n========================================\n"
            << "              GAME OVER\n"
            << "========================================\n"
            << "Round reached:   " << round + 1 << "\n"
            << "Rounds won:      " << round << "\n"
            << "Ante reached:    " << ante << "\n"
            << "Last blind:      " << blind_name(cur_blind) << "\n"
            << "Final round:     " << round_score << " / "
            << cur_blind_score_req << "\n"
            << "Total score:     " << total_score << "\n"
            << "Best hand score: " << best_hand_score << "\n"
            << "Hands played:    " << hands_played << "\n"
            << "Money left:      $" << money << "\n"
            << "========================================\n"
            << "[R] Restart game\n"
            << "[Q] Quit\n"
            << "Choice: ";

  char choice;
  while (std::cin >> choice) {
    switch (std::tolower(static_cast<unsigned char>(choice))) {
    case 'r':
      *this = GameState{};
      return true;
    case 'q':
      return false;
    default:
      std::cout << "Enter R to restart or Q to quit: ";
    }
  }
  return false;
}

bool GameState::start_new_round() {
  std::vector<Card> hand;
  while (hands_left > 0) {
    const auto drawn_cards =
        deck.deal(static_cast<int>(hand.size()), max_cards_in_hand);
    hand.insert(hand.end(), drawn_cards.begin(), drawn_cards.end());
    std::stable_sort(hand.begin(), hand.end(),
                     [](const Card &left, const Card &right) {
                       if (left.rank != right.rank) {
                         return left.rank > right.rank;
                       }
                       return left.suit < right.suit;
                     });
    print_round_status(*this);
    print_hand(hand);
    if (hand.empty()) {
      return lose_round();
    }

    std::vector<size_t> chosen_cards_indices;
    chosen_cards_indices.reserve(5);
    std::cout << "Choose card indices separated by spaces. Any non-digit fixes "
                 "selection: \n";
    size_t chosen_card_index;
    while (chosen_cards_indices.size() < max_cards_played) {
      char choice;
      if (!(std::cin >> choice)) {
        return false;
      }
      if (!std::isdigit(static_cast<unsigned char>(choice))) {
        break;
      }
      chosen_card_index = static_cast<size_t>(choice) - '0';
      chosen_cards_indices.push_back(chosen_card_index);
    }
    std::cout << "Enter 'd' if you want to discard, or any other key if you "
                 "want to play the selected hand: "
              << std::endl;
    char option;
    if (!(std::cin >> option)) {
      return false;
    }
    option = std::tolower(static_cast<unsigned char>(option));

    auto sorted_indices = chosen_cards_indices;
    std::sort(sorted_indices.begin(), sorted_indices.end());
    if (sorted_indices.empty() || sorted_indices.back() >= hand.size() ||
        std::adjacent_find(sorted_indices.begin(), sorted_indices.end()) !=
            sorted_indices.end()) {
      std::cout << "Choose unique indices of cards currently in hand.\n";
      continue;
    }

    if (option != 'd') {
      std::vector<Card> chosen_cards;
      for (size_t index : chosen_cards_indices) {
        chosen_cards.push_back(hand[index]);
      }

      print_cards("Played cards", chosen_cards);
      const HandEval hand_eval = evaluate_hand(chosen_cards);
      std::cout << "Combination: " << hand_type_name(hand_eval.hand_type)
                << "\n";

      const auto score_before = round_score;

      play_hand(chosen_cards);

      std::cout << "Score gained: " << round_score - score_before << "\n";
    } else {
      if (discards_left <= 0) {
        std::cout << "No discards left. Play a hand instead.\n";
        continue;
      }
      std::vector<Card> discarded_cards;
      for (size_t index : chosen_cards_indices) {
        discarded_cards.push_back(hand[index]);
      }
      print_cards("Discarded cards", discarded_cards);
      --discards_left;
    }
    hand = discard(hand, chosen_cards_indices);

    if (this->round_score >= cur_blind_score_req) {
      win_round();
      return true;
    } else if (this->hands_left == 0) {
      return lose_round();
    }
  }
  return lose_round();
}

void GameState::enter_shop() {
  this->context = Context::SHOP;
  available_jokers.
}

void GameState::game_loop() {
  bool game_end = false;
  while (!game_end) {
    game_end = !start_new_round();
    if (!game_end) {
      context = Context::SHOP;
    }
  }
}
