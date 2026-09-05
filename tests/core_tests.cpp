#include "../checks.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace {

int passed_tests = 0;
int failed_tests = 0;

template <typename Actual, typename Expected>
void expect_equal(const Actual &actual, const Expected &expected) {
  if (actual != expected) {
    std::ostringstream message;
    message << "expected " << expected << ", got " << actual;
    throw std::runtime_error(message.str());
  }
}

template <typename Test>
void run_test(const char *name, const char *input, const char *expected,
              Test test) {
  std::cout << "========================================\n"
            << "Test name: " << name << "\n"
            << "Input: " << input << "\n";
  try {
    test();
    ++passed_tests;
    std::cout << "Output: " << expected << "\n"
              << "Correct output: " << expected << "\nVerdict: PASS\n";
  } catch (const std::exception &error) {
    ++failed_tests;
    std::cout << "Output: " << error.what() << "\n"
              << "Correct output: " << expected << "\nVerdict: FAILED\n";
  }
}

// Restore streams even when a test throws; keep interactive output out of the report.
struct ConsoleSession {
  std::istringstream input;
  std::ostringstream output;
  std::streambuf *old_input;
  std::streambuf *old_output;

  explicit ConsoleSession(const std::string &text)
      : input(text), old_input(std::cin.rdbuf(input.rdbuf())),
        old_output(std::cout.rdbuf(output.rdbuf())) {}

  ~ConsoleSession() {
    std::cin.rdbuf(old_input);
    std::cout.rdbuf(old_output);
  }
};

using CardKey = std::tuple<Suit, CardRank, Edition, Enhancement, Seal, int>;

std::vector<CardKey> card_keys(const std::vector<Card> &cards) {
  std::vector<CardKey> result;
  for (const auto &card : cards) {
    result.emplace_back(card.suit, card.rank, card.edition, card.enhancement,
                        card.seal, card.chips);
  }
  return result;
}

std::vector<CardKey> sorted_keys(const std::vector<Card> &cards) {
  auto result = card_keys(cards);
  std::sort(result.begin(), result.end());
  return result;
}

const std::vector<Card> pair_hand = {
    {Suit::HEARTS, CardRank::TWO}, {Suit::CLUBS, CardRank::TWO},
    {Suit::SPADES, CardRank::ACE}};

void test_cards() {
  const std::array expected_chips = {2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10, 11};
  for (std::size_t i = 0; i < ALL_CARD_RANKS.size(); ++i) {
    Card card(Suit::HEARTS, ALL_CARD_RANKS[i]);
    expect_equal(card.chips, expected_chips[i]);
    expect_equal(card.edition == Edition::BASE, true);
    expect_equal(card.enhancement == Enhancement::NONE, true);
    expect_equal(card.seal == Seal::NONE, true);
  }
  Card special(Suit::SPADES, CardRank::ACE, Edition::FOIL,
               Enhancement::BONUS, Seal::RED);
  expect_equal(special.edition == Edition::FOIL, true);
  expect_equal(special.enhancement == Enhancement::BONUS, true);
  expect_equal(special.seal == Seal::RED, true);
}

void test_deck() {
  Deck deck;
  auto initial = sorted_keys(deck.deck);
  expect_equal(initial.size(), 52u);
  expect_equal(std::adjacent_find(initial.begin(), initial.end()) == initial.end(), true);
  deck.deck[0].edition = Edition::FOIL;
  initial = sorted_keys(deck.deck);
  deck.shuffle();
  expect_equal(sorted_keys(deck.deck) == initial, true);
  auto dealt = deck.deal(3, 8);
  expect_equal(dealt.size(), 5u);
  expect_equal(deck.deck.size(), 47u);
  dealt.insert(dealt.end(), deck.deck.begin(), deck.deck.end());
  expect_equal(sorted_keys(dealt) == initial, true);
  expect_equal(deck.deal(8, 8).empty(), true);
  expect_equal(deck.deal(9, 8).empty(), true);
  expect_equal(deck.deal(0, 100).size(), 47u);
  deck.shuffle();
  expect_equal(deck.deal(0, 8).empty(), true);
  deck.deck.emplace_back(Suit::HEARTS, CardRank::ACE);
  deck.shuffle();
  expect_equal(deck.deal(0, 8).size(), 1u);
}

void test_grouping_and_evaluation() {
  GameState game;
  const auto ranks = game.get_cards_by_rank(pair_hand);
  const auto suits = game.get_cards_by_suit(pair_hand);
  expect_equal(ranks[0].size(), 2u);
  expect_equal(ranks[0][0] == &pair_hand[0], true);
  expect_equal(suits[static_cast<std::size_t>(Suit::SPADES)][0] == &pair_hand[2], true);
  const auto evaluation = game.evaluate_hand(pair_hand);
  expect_equal(evaluation.hand_type == HandType::PAIR, true);
  expect_equal(evaluation.cards_scored.size(), 2u);
  expect_equal(evaluation.cards_scored[0] == &pair_hand[0], true);
  expect_equal(evaluation.cards_scored[1] == &pair_hand[1], true);
  bool threw = false;
  try { game.evaluate_hand({}); }
  catch (const std::invalid_argument &) { threw = true; }
  expect_equal(threw, true);
}

void test_scoring_and_stats() {
  GameState game;
  game.play_hand(pair_hand);
  expect_equal(game.round_score, 28u); // (10 + 2 + 2) * 2; ace does not score.
  expect_equal(game.total_score, 28u);
  expect_equal(game.best_hand_score, 28u);
  expect_equal(game.hands_left, 3);
  game.cur_blind_score_req = 28;
  game.win_round();
  expect_equal(game.round_score, 0u);
  expect_equal(game.total_score, 28u);
  game.play_hand({{Suit::HEARTS, CardRank::TWO}});
  expect_equal(game.total_score, 35u);
  expect_equal(game.best_hand_score, 28u);
  expect_equal(game.hands_played, 2u);
}

void test_all_hand_types() {
  struct Case {
    HandType expected;
    std::vector<CardRank> ranks;
    std::vector<Suit> suits;
    std::size_t scoring_cards;
  };
  using enum CardRank;
  using enum Suit;
  const std::vector<Case> cases = {
      {HandType::HIGH_CARD, {TWO, NINE, ACE}, {HEARTS, CLUBS, SPADES}, 1},
      {HandType::PAIR, {TWO, TWO, ACE}, {HEARTS, CLUBS, SPADES}, 2},
      {HandType::TWO_PAIR, {TWO, TWO, KING, KING, ACE},
       {HEARTS, CLUBS, SPADES, DIAMONDS, HEARTS}, 4},
      {HandType::THREE_OF_A_KIND, {TWO, TWO, TWO, KING, ACE},
       {HEARTS, CLUBS, SPADES, DIAMONDS, HEARTS}, 3},
      {HandType::STRAIGHT, {TWO, THREE, FOUR, FIVE, SIX},
       {HEARTS, CLUBS, SPADES, DIAMONDS, HEARTS}, 5},
      {HandType::FLUSH, {TWO, FOUR, SIX, NINE, ACE},
       {HEARTS, HEARTS, HEARTS, HEARTS, HEARTS}, 5},
      {HandType::FULL_HOUSE, {TWO, TWO, TWO, KING, KING},
       {HEARTS, CLUBS, SPADES, DIAMONDS, HEARTS}, 5},
      {HandType::FOUR_OF_A_KIND, {TWO, TWO, TWO, TWO, ACE},
       {HEARTS, CLUBS, SPADES, DIAMONDS, HEARTS}, 4},
      {HandType::STRAIGHT_FLUSH, {TWO, THREE, FOUR, FIVE, SIX},
       {HEARTS, HEARTS, HEARTS, HEARTS, HEARTS}, 5},
      {HandType::ROYAL_FLUSH, {TEN, JACK, QUEEN, KING, ACE},
       {HEARTS, HEARTS, HEARTS, HEARTS, HEARTS}, 5},
      {HandType::FIVE_OF_A_KIND, {TWO, TWO, TWO, TWO, TWO},
       {HEARTS, CLUBS, SPADES, DIAMONDS, HEARTS}, 5},
      {HandType::FLUSH_HOUSE, {TWO, TWO, TWO, KING, KING},
       {HEARTS, HEARTS, HEARTS, HEARTS, HEARTS}, 5},
      {HandType::FLUSH_FIVE, {TWO, TWO, TWO, TWO, TWO},
       {HEARTS, HEARTS, HEARTS, HEARTS, HEARTS}, 5},
  };
  GameState game;
  for (const auto &test : cases) {
    std::vector<Card> cards;
    for (std::size_t i = 0; i < test.ranks.size(); ++i) {
      cards.emplace_back(test.suits[i], test.ranks[i]);
    }
    // Evaluate both orders to ensure classification is independent of input order.
    for (int order = 0; order < 2; ++order) {
      const auto result = game.evaluate_hand(cards);
      expect_equal(static_cast<int>(result.hand_type), static_cast<int>(test.expected));
      expect_equal(result.cards_scored.size(), test.scoring_cards);
      for (const Card *scored : result.cards_scored) {
        expect_equal(std::any_of(cards.begin(), cards.end(),
                                [scored](const Card &card) { return scored == &card; }), true);
      }
      std::reverse(cards.begin(), cards.end());
    }
  }
}

