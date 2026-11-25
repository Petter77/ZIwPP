#pragma once

#include "algorithm.h"

// Algorytm dokładny przeglądu zupełnego (Brute Force) dla problemu PFSP-SDST.
// Sprawdza wszystkie n! permutacji zadań i gwarantuje znalezienie optimum globalnego.
// Ze względu na złożoność silniową posiada zabezpieczenie przed zbyt dużym n (domyślnie max_n = 11).
class BruteForce : public Algorithm {
public:
  std::string name() const override { return "bruteforce"; }
  Result run(const Instance &inst, const Params &params) const override;
};
