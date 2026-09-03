//Cheks is this flush
#include "../Card.cpp"
#include <array>
#include <vector>

bool is_flush(const std::array<std::vector<const Card *>, ALL_SUITS.size()> &hand,
           int threshold = 5) {
    for (size_t i = 0; i < ALL_SUITS.size(); ++i) {
      if (hand[i].size() >= threshold) {
        return true;
      }
    }
    return false;
  }
