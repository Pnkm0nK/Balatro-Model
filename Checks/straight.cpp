//Cheks is this straight
#include "../Card.cpp"
#include <array>
#include <vector>

bool is_straight(const std::array<std::vector<const Card *>,
                                    ALL_CARD_RANKS.size()> &hand) {
    int count_for_straight = 5;
    int count = 0;
    for (size_t i = 0; i <= ALL_CARD_RANKS.size(); i++) {
      if (!hand[i].empty()) {
        count++;
      } else {
        count = 0;
      }
      if (count >= count_for_straight) {
        return true;
      }
    }
    return false;
}