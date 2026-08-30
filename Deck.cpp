#include "Card.cpp"
#include <vector>
#include "types.cpp"

class Deck{
    public:
        std::vector<Card> deck;
        Deck(){
            deck.reserve(ALL_SUITS.size() * ALL_CARD_RANKS.size());
            for(Suit suit : ALL_SUITS){
                for(CardRank rank : ALL_CARD_RANKS){
                    deck.emplace_back(suit, rank);
                }
            }
        }
        
};