// TarotType intentionally lives in the implementation file.
// Do not link TarotCard.cpp separately when building this test executable.
#include "../TarotCard.cpp"

#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>

namespace {

int passed = 0;
int failed = 0;

void expect(bool condition, const char *message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename Test>
void run(const std::string &name, const char *input, const char *expected,
         Test test) {
  std::cout << "========================================\n"
            << "Test name: " << name << "\nInput: " << input << '\n';
  try {
    const std::string output = test();
    std::cout << "Output: " << output << "\nCorrect output: " << expected
              << "\nVerdict: PASS\n";
    ++passed;
  } catch (const std::exception &error) {
    std::cout << "Output: " << error.what() << "\nCorrect output: " << expected
              << "\nVerdict: FAILED\n";
    ++failed;
  }
}

auto tarot(TarotType type) {
  return std::make_unique<TarotCard>(type, 3, 1);
}

const TarotCard &as_tarot(const Item *item) {
  const auto *card = dynamic_cast<const TarotCard *>(item);
  expect(card != nullptr, "Expected a TarotCard");
  return *card;
}

void expect_history(const GameState &game, TarotType type) {
  expect(as_tarot(game.last_used_card.get()).name == type,
         "Incorrect last used Tarot");
}

struct TestJoker : Joker {
  explicit TestJoker(int price) {
    buy_cost = 0;
    sell_price = price;
    rarity = JokerRarity::COMMON;
  }
};

using CardKey = std::tuple<Suit, CardRank, Edition, Enhancement, Seal, int>;

auto card_keys(const std::vector<Card> &cards) {
  std::vector<CardKey> result;
  for (const auto &card : cards) {
    result.emplace_back(card.suit, card.rank, card.edition, card.enhancement,
                        card.seal, card.chips);
  }
  std::sort(result.begin(), result.end());
  return result;
}

void test_fool() {
  run("Fool without history", "Fool in inventory; no previous card",
      "Fool consumed; no card created", [] {
    GameState game;
    std::vector<Card> hand;
    game.inventory.push_back(tarot(TarotType::THE_FOOL));
    game.use_item(0, hand);
    expect(game.inventory.empty(), "Fool was not consumed");
    expect(!game.last_used_card, "Fool incorrectly changed history");
    return "Fool consumed; no card created";
  });

  run("Fool frees its slot", "full 2-slot inventory; previously used Hermit",
      "2 items; independent Hermit copy; money unchanged", [] {
    GameState game;
    std::vector<Card> hand;
    {
      TarotCard previous(TarotType::THE_HERMIT, 7, 2);
      game.last_used_card = previous.clone();
      previous.set_sell_price(99);
    }
    const Item *history = game.last_used_card.get();
    game.inventory.push_back(tarot(TarotType::THE_FOOL));
    game.inventory.push_back(tarot(TarotType::TEMPERANCE));
    const Item *other = game.inventory[1].get();
    game.use_item(0, hand);
    expect(game.inventory.size() == 2, "Incorrect inventory size");
    expect(game.inventory[0].get() == other, "Unrelated item was replaced");
    const auto &copy = as_tarot(game.inventory[1].get());
    expect(copy.name == TarotType::THE_HERMIT && copy.buy_cost == 7 &&
               copy.sell_price == 2, "Type or prices not copied");
    expect(&copy != history && game.last_used_card.get() == history,
           "History was aliased or replaced");
    expect(game.money == 4, "Fool activated the created card");
    return "2 items; independent Hermit copy; money unchanged";
  });

  run("Fool exclusions and capacity", "Fool history, unsupported item, full inventory",
      "no copies created", [] {
    GameState game;
    std::vector<Card> hand;
    TarotCard fool(TarotType::THE_FOOL, 3, 1);
    game.last_used_card = fool.clone();
    fool.activate(game, hand, {});
    expect(game.inventory.empty(), "Fool copied itself");
    game.last_used_card = std::make_unique<Item>();
    fool.activate(game, hand, {});
    expect(game.inventory.empty(), "Unsupported item was copied");
    game.last_used_card = tarot(TarotType::THE_HERMIT);
    game.inventory.push_back(tarot(TarotType::THE_SUN));
    game.inventory.push_back(tarot(TarotType::THE_MOON));
    fool.activate(game, hand, {}); // Direct activation: no slot is freed.
    expect(game.inventory.size() == 2, "Inventory capacity exceeded");
    return "no copies created";
  });
}

void test_money_cards() {
  run("Hermit balances and cap", "balances -5, 0, 4, 20, 100",
      "-5, 0, 8, 40, 120", [] {
    const std::array balances = {-5, 0, 4, 20, 100};
    const std::array expected = {-5, 0, 8, 40, 120};
    std::ostringstream output;
    for (std::size_t i = 0; i < balances.size(); ++i) {
      GameState game;
      std::vector<Card> hand;
      game.money = balances[i];
      game.inventory.push_back(tarot(TarotType::THE_HERMIT));
      game.use_item(0, hand, {999}); // Money effects ignore the selection.
      expect(game.money == expected[i], "Incorrect Hermit payout");
      expect(game.inventory.empty(), "Hermit was not consumed");
      expect_history(game, TarotType::THE_HERMIT);
      if (i != 0) output << ", ";
      output << game.money;
    }
    return output.str();
  });

  run("Temperance total sell value", "Jokers worth 20, 25, 15; balance 4",
      "balance 54; Jokers unchanged", [] {
    GameState game;
    std::vector<Card> hand;
    TestJoker first(20), second(25), third(15);
    game.jokers = {&first, &second, &third};
    game.inventory.push_back(tarot(TarotType::TEMPERANCE));
    game.use_item(0, hand);
    expect(game.money == 54, "The $50 cap was not applied to the total");
    expect(game.jokers == std::vector<Joker *>{&first, &second, &third},
           "Jokers were changed or removed");
    expect(first.sell_price == 20 && second.sell_price == 25 && third.sell_price == 15,
           "Sell values were changed");
    expect_history(game, TarotType::TEMPERANCE);
    return "balance 54; Jokers unchanged";
  });

  run("Temperance empty and boundary values", "no Jokers; 7+12; very expensive Jokers",
      "balances 4, 23, 54", [] {
    std::vector<Card> hand;
    TarotCard temperance(TarotType::TEMPERANCE, 3, 1);
    GameState game;
    temperance.activate(game, hand, {});
    expect(game.money == 4, "Empty inventory gave a payout");
    TestJoker first(7), second(12), expensive(std::numeric_limits<int>::max());
    game.jokers = {&first, nullptr, &second};
    temperance.activate(game, hand, {});
    expect(game.money == 23, "Incorrect uncapped payout");
    game.money = 4;
    game.jokers = {&expensive, &expensive};
    temperance.activate(game, hand, {});
    expect(game.money == 54, "Large sell values overflowed the payout");
    return "balances 4, 23, 54";
  });

  run("Hermit -> Fool -> Hermit", "balance 4; use Hermit, recreate it, use copy",
      "balance 16; inventory empty; history Hermit", [] {
    GameState game;
    std::vector<Card> hand;
    game.inventory.push_back(tarot(TarotType::THE_HERMIT));
    game.inventory.push_back(tarot(TarotType::THE_FOOL));
    game.use_item(0, hand);
    game.use_item(0, hand);
    game.use_item(0, hand);
    expect(game.money == 16 && game.inventory.empty(), "Incorrect reuse result");
    expect_history(game, TarotType::THE_HERMIT);
    return "balance 16; inventory empty; history Hermit";
  });
}

void test_emperor() {
  run("Emperor slot limits", "available slots 0, 1, 2, 5",
      "created 0, 1, 2, 2; valid types; prices 3/1", [] {
    std::vector<Card> hand;
    for (const int limit : {0, 1, 2, 5}) {
      GameState game;
      game.max_inventory_slots = limit;
      TarotCard emperor(TarotType::THE_EMPEROR, 3, 1);
      emperor.activate(game, hand, {});
      expect(game.inventory.size() == static_cast<std::size_t>(std::min(limit, 2)),
             "Incorrect generated count");
      for (const auto &item : game.inventory) {
        const auto &card = as_tarot(item.get());
        expect(card.name >= TarotType::THE_FOOL && card.name <= TarotType::THE_WORLD,
               "Generated an invalid Tarot type");
        expect(card.buy_cost == 3 && card.sell_price == 1, "Incorrect generated prices");
      }
      expect(game.money == 4, "Generated cards were charged or activated");
      expect_history(game, TarotType::THE_EMPEROR);
    }
    return "created 0, 1, 2, 2; valid types; prices 3/1";
  });

  run("Emperor in a full inventory", "Emperor and Fool in 2 slots",
      "one random Tarot created; Fool recreates Emperor", [] {
    GameState game;
    std::vector<Card> hand;
    game.inventory.push_back(tarot(TarotType::THE_EMPEROR));
    game.inventory.push_back(tarot(TarotType::THE_FOOL));
    const Item *fool = game.inventory[1].get();
    game.use_item(0, hand);
    expect(game.inventory.size() == 2 && game.inventory[0].get() == fool,
           "Emperor did not respect the newly freed slot");
    game.use_item(0, hand);
    expect(game.inventory.size() == 2, "Fool did not replace its own slot");
    expect(as_tarot(game.inventory[1].get()).name == TarotType::THE_EMPEROR,
           "Created Tarot replaced Emperor in the history");
    return "one random Tarot created; Fool recreates Emperor";
  });
}

void test_suit_card(TarotType type, Suit suit, const char *name) {
  run(std::string(name) + " selection", "unordered, repeated and invalid indices; 4 valid choices",
      "first 3 unique cards changed; other cards and properties preserved", [=] {
    GameState game;
    std::vector<Card> hand = {
        {Suit::HEARTS, CardRank::TWO}, {Suit::CLUBS, CardRank::KING},
        {Suit::SPADES, CardRank::ACE}, {Suit::DIAMONDS, CardRank::NINE}};
    hand[0].edition = Edition::FOIL;
    hand[1].seal = Seal::RED;
    hand[2].enhancement = Enhancement::BONUS;
    const auto before = hand;
    game.inventory.push_back(tarot(type));
    game.use_item(0, hand, {2, 0, 2, 1, 3, 999});
    expect(game.inventory.empty(), "Tarot was not consumed");
    for (std::size_t i = 0; i < hand.size(); ++i) {
      expect(hand[i].suit == (i < 3 ? suit : before[i].suit), "Incorrect suit");
      expect(hand[i].rank == before[i].rank && hand[i].edition == before[i].edition &&
                 hand[i].enhancement == before[i].enhancement &&
                 hand[i].seal == before[i].seal && hand[i].chips == before[i].chips,
             "A non-suit property changed");
    }
    expect_history(game, type);
    return "first 3 unique cards changed; other cards and properties preserved";
  });

  run(std::string(name) + " no selection", "empty selection, then invalid indices",
      "items consumed; hand unchanged", [=] {
    GameState game;
    std::vector<Card> hand = {{Suit::HEARTS, CardRank::TWO}};
    const auto before = card_keys(hand);
    for (const auto &selection : std::vector<std::vector<std::size_t>>{{}, {999}}) {
      game.inventory.push_back(tarot(type));
      game.use_item(0, hand, selection);
      expect(game.inventory.empty() && card_keys(hand) == before,
             "Incorrect behavior for an empty/invalid selection");
    }
    return "items consumed; hand unchanged";
  });

  run(std::string(name) + " persistence", "modified hand; discard; 3 subsequent rounds; restart",
      "changes preserved with 52 cards; restart restores standard deck", [=] {
    GameState game;
    auto hand = game.deck.deal(0, 8);
    game.inventory.push_back(tarot(type));
    game.use_item(0, hand, {0, 2});
    auto all_cards = hand;
    all_cards.insert(all_cards.end(), game.deck.deck.begin(), game.deck.deck.end());
    const auto expected = card_keys(all_cards);
    hand = game.discard(hand, {0});
    game.deck.finish_round(hand); // Return both the used card and the remaining hand.
    expect(hand.empty() && card_keys(game.deck.deck) == expected, "Changes lost on return");
    for (int round = 0; round < 3; ++round) {
      hand = game.deck.deal(0, 52);
      expect(card_keys(hand) == expected, "Changes lost on deal");
      hand = game.discard(hand, {0, 1, 2});
      game.deck.finish_round(hand);
      expect(card_keys(game.deck.deck) == expected, "Cards lost or duplicated");
    }
    game.inventory.push_back(tarot(TarotType::THE_FOOL));
    game.use_item(0, hand);
    expect(as_tarot(game.inventory[0].get()).name == type, "Fool did not copy suit Tarot");
    game = GameState{};
    const Deck fresh;
    expect(card_keys(game.deck.deck) == card_keys(fresh.deck), "Restart kept modified cards");
    expect(!game.last_used_card && game.inventory.empty(), "Restart kept Tarot history");
    return "changes preserved with 52 cards; restart restores standard deck";
  });
}

void test_death() {
  run("Death copies the right card", "select indices 2, 0; right card has modifiers and prices",
      "left becomes independent full copy; right and middle unchanged", [] {
    GameState game;
    std::vector<Card> hand = {
        {Suit::HEARTS, CardRank::TWO}, {Suit::CLUBS, CardRank::NINE},
        {Suit::SPADES, CardRank::ACE, Edition::POLYCHROME, Enhancement::BONUS, Seal::RED}};
    for (auto &card : hand) { card.buy_cost = 0; card.sell_price = 0; }
    hand[2].chips = 99;
    hand[2].buy_cost = 8;
    hand[2].sell_price = 4;
    const auto before = hand;
    game.inventory.push_back(tarot(TarotType::DEATH));
    game.use_item(0, hand, {2, 0});
    expect(hand.size() == 3 && game.inventory.empty(), "Incorrect consumption or hand size");
    expect(card_keys({hand[0]}) == card_keys({before[2]}), "Not all card properties copied");
    expect(hand[0].buy_cost == 8 && hand[0].sell_price == 4, "Item prices not copied");
    expect(card_keys({hand[1], hand[2]}) == card_keys({before[1], before[2]}),
           "Unselected or source card changed");
    hand[0].chips = 1;
    expect(hand[2].chips == 99, "Copied cards share mutable state");
    expect_history(game, TarotType::DEATH);
    return "left becomes independent full copy; right and middle unchanged";
  });

  run("Death invalid selections", "empty, single, duplicate, out-of-range and three indices",
      "Death retained; cards and previous history unchanged", [] {
    GameState game;
    std::vector<Card> hand = {
        {Suit::HEARTS, CardRank::TWO}, {Suit::CLUBS, CardRank::NINE},
        {Suit::SPADES, CardRank::ACE}};
    const auto before = card_keys(hand);
    game.last_used_card = tarot(TarotType::THE_HERMIT);
    const Item *history = game.last_used_card.get();
    game.inventory.push_back(tarot(TarotType::DEATH));
    const Item *death = game.inventory[0].get();
    for (const auto &selection : std::vector<std::vector<std::size_t>>{
             {}, {0}, {1, 1}, {0, 999}, {0, 1, 2}}) {
      expect(!game.use_item(0, hand, selection), "Invalid Death activation succeeded");
      expect(game.inventory.size() == 1 && game.inventory[0].get() == death,
             "Invalid selection consumed Death");
      expect(card_keys(hand) == before, "Invalid selection changed cards");
      expect(game.last_used_card.get() == history, "Invalid use replaced history");
    }
    return "Death retained; cards and previous history unchanged";
  });

  run("Death persists and Fool recreates it", "clone, discard, return, redeal, restart",
      "52 cards with copied properties; Fool recreates Death; restart resets", [] {
    GameState game;
    auto hand = game.deck.deal(0, 8);
    hand[2].edition = Edition::FOIL;
    hand[2].chips = 99;
    hand[2].buy_cost = 8;
    hand[2].sell_price = 4;
    game.inventory.push_back(tarot(TarotType::DEATH));
    game.use_item(0, hand, {0, 2});
    auto all = hand;
    all.insert(all.end(), game.deck.deck.begin(), game.deck.deck.end());
    const auto expected = card_keys(all);
    hand = game.discard(hand, {0, 2});
    game.deck.finish_round(hand);
    for (int round = 0; round < 3; ++round) {
      hand = game.deck.deal(0, 52);
      expect(card_keys(hand) == expected, "Death changes lost or cards duplicated");
      game.deck.finish_round(hand);
    }
    game.inventory.push_back(tarot(TarotType::THE_FOOL));
    game.use_item(0, hand);
    expect(as_tarot(game.inventory[0].get()).name == TarotType::DEATH,
           "Fool did not recreate Death");
    game = GameState{};
    const Deck fresh;
    expect(card_keys(game.deck.deck) == card_keys(fresh.deck), "Restart kept Death changes");
    return "52 cards with copied properties; Fool recreates Death; restart resets";
  });
}

} // namespace

int main() {
  test_fool();
  test_money_cards();
  test_emperor();
  test_death();
  test_suit_card(TarotType::THE_STAR, Suit::DIAMONDS, "Star");
  test_suit_card(TarotType::THE_MOON, Suit::CLUBS, "Moon");
  test_suit_card(TarotType::THE_SUN, Suit::HEARTS, "Sun");
  test_suit_card(TarotType::THE_WORLD, Suit::SPADES, "World");
  std::cout << "\nTarot tests: " << passed << " passed, " << failed << " failed.\n";
  return failed == 0 ? 0 : 1;
}
