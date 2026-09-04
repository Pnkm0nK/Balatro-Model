#include "../checks.hpp"
#include "../core.hpp"
#include <array>
#include <vector>

std::vector<std::array<const Card *, 5>>
find_full_house(const std::vector<std::array<const Card *, 2>> &pairs,
                const std::vector<std::array<const Card *, 3>> &triples) {
  std::vector<std::array<const Card *, 5>> full_houses;
  if (pairs.size() != 1 || triples.size() != 1) {
    return full_houses;
  } else {
    std::array<const Card *, 2> pair = pairs[0];
    std::array<const Card *, 3> triplet = triples[0];
    std::array<const Card *, 5> full_house = {pair[0], pair[1], triplet[0],
                                              triplet[1], triplet[2]};
    full_houses.push_back(full_house);
    return full_houses;
  }
}