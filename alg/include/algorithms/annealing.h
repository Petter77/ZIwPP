#pragma once

#include "algorithm.h"

// Symulowane wyżarzanie z automatycznym doborem temperatury i dwoma rodzajami ruchów.
// Parametry (nazwy w Params) i ich opis: docs/wyzarzanie.md.
class Annealing : public Algorithm {
public:
  std::string name() const override { return "sa"; }
  // Rzuca std::invalid_argument przy nieznanym parametrze lub złej wartości.
  Result run(const Instance &inst, const Params &params, const Logger &log) const override;
};
