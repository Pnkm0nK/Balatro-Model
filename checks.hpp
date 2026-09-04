#pragma once

#include "core.hpp"
#include <array>
#include <vector>

std::vector<std::array<const Card *, 5>>
find_flush(const std::array<std::vector<const Card *>, ALL_SUITS.size()> &hand,
           std::size_t threshold = 5);

std::vector<std::array<const Card *, 5>> find_straight(
    const std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> &hand,
    std::size_t threshold = 5);

std::vector<std::array<const Card *, 2>> find_pairs(
    const std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> &hand);
std::vector<std::array<const Card *, 3>> find_triples(
    const std::array<std::vector<const Card *>, ALL_CARD_RANKS.size()> &hand);
std::vector<std::array<const Card *, 5>>
find_full_house(const std::vector<std::array<const Card *, 2>> &pairs,
                const std::vector<std::array<const Card *, 3>> &triplets);
