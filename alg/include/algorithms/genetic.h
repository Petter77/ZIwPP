#pragma once

#include "algorithm.h"

// Algorytm genetyczny z elitaryzmem, lokalnym przeszukiwaniem elity i adaptacyjną mutacją.
// Parametry (nazwy w Params) i ich opis: docs/genetyczny.md.
class Genetic : public Algorithm {
public:
  std::string name() const override { return "genetic"; }
  // Rzuca std::invalid_argument przy nieznanym parametrze lub złej wartości.
  Result run(const Instance &inst, const Params &params, const Logger &log) const override;
};
