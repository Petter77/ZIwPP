#pragma once

#include "algorithm.h"

// Algorytm wzorcowy: zwraca zadania w kolejności z pliku (0, 1, ..., n-1).
// Pokazuje, jak wygląda klasa pochodna, i służy jako punkt odniesienia dla innych algorytmów.
class Identity : public Algorithm {
public:
  std::string name() const override { return "identity"; }
  Result run(const Instance &inst, const Params &params, const Logger &log) const override;
};
