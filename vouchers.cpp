#include "vouchers.hpp"
#include <algorithm>

Overstock::Overstock() {
  this->name = "Overstock";
  this->buy_cost = 10;
}
void Overstock::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.shop_item_slots += 1;
}

ClearanceSale::ClearanceSale() {
  this->name = "Clearance Sale";
  this->buy_cost = 10;
}
void ClearanceSale::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.shop_discount += 0.25;
}

Hone::Hone() {
  this->name = "Hone";
  this->buy_cost = 10;
}
void Hone::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.edition_rate_mult *= 2.0;
}

RerollSurplus::RerollSurplus() {
  this->name = "Reroll Surplus";
  this->buy_cost = 10;
}
void RerollSurplus::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.reroll_discount += 2;
  state.reroll_cost = std::max(0, state.reroll_cost - 2);
}

CrystalBall::CrystalBall() {
  this->name = "Crystal Ball";
  this->buy_cost = 10;
}
void CrystalBall::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.max_inventory_slots += 1;
}

Telescope::Telescope() {
  this->name = "Telescope";
  this->buy_cost = 10;
}
void Telescope::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.telescope = true;
}

Grabber::Grabber() {
  this->name = "Grabber";
  this->buy_cost = 10;
}
void Grabber::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.hands += 1;
}

Wasteful::Wasteful() {
  this->name = "Wasteful";
  this->buy_cost = 10;
}
void Wasteful::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.discards += 1;
  state.discards_left += 1;
}

TarotMerchant::TarotMerchant() {
  this->name = "Tarot Merchant";
  this->buy_cost = 10;
}
void TarotMerchant::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.tarot_rate_mult *= 2.0;
}

PlanetMerchant::PlanetMerchant() {
  this->name = "Planet Merchant";
  this->buy_cost = 10;
}
void PlanetMerchant::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.planet_rate_mult *= 2.0;
}

SeedMoney::SeedMoney() {
  this->name = "Seed Money";
  this->buy_cost = 10;
}
void SeedMoney::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.interest_cap = 10;
}

Blank::Blank() {
  this->name = "Blank";
  this->buy_cost = 10;
}
void Blank::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  // Does nothing?
}

MagicTrick::MagicTrick() {
  this->name = "Magic Trick";
  this->buy_cost = 10;
}
void MagicTrick::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.can_buy_playing_cards_in_shop = true;
}

Hieroglyph::Hieroglyph() {
  this->name = "Hieroglyph";
  this->buy_cost = 10;
}
void Hieroglyph::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.ante = std::max(1, state.ante - 1);
  state.hands = std::max(1, state.hands - 1);
  state.hands_left = std::max(1, state.hands_left - 1);
}

DirectorsCut::DirectorsCut() {
  this->name = "Director's Cut";
  this->buy_cost = 10;
}
void DirectorsCut::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.boss_rerolls_per_ante += 1;
}

PaintBrush::PaintBrush() {
  this->name = "Paint Brush";
  this->buy_cost = 10;
}
void PaintBrush::activate(GameState &state) {
  state.redeemed_vouchers.push_back(this->name);
  state.max_cards_in_hand += 1;
}

std::vector<std::unique_ptr<Voucher>> create_all_base_vouchers() {
  std::vector<std::unique_ptr<Voucher>> list;
  list.reserve(16);
  list.push_back(std::make_unique<Overstock>());
  list.push_back(std::make_unique<ClearanceSale>());
  list.push_back(std::make_unique<Hone>());
  list.push_back(std::make_unique<RerollSurplus>());
  list.push_back(std::make_unique<CrystalBall>());
  list.push_back(std::make_unique<Telescope>());
  list.push_back(std::make_unique<Grabber>());
  list.push_back(std::make_unique<Wasteful>());
  list.push_back(std::make_unique<TarotMerchant>());
  list.push_back(std::make_unique<PlanetMerchant>());
  list.push_back(std::make_unique<SeedMoney>());
  list.push_back(std::make_unique<Blank>());
  list.push_back(std::make_unique<MagicTrick>());
  list.push_back(std::make_unique<Hieroglyph>());
  list.push_back(std::make_unique<DirectorsCut>());
  list.push_back(std::make_unique<PaintBrush>());
  return list;
}
