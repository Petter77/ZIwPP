#pragma once

#include "algorithm.h"

// Algorytm heurystyczny NEH dostosowany do problemu PFSP z przezbrojeniami zależnymi od kolejności (SDST).
// Sortuje zadania malejąco po sumie czasów operacji, a następnie buduje permutację przyrostowo,
// wstawiając każde kolejne zadanie na pozycję dającą najmniejszy makespan.
class Neh : public Algorithm {
public:
  std::string name() const override { return "neh"; }
  Result run(const Instance &inst, const Params &params, const Logger &log) const override;
};
