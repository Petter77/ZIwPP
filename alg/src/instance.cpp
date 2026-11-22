#include "instance.h"
#include <fstream>
#include <stdexcept>

namespace {

// Wczytuje jedną dodatnią liczbę; name i indeksy trafiają do komunikatu błędu.
int readValue(std::istream &in, const char *name, int a = -1, int b = -1, int c = -1) {
  int value;
  if (!(in >> value) || value < 1) {
    std::string where = name;
    for (int idx : {a, b, c})
      if (idx >= 0)
        where += "[" + std::to_string(idx) + "]";
    throw std::runtime_error("Błąd wczytywania instancji: brak lub niepoprawna wartość w " + where);
  }
  return value;
}

} // namespace

Instance Instance::parse(std::istream &in) {
  Instance inst;
  inst.n = readValue(in, "n");
  inst.m = readValue(in, "m");

  inst.p.assign(inst.m, std::vector<int>(inst.n));
  for (int i = 0; i < inst.m; ++i)
    for (int j = 0; j < inst.n; ++j)
      inst.p[i][j] = readValue(in, "p", i, j);

  inst.s.assign(inst.m, std::vector<std::vector<int>>(inst.n, std::vector<int>(inst.n)));
  for (int i = 0; i < inst.m; ++i)
    for (int j = 0; j < inst.n; ++j)
      for (int k = 0; k < inst.n; ++k)
        inst.s[i][j][k] = readValue(in, "s", i, j, k);

  int extra;
  if (in >> extra)
    throw std::runtime_error("Błąd wczytywania instancji: nadmiarowe dane na końcu pliku");

  return inst;
}

Instance Instance::load(const std::string &path) {
  std::ifstream f(path);
  if (!f)
    throw std::runtime_error("Nie można otworzyć pliku: " + path);
  return parse(f);
}

void Instance::save(const std::string &path) const {
  std::ofstream f(path);
  if (!f)
    throw std::runtime_error("Nie można zapisać pliku: " + path);

  f << n << ' ' << m << "\n\n";
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j)
      f << (j ? " " : "") << p[i][j];
    f << '\n';
  }
  f << '\n';
  for (int i = 0; i < m; ++i)
    for (int j = 0; j < n; ++j) {
      for (int k = 0; k < n; ++k)
        f << (k ? " " : "") << s[i][j][k];
      f << '\n';
    }
}
