//Finds pairs
#include "../Card.cpp"
#include <array>
#include <vector>

std::vector<std::array<const Card *, 2>> find_pairs(const std::array<std::vector<const Card *>,
                                    ALL_CARD_RANKS.size()> &hand) {
    std::vector<std::array<const Card *, 2>> result;
    for(int i = 0; i < hand.size(); i++){
        if(hand[i].size() == 2){
            result.push_back({hand[i][0], hand[i][1]});
        }
    }
    return result;
}
