#pragma once

#include <map>
#include <string>
#include <vector>

#include "instance.h"

// Parametry algorytmu: nazwa -> wartość.
using Params = std::map<std::string, double>;

// Wynik algorytmu: kolejność zadań (indeksy od 0), ta sama na każdej maszynie.
struct Result {
  std::vector<int> permutation;
};

// Klasa bazowa algorytmów. Każdy algorytm to osobna klasa w osobnym pliku (algorithms/).
// Algorytm nie wczytuje ani nie zapisuje danych: dostaje Instance i zwraca Result.
class Algorithm {
public:
  virtual ~Algorithm() = default;

  virtual std::string name() const = 0;
  virtual Result run(const Instance &inst, const Params &params) const = 0;
};