void test_discard() {
  GameState game;
  auto remaining = game.discard(pair_hand, {0, 2});
  expect_equal(remaining.size(), 1u);
  expect_equal(card_keys(remaining) == card_keys({pair_hand[1]}), true);
  expect_equal(card_keys(game.discard(pair_hand, {2, 0})) == card_keys(remaining), true);
  expect_equal(card_keys(game.discard(pair_hand, {})) == card_keys(pair_hand), true);
  expect_equal(game.discard(pair_hand, {1, 0, 2}).empty(), true);
  expect_equal(game.discard({}, {}).empty(), true);
}

void test_round_transitions() {
  GameState game(10, 6, 2);
  expect_equal(game.hands_left, 6);
  expect_equal(game.discards_left, 2);
  game.hands_left = 2;
  game.discards_left = 0;
  game.round_score = 500;
  game.win_round();
  expect_equal(game.money, 15);
  expect_equal(game.round, 1);
  expect_equal(game.cur_blind == Blind::BIG, true);
  expect_equal(game.hands_left, 6);
  expect_equal(game.discards_left, 2);
  expect_equal(game.round_score, 0u);
  game.win_round();
  expect_equal(game.cur_blind == Blind::BOSS, true);
  game.win_round();
  expect_equal(game.cur_blind == Blind::SMALL, true);
  expect_equal(game.ante, 2);
}

void test_restart_and_exit() {
  GameState game;
  game.play_hand(pair_hand);
  game.win_round();
  {
    ConsoleSession console("?\nr\n");
    expect_equal(game.lose_round(), true);
    expect_equal(console.output.str().find("Round reached:   2") != std::string::npos, true);
    expect_equal(console.output.str().find("Total score:     28") != std::string::npos, true);
  }
  expect_equal(game.round, 0);
  expect_equal(game.ante, 1);
  expect_equal(game.money, 4);
  expect_equal(game.deck.deck.size(), 52u);
  expect_equal(game.total_score, 0u);
  expect_equal(game.best_hand_score, 0u);
  expect_equal(game.hands_played, 0u);
  { ConsoleSession console("Q\n"); expect_equal(game.lose_round(), false); }
  { ConsoleSession console(""); expect_equal(game.lose_round(), false); }
}

void test_negative_checks() {
  GameState game;
  const auto empty_ranks = game.get_cards_by_rank({});
  const auto empty_suits = game.get_cards_by_suit({});
  expect_equal(find_pairs(empty_ranks).empty(), true);
  expect_equal(find_three_of_a_kind(empty_ranks).empty(), true);
  expect_equal(find_four_of_a_kind(empty_ranks).empty(), true);
  expect_equal(find_five_of_a_kind(empty_ranks).empty(), true);
  expect_equal(find_straight(empty_ranks).empty(), true);
  expect_equal(find_flush(empty_suits).empty(), true);
  expect_equal(find_royal_flush(empty_suits).empty(), true);
  expect_equal(find_full_house({}, {}).empty(), true);
  const std::vector<Card> five(5, Card(Suit::HEARTS, CardRank::TWO));
  const auto ranks = game.get_cards_by_rank(five);
  expect_equal(find_five_of_a_kind(ranks).size(), 1u);
  expect_equal(find_four_of_a_kind(ranks).empty(), true);
  expect_equal(find_pairs(ranks).empty(), true);
}

} // namespace

