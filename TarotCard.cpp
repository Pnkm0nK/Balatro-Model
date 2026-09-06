#include "core.hpp"
#include <algorithm>

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

void TarotCard::activate(GameState &state) {
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
    // TODO: Create up to 2 random Tarots if there is room.
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
    // TODO: Turn the left selected card into the right selected card.
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
    // TODO: Convert up to 3 selected cards to Diamonds.
    break;
  }
  case TarotType::THE_MOON: {
    // TODO: Convert up to 3 selected cards to Clubs.
    break;
  }
  case TarotType::THE_SUN: {
    // TODO: Convert up to 3 selected cards to Hearts.
    break;
  }
  case TarotType::JUDGEMENT: {
    // TODO: Create a random Joker if there is a free Joker slot.
    break;
  }
  case TarotType::THE_WORLD: {
    // TODO: Convert up to 3 selected cards to Spades.
    break;
  }
  }
}
