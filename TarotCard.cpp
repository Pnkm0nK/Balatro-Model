#include "core.hpp"
#include <algorithm>
#include <random>

enum class TarotType {
  THE_FOOL,
  THE_MAGICIAN,
  THE_HIGH_PRIESTESS,
  THE_EMPRESS,
  THE_EMPEROR,
  THE_HIEROPHANT,
  THE_LOVERS,
  THE_CHARIOT,
  JUSTICE,
  THE_HERMIT,
  THE_WHEEL_OF_FORTUNE,
  STRENGTH,
  THE_HANGED_MAN,
  DEATH,
  TEMPERANCE,
  THE_DEVIL,
  THE_TOWER,
  THE_STAR,
  THE_MOON,
  THE_SUN,
  JUDGEMENT,
  THE_WORLD,
};

namespace {

void change_selected_suits(std::vector<Card> &hand,
                           const std::vector<std::size_t> &selected_indices,
                           Suit suit) {
  auto indices = selected_indices;
  std::sort(indices.begin(), indices.end());
  indices.erase(std::unique(indices.begin(), indices.end()), indices.end());

  std::size_t changed = 0;
  for (const auto index : indices) {
    if (index >= hand.size() || changed == 3) {
      break;
    }
    hand[index].suit = suit;
    ++changed;
  }
}

} // namespace

TarotCard::TarotCard(TarotType name, int buy_cost, int sell_price) : name(name) {
  this->buy_cost = buy_cost;
  this->sell_price = sell_price;
}

void TarotCard::set_sell_price(int price) {
  sell_price = price;
}

std::unique_ptr<Item> TarotCard::clone() const {
  return std::make_unique<TarotCard>(*this);
}

bool TarotCard::can_activate(
    const std::vector<Card> &hand,
    const std::vector<std::size_t> &selected_indices) const {
  if (name != TarotType::DEATH) {
    return true;
  }
  return selected_indices.size() == 2 &&
         selected_indices[0] != selected_indices[1] &&
         selected_indices[0] < hand.size() && selected_indices[1] < hand.size();
}

void TarotCard::activate(
    GameState &state, std::vector<Card> &hand,
    const std::vector<std::size_t> &selected_indices) {
  // Once an effect below is implemented, record only successful use with:
  // state.last_used_card = clone();
  // Placeholder effects and The Fool must leave the history unchanged.
  switch (name) {
  case TarotType::THE_FOOL: {
    if (!state.last_used_card || state.max_inventory_slots <= 0 ||
        state.inventory.size() >=
            static_cast<std::size_t>(state.max_inventory_slots)) {
      return;
    }

    const auto *last_tarot =
        dynamic_cast<const TarotCard *>(state.last_used_card.get());
    if (last_tarot != nullptr && last_tarot->name == TarotType::THE_FOOL) {
      return;
    }

    auto card = state.last_used_card->clone();
    if (card) {
      state.inventory.push_back(std::move(card));
    }
    // The Fool never replaces the saved card or activates the created copy.
    return;
  }
  case TarotType::THE_MAGICIAN: {
    // TODO: Enhance 2 selected cards into Lucky Cards.
    break;
  }
  case TarotType::THE_HIGH_PRIESTESS: {
    // TODO: Create up to 2 random Planets if there is room.
    break;
  }
  case TarotType::THE_EMPRESS: {
    // TODO: Enhance 2 selected cards into Mult Cards.
    break;
  }
  case TarotType::THE_EMPEROR: {
    static std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> tarot_type(
        static_cast<int>(TarotType::THE_FOOL),
        static_cast<int>(TarotType::THE_WORLD));
    constexpr int tarot_buy_cost = 3;
    constexpr int tarot_sell_price = 1;

    if (state.max_inventory_slots > 0) {
      for (int created = 0;
           created < 2 && state.inventory.size() <
                              static_cast<std::size_t>(state.max_inventory_slots);
           ++created) {
        state.inventory.push_back(std::make_unique<TarotCard>(
            static_cast<TarotType>(tarot_type(generator)),
            tarot_buy_cost, tarot_sell_price));
      }
    }
    // The Emperor was used even if there was no room to create cards.
    state.last_used_card = clone();
    break;
  }
  case TarotType::THE_HIEROPHANT: {
    // TODO: Enhance 2 selected cards into Bonus Cards.
    break;
  }
  case TarotType::THE_LOVERS: {
    // TODO: Enhance 1 selected card into a Wild Card.
    break;
  }
  case TarotType::THE_CHARIOT: {
    // TODO: Enhance 1 selected card into a Steel Card.
    break;
  }
  case TarotType::JUSTICE: {
    // TODO: Enhance 1 selected card into a Glass Card.
    break;
  }
  case TarotType::THE_HERMIT: {
    // Add up to $20; a negative balance must not increase the debt.
    state.money += std::clamp(state.money, 0, 20);
    state.last_used_card = clone();
    break;
  }
  case TarotType::THE_WHEEL_OF_FORTUNE: {
    // TODO: With a 1 in 4 chance, give a random Joker a random edition
    // from Foil, Holographic, or Polychrome.
    break;
  }
  case TarotType::STRENGTH: {
    // TODO: Increase the rank of up to 2 selected cards by 1.
    break;
  }
  case TarotType::THE_HANGED_MAN: {
    // TODO: Destroy up to 2 selected cards.
    break;
  }
  case TarotType::DEATH: {
    if (selected_indices.size() != 2) {
      return;
    }
    const auto left = std::min(selected_indices[0], selected_indices[1]);
    const auto right = std::max(selected_indices[0], selected_indices[1]);
    if (left == right || right >= hand.size()) {
      return;
    }

    // Copy all card properties; left/right refers to position in the hand.
    hand[left] = hand[right];
    state.last_used_card = clone();
    break;
  }
  case TarotType::TEMPERANCE: {
    int payout = 0;
    for (const Joker *joker : state.jokers) {
      if (joker != nullptr) {
        payout += std::clamp(joker->sell_price, 0, 50 - payout);
      }
    }
    state.money += payout;
    state.last_used_card = clone();
    break;
  }
  case TarotType::THE_DEVIL: {
    // TODO: Enhance 1 selected card into a Gold Card.
    break;
  }
  case TarotType::THE_TOWER: {
    // TODO: Enhance 1 selected card into a Stone Card.
    break;
  }
  case TarotType::THE_STAR: {
    change_selected_suits(hand, selected_indices, Suit::DIAMONDS);
    state.last_used_card = clone();
    break;
  }
  case TarotType::THE_MOON: {
    change_selected_suits(hand, selected_indices, Suit::CLUBS);
    state.last_used_card = clone();
    break;
  }
  case TarotType::THE_SUN: {
    change_selected_suits(hand, selected_indices, Suit::HEARTS);
    state.last_used_card = clone();
    break;
  }
  case TarotType::JUDGEMENT: {
    // TODO: Create a random Joker if there is a free Joker slot.
    break;
  }
  case TarotType::THE_WORLD: {
    change_selected_suits(hand, selected_indices, Suit::SPADES);
    state.last_used_card = clone();
    break;
  }
  }
}
