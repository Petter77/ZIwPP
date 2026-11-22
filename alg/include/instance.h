#pragma once

#include <istream>
#include <string>
#include <vector>

// Instancja problemu: n zadań na m maszynach. Format pliku opisany w docs/format.md.
// Indeksy od 0. Po wczytaniu (parse/load) wszystkie wartości są dodatnie.
class Instance {
public:
  int n = 0; // liczba zadań
  int m = 0; // liczba maszyn
  std::vector<std::vector<int>> p;              // p[i][j] - czas zadania j na maszynie i
  std::vector<std::vector<std::vector<int>>> s; // s[i][j][k] - przezbrojenie na maszynie i z zadania j na k

  // Rzucają std::runtime_error przy błędnych danych lub braku pliku.
  static Instance parse(std::istream &in);
  static Instance load(const std::string &path);

  void save(const std::string &path) const;
};
