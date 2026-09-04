#include "core.hpp"

Card::Card(Suit suit, CardRank rank, Edition edition, Enhancement enhancement,
           Seal seal)
    : suit(suit),
      rank(rank),
      edition(edition),
      enhancement(enhancement),
      seal(seal),
      chips(base_chips[static_cast<std::size_t>(rank)]) {}