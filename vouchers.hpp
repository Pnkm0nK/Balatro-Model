#pragma once

#include "core.hpp"
#include <memory>
#include <vector>

// 1. Overstock: +1 card slot available in shop (to 3 slots)
class Overstock : public Voucher {
public:
  Overstock();
  void activate(GameState &state) override;
};

// 2. Clearance Sale: All cards and packs in shop are 25% off
class ClearanceSale : public Voucher {
public:
  ClearanceSale();
  void activate(GameState &state) override;
};

// 3. Hone: Foil, Holographic, and Polychrome cards appear 2X more often
class Hone : public Voucher {
public:
  Hone();
  void activate(GameState &state) override;
};

// 4. Reroll Surplus: Rerolls cost $2 less
class RerollSurplus : public Voucher {
public:
  RerollSurplus();
  void activate(GameState &state) override;
};

// 5. Crystal Ball: +1 consumable slot
class CrystalBall : public Voucher {
public:
  CrystalBall();
  void activate(GameState &state) override;
};

// 6. Telescope: Celestial Packs always contain the Planet card for your most played poker hand
class Telescope : public Voucher {
public:
  Telescope();
  void activate(GameState &state) override;
};

// 7. Grabber: Permanently gain +1 hand per round
class Grabber : public Voucher {
public:
  Grabber();
  void activate(GameState &state) override;
};

// 8. Wasteful: Permanently gain +1 discard each round
class Wasteful : public Voucher {
public:
  Wasteful();
  void activate(GameState &state) override;
};

// 9. Tarot Merchant: Tarot cards appear 2X more frequently in the shop
class TarotMerchant : public Voucher {
public:
  TarotMerchant();
  void activate(GameState &state) override;
};

// 10. Planet Merchant: Planet cards appear 2X more frequently in the shop
class PlanetMerchant : public Voucher {
public:
  PlanetMerchant();
  void activate(GameState &state) override;
};

// 11. Seed Money: Raise the cap on interest earned in each round to $10
class SeedMoney : public Voucher {
public:
  SeedMoney();
  void activate(GameState &state) override;
};

// 12. Blank: Does nothing?
class Blank : public Voucher {
public:
  Blank();
  void activate(GameState &state) override;
};

// 13. Magic Trick: Playing cards can be purchased from the shop
class MagicTrick : public Voucher {
public:
  MagicTrick();
  void activate(GameState &state) override;
};

// 14. Hieroglyph: -1 Ante, -1 hand each round
class Hieroglyph : public Voucher {
public:
  Hieroglyph();
  void activate(GameState &state) override;
};

// 15. Director's Cut: Reroll Boss Blind 1 time per Ante, $10 per roll
class DirectorsCut : public Voucher {
public:
  DirectorsCut();
  void activate(GameState &state) override;
};

// 16. Paint Brush: +1 hand size
class PaintBrush : public Voucher {
public:
  PaintBrush();
  void activate(GameState &state) override;
};

// Factory function to create instances of all base vouchers
std::vector<std::unique_ptr<Voucher>> create_all_base_vouchers();