int main() {
  run_test("card defaults and modifiers", "all ranks; modified ace", "correct chips and fields", test_cards);
  run_test("deck, shuffle, deal", "full, depleted, empty and singleton decks", "cards preserved; correct deal sizes", test_deck);
  run_test("grouping and evaluation", "2H 2C AS; empty hand", "pair pointers; empty hand rejected", test_grouping_and_evaluation);
  run_test("all hand types and priority", "13 combinations, each in two orders", "strongest type; correct count; pointers into input", test_all_hand_types);
  run_test("scoring and statistics", "pair, round win, high card", "28 + 7 = 35 total; best 28; two hands", test_scoring_and_stats);
  run_test("discard by indices", "empty, reversed, partial and full selections", "only selected cards removed", test_discard);
  run_test("round transitions", "three wins; custom hand/discard limits", "rewards; Small -> Big -> Boss -> Small", test_round_transitions);
  run_test("restart and exit", "invalid choice, R, Q, EOF", "report before reset; fresh state; clean exit", test_restart_and_exit);
  run_test("negative checks and five of a kind", "empty hands; five identical cards", "no false matches; five of a kind found", test_negative_checks);

  run_test("score table coverage", "all HandType values", "each combination has chips and mult", [] {
    const auto count = static_cast<std::size_t>(HandType::FLUSH_FIVE) + 1;
    expect_equal(hand_chips.size(), count);
    expect_equal(hand_mult.size(), count);
  });
  run_test("upper hand types score correctly", "royal flush, five of a kind, flush house, flush five", "1208, 1560, 2296, 2720 points", [] {
    using enum CardRank;
    using enum Suit;
    const std::vector<std::vector<Card>> hands = {
        {{HEARTS, TEN}, {HEARTS, JACK}, {HEARTS, QUEEN}, {HEARTS, KING}, {HEARTS, ACE}},
        {{HEARTS, TWO}, {CLUBS, TWO}, {SPADES, TWO}, {DIAMONDS, TWO}, {HEARTS, TWO}},
        {{HEARTS, TWO}, {HEARTS, TWO}, {HEARTS, TWO}, {HEARTS, NINE}, {HEARTS, NINE}},
        std::vector<Card>(5, Card(HEARTS, TWO))};
    const std::array expected = {1208u, 1560u, 2296u, 2720u};
    for (std::size_t i = 0; i < hands.size(); ++i) {
      GameState game;
      game.play_hand(hands[i]);
      expect_equal(game.round_score, expected[i]);
    }
  });
  run_test("flush without royal sequence", "2H JH QH KH AH", "no royal flush", [] {
    GameState game;
    const std::vector<Card> cards = {
        {Suit::HEARTS, CardRank::TWO},
        {Suit::HEARTS, CardRank::JACK}, {Suit::HEARTS, CardRank::QUEEN},
        {Suit::HEARTS, CardRank::KING}, {Suit::HEARTS, CardRank::ACE}};
    expect_equal(find_royal_flush(game.get_cards_by_suit(cards)).empty(), true);
  });
  run_test("victory on last hand", "one hand left; 2H scores exactly 7", "win before loss; round advances", [] {
    GameState game(4, 1, 4);
    game.deck.deck.assign(8, Card(Suit::HEARTS, CardRank::TWO));
    game.cur_blind_score_req = 7;
    ConsoleSession console("0 x p\n");
    expect_equal(game.start_new_round(), true);
    expect_equal(game.round, 1);
    expect_equal(game.total_score, 7u);
  });
  run_test("game loop loss and quit", "four single-card plays, Q", "four hands; 28 total; game over", [] {
    GameState game;
    game.deck.deck.assign(32, Card(Suit::HEARTS, CardRank::TWO));
    ConsoleSession console("0 x p\n0 x p\n0 x p\n0 x p\nq\n");
    game.game_loop();
    expect_equal(game.hands_played, 4u);
    expect_equal(game.total_score, 28u);
    expect_equal(console.output.str().find("GAME OVER") != std::string::npos, true);
  });
  run_test("discard consumes a discard", "discard once, then win with 2H", "discards left: 3 shown before win", [] {
    GameState game;
    game.deck.deck.assign(16, Card(Suit::HEARTS, CardRank::TWO));
    game.cur_blind_score_req = 7;
    ConsoleSession console("0 x d\n0 x p\n");
    expect_equal(game.start_new_round(), true);
    expect_equal(console.output.str().find("Discards left: 3") != std::string::npos, true);
  });
  run_test("remaining cards stay in hand", "discard one of eight, then play one", "seven remaining cards plus one drawn", [] {
    GameState game;
    game.deck.deck.assign(52, Card(Suit::HEARTS, CardRank::TWO));
    game.cur_blind_score_req = 7;
    ConsoleSession console("0 x d\n0 x p\n");
    expect_equal(game.start_new_round(), true);
    expect_equal(game.deck.deck.size(), 43u);
  });

  run_test("exhausted discards", "zero discards; attempt discard then play", "discard rejected; only one card scored", [] {
    GameState game(4, 4, 0);
    game.deck.deck.assign(16, Card(Suit::HEARTS, CardRank::TWO));
    game.cur_blind_score_req = 7;
    ConsoleSession console("0 x d\n0 x p\n");
    expect_equal(game.start_new_round(), true);
    expect_equal(game.deck.deck.size(), 8u);
    expect_equal(game.hands_played, 1u);
    expect_equal(game.discards_left, 0);
    expect_equal(console.output.str().find("No discards left") != std::string::npos, true);
  });
  run_test("invalid selections preserve cards", "empty, out-of-range, duplicate indices, then valid play", "no cards consumed by rejected selections", [] {
    GameState game;
    game.deck.deck.assign(8, Card(Suit::HEARTS, CardRank::TWO));
    game.cur_blind_score_req = 7;
    ConsoleSession console("x p\n9 x p\n0 0 x p\n0 x p\n");
    expect_equal(game.start_new_round(), true);
    expect_equal(game.hands_played, 1u);
    expect_equal(game.total_score, 7u);
  });
  run_test("empty deck and input termination", "empty deck, zero hands, EOF during selection/action", "return without invalid card access", [] {
    {
      GameState game;
      game.deck.deck.clear();
      ConsoleSession console("q\n");
      expect_equal(game.start_new_round(), false);
    }
    {
      GameState game(4, 0, 4);
      ConsoleSession console("q\n");
      expect_equal(game.start_new_round(), false);
    }
    for (const auto *input : {"", "0 x"}) {
      GameState game;
      ConsoleSession console(input);
      expect_equal(game.start_new_round(), false);
      expect_equal(game.hands_played, 0u);
    }
  });

  run_test("hand sorted before index selection", "AS 2S KH 2H; play index 0", "AS, KH, 2H, 2S; selected AS scores 16", [] {
    GameState game;
    game.deck.deck = {
        {Suit::SPADES, CardRank::ACE}, {Suit::SPADES, CardRank::TWO},
        {Suit::HEARTS, CardRank::KING}, {Suit::HEARTS, CardRank::TWO}};
    game.cur_blind_score_req = 16;
    ConsoleSession console("0 x p\n");
    expect_equal(game.start_new_round(), true);
    expect_equal(game.total_score, 16u);
    expect_equal(console.output.str().find(
        "  [0] A♠\n  [1] K♥\n  [2] 2♥\n  [3] 2♠\n") != std::string::npos, true);
    expect_equal(console.output.str().find("Played cards: [A ♠]") != std::string::npos, true);
  });

  std::cout << "\nCore tests: " << passed_tests << " passed, "
            << failed_tests << " failed.\n";
  return failed_tests == 0 ? 0 : 1;
}
