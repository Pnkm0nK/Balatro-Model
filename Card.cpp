#include "types.cpp"

const int base_chips[] = {2,3,4,5,6,7,8,9,10,10,10,10,11};

struct Card{
    public:
        Suit suit;
        CardRank rank;
        Edition edition;
        Enhancement enhancement;
        Seal seal;
        int chips;

        Card(Suit suit, CardRank rank,
             Edition edition = Edition::BASE,
             Enhancement enhancement = Enhancement::NONE,
             Seal seal = Seal::NONE)
            : suit(suit),
              rank(rank),
              edition(edition),
              enhancement(enhancement),
              seal(seal),
              chips(base_chips[static_cast<int>(rank)]) {}
};