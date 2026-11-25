#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "instance.h"

// Parametry algorytmu: nazwa -> wartość.
using Params = std::map<std::string, double>;

// Funkcja, do której algorytm zgłasza, co robi (tryb --verbose). Algorytm nie wie, dokąd trafia tekst.
// Wywołujący zawsze przekazuje poprawną funkcję (noLog, gdy logowanie jest wyłączone).
using Logger = std::function<void(const std::string &)>;
inline const Logger noLog = [](const std::string &) {};

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
  virtual Result run(const Instance &inst, const Params &params, const Logger &log) const = 0;
};
