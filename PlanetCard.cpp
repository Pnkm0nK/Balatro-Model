#include "core.hpp"

PlanetCard::PlanetCard(Planet planet_type) : planet_type(planet_type) {
  this->chips_to_add =
      planet_chip_additions[static_cast<std::size_t>(planet_type)];
  this->mult_to_add =
      planet_mult_additions[static_cast<std::size_t>(planet_type)];
}

void PlanetCard::activate(
    GameState &game_state, std::vector<Card> & /*hand*/,
    const std::vector<std::size_t> & /*selected_indices*/) {
  game_state.hand_chips[static_cast<std::size_t>(planet_type)] += chips_to_add;
  game_state.hand_mult[static_cast<std::size_t>(planet_type)] += mult_to_add;
  game_state.last_used_card = this->clone();
}

std::unique_ptr<Item> PlanetCard::clone() const {
  return std::make_unique<PlanetCard>(*this);
}
