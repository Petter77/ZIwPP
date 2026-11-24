#include <filesystem>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>

#include "instance.h"

// Generator instancji: osobny program, nie jest częścią algorytmów.
// Użycie: generator <n> <m> <ziarno> <nazwa_pliku> [p_max] [s_max]
int main(int argc, char **argv) {
  if (argc < 5 || argc > 7) {
    std::cerr << "Użycie: " << argv[0] << " <n> <m> <ziarno> <nazwa_pliku> [p_max] [s_max]\n";
    return 1;
  }

  try {
    int n = std::stoi(argv[1]);
    int m = std::stoi(argv[2]);
    unsigned seed = static_cast<unsigned>(std::stoul(argv[3]));
    std::string path = std::string("instances/") + argv[4];
    int pMax = argc > 5 ? std::stoi(argv[5]) : 99;
    int sMax = argc > 6 ? std::stoi(argv[6]) : 50;
    if (n < 1 || m < 1 || pMax < 1 || sMax < 1)
      throw std::runtime_error("n, m, p_max i s_max muszą być dodatnie");

    // Własne mapowanie zamiast std::uniform_int_distribution, żeby to samo ziarno
    // dawało ten sam plik niezależnie od kompilatora.
    std::mt19937 rng(seed);
    auto randomValue = [&](int max) { return 1 + static_cast<int>(rng() % max); };

    Instance inst;
    inst.n = n;
    inst.m = m;
    inst.p.assign(m, std::vector<int>(n));
    inst.s.assign(m, std::vector<std::vector<int>>(n, std::vector<int>(n)));
    for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j)
        inst.p[i][j] = randomValue(pMax);
    for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j)
        for (int k = 0; k < n; ++k)
          inst.s[i][j][k] = randomValue(sMax);

    std::filesystem::create_directories("instances");
    inst.save(path);
    std::cout << "Zapisano instancję " << n << " x " << m << " do " << path << "\n";
  } catch (const std::invalid_argument &) {
    std::cerr << "Błąd: n, m, ziarno, p_max i s_max muszą być liczbami całkowitymi\n";
    return 1;
  } catch (const std::exception &e) {
    std::cerr << "Błąd: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
