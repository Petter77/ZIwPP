#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "algorithms/bruteforce.h"
#include "algorithms/identity.h"
#include "algorithms/neh.h"
#include "algorithms/sdst_eval.h"
#include "instance.h"

namespace {

// Sprawdza, czy wektor zawiera dokładnie wszystkie zadania 0..n-1 bez powtórzeń.
bool isValidPermutation(const std::vector<int> &perm, int n) {
  if (static_cast<int>(perm.size()) != n) {
    return false;
  }
  std::vector<char> seen(n, 0);
  for (int job : perm) {
    if (job < 0 || job >= n || seen[job]) {
      return false;
    }
    seen[job] = 1;
  }
  return true;
}

// Niezależny, referencyjny przegląd zupełny oparty o std::next_permutation (tylko do weryfikacji w testach).
long long referenceBruteForceMakespan(const Instance &inst) {
  std::vector<int> perm(inst.n);
  std::iota(perm.begin(), perm.end(), 0);

  long long best = std::numeric_limits<long long>::max();
  do {
    long long cmax = evaluateMakespan(inst, perm);
    if (cmax < best) {
      best = cmax;
    }
  } while (std::next_permutation(perm.begin(), perm.end()));

  return best;
}

// Pomocnicza funkcja budująca deterministyczną instancję testową (w ten sam sposób co generator.cpp).
Instance makeTestInstance(int n, int m, unsigned seed, int pMax = 99, int sMax = 50) {
  std::mt19937 rng(seed);
  auto randomValue = [&](int maxVal) { return 1 + static_cast<int>(rng() % maxVal); };

  Instance inst;
  inst.n = n;
  inst.m = m;
  inst.p.assign(m, std::vector<int>(n));
  inst.s.assign(m, std::vector<std::vector<int>>(n, std::vector<int>(n)));

  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      inst.p[i][j] = randomValue(pMax);
    }
  }
  for (int i = 0; i < m; ++i) {
    for (int j = 0; j < n; ++j) {
      for (int k = 0; k < n; ++k) {
        inst.s[i][j][k] = randomValue(sMax);
      }
    }
  }
  return inst;
}

void check(bool condition, const std::string &message) {
  if (!condition) {
    throw std::runtime_error("NIEUDANY TEST: " + message);
  }
}

// Test 1: Przykład 3x2 z pliku docs/format.md (wartości policzone ręcznie).
void testDocsFormatExample() {
  std::istringstream input(
      "3 2\n"
      "10 20 30\n"
      "15 25 5\n"
      "1 2 3\n"
      "2 1 3\n"
      "3 2 1\n"
      "4 5 6\n"
      "5 4 6\n"
      "6 5 4\n");

  Instance inst = Instance::parse(input);

  // Weryfikujemy ręcznie policzone wartości dla wszystkich 6 permutacji.
  check(evaluateMakespan(inst, {0, 1, 2}) == 71, "format.md [0,1,2] powinno dać 71");
  check(evaluateMakespan(inst, {0, 2, 1}) == 91, "format.md [0,2,1] powinno dać 91");
  check(evaluateMakespan(inst, {1, 0, 2}) == 77, "format.md [1,0,2] powinno dać 77");
  check(evaluateMakespan(inst, {1, 2, 0}) == 82, "format.md [1,2,0] powinno dać 82");
  check(evaluateMakespan(inst, {2, 0, 1}) == 91, "format.md [2,0,1] powinno dać 91");
  check(evaluateMakespan(inst, {2, 1, 0}) == 98, "format.md [2,1,0] powinno dać 98");

  BruteForce bf;
  Result bfRes = bf.run(inst, {});
  check(isValidPermutation(bfRes.permutation, inst.n), "BruteForce zwrócił niepoprawną permutację");
  check(evaluateMakespan(inst, bfRes.permutation) == 71, "BruteForce powinien znaleźć optimum 71");

  Neh neh;
  Result nehRes = neh.run(inst, {});
  check(isValidPermutation(nehRes.permutation, inst.n), "NEH zwrócił niepoprawną permutację");
  check(evaluateMakespan(inst, nehRes.permutation) >= 71, "NEH nie może mieć wyniku lepszego od optimum");

  std::cout << "[OK] Test 1: przykład 3x2 z docs/format.md (optimum=71, NEH="
            << evaluateMakespan(inst, nehRes.permutation) << ")\n";
}

// Test 2: Asymetria przezbrojeń — kolejność [1, 0] musi być wyraźnie lepsza od [0, 1].
void testAsymmetricSetup() {
  std::istringstream input(
      "2 2\n"
      "10 10\n"
      "10 10\n"
      "1 50\n"
      "1 1\n"
      "1 50\n"
      "1 1\n");

  Instance inst = Instance::parse(input);
  long long c01 = evaluateMakespan(inst, {0, 1}); // [0,1]: wysokie przezbrojenie 0->1 = 50
  long long c10 = evaluateMakespan(inst, {1, 0}); // [1,0]: niskie przezbrojenie 1->0 = 1
  check(c10 < c01, "Kolejność [1, 0] powinna być krótsza niż [0, 1]");

  BruteForce bf;
  Neh neh;
  Result bfRes = bf.run(inst, {});
  Result nehRes = neh.run(inst, {});

  check(bfRes.permutation == std::vector<int>({1, 0}), "BruteForce powinien wybrać [1, 0]");
  check(nehRes.permutation == std::vector<int>({1, 0}), "NEH powinien wybrać [1, 0]");

  std::cout << "[OK] Test 2: asymetria przezbrojeń (Cmax [0,1]=" << c01 << ", [1,0]=" << c10 << ")\n";
}

// Test 3: Przypadki brzegowe (1x1, 1x4, 4x1).
void testEdgeCases() {
  BruteForce bf;
  Neh neh;

  for (auto [n, m] : std::vector<std::pair<int, int>>{{1, 1}, {1, 4}, {4, 1}}) {
    Instance inst = makeTestInstance(n, m, 77);
    Result bfRes = bf.run(inst, {});
    Result nehRes = neh.run(inst, {});

    check(isValidPermutation(bfRes.permutation, n), "BruteForce: niepoprawna permutacja (brzeg)");
    check(isValidPermutation(nehRes.permutation, n), "NEH: niepoprawna permutacja (brzeg)");

    long long refOpt = referenceBruteForceMakespan(inst);
    long long bfCmax = evaluateMakespan(inst, bfRes.permutation);
    long long nehCmax = evaluateMakespan(inst, nehRes.permutation);

    check(bfCmax == refOpt, "BruteForce nie zgadza się z referencją w przypadku brzegowym");
    check(nehCmax >= refOpt, "NEH zwrócił wynik niższy niż optimum");
  }

  std::cout << "[OK] Test 3: przypadki brzegowe (1x1, 1x4, 4x1)\n";
}

// Test 4: Krzyżowa weryfikacja BruteForce (DFS przyrostowy) z std::next_permutation oraz porównanie z NEH.
void testCrossVerificationAndQuality() {
  Identity id;
  Neh neh;
  BruteForce bf;

  struct Case {
    int n;
    int m;
    unsigned seed;
  };
  std::vector<Case> cases = {
      {5, 3, 10}, {6, 4, 20}, {7, 3, 30}, {7, 5, 42}, {8, 4, 99}
  };

  for (const auto &c : cases) {
    Instance inst = makeTestInstance(c.n, c.m, c.seed);

    Result idRes = id.run(inst, {});
    Result nehRes = neh.run(inst, {});
    Result bfRes = bf.run(inst, {});

    check(isValidPermutation(nehRes.permutation, c.n), "NEH: niepoprawna permutacja");
    check(isValidPermutation(bfRes.permutation, c.n), "BruteForce: niepoprawna permutacja");

    long long idCmax = evaluateMakespan(inst, idRes.permutation);
    long long nehCmax = evaluateMakespan(inst, nehRes.permutation);
    long long bfCmax = evaluateMakespan(inst, bfRes.permutation);

    // Dla n <= 7 krzyżowo porównujemy DFS z niezależnym std::next_permutation.
    if (c.n <= 7) {
      long long refOpt = referenceBruteForceMakespan(inst);
      check(bfCmax == refOpt, "BruteForce DFS != next_permutation");
    }

    check(bfCmax <= nehCmax, "BruteForce musi być <= NEH");
    check(bfCmax <= idCmax, "BruteForce musi być <= Identity");

    double gapPercent = 100.0 * static_cast<double>(nehCmax - bfCmax) / static_cast<double>(bfCmax);
    std::cout << "  instancja " << c.n << "x" << c.m << " (seed=" << c.seed << "): "
              << "Identity=" << idCmax
              << ", NEH=" << nehCmax
              << ", BruteForce(OPT)=" << bfCmax
              << " (błąd NEH: " << gapPercent << "%)\n";
  }

  std::cout << "[OK] Test 4: krzyżowa weryfikacja BruteForce i jakość NEH\n";
}

// Test 5: Zabezpieczenie limitu max_n w BruteForce.
void testBruteForceLimit() {
  BruteForce bf;

  // Domyślny limit to 11 -> dla n = 12 powinien rzucić wyjątek.
  Instance tooBig = makeTestInstance(12, 2, 1);
  bool threwDefault = false;
  try {
    bf.run(tooBig, {});
  } catch (const std::runtime_error &) {
    threwDefault = true;
  }
  check(threwDefault, "BruteForce powinien rzucić wyjątek dla n = 12 przy domyślnym limicie");

  // Niestandardowy limit w parametrach: max_n = 4 -> dla n = 5 powinien rzucić wyjątek.
  Instance sizeFive = makeTestInstance(5, 2, 2);
  bool threwCustom = false;
  try {
    bf.run(sizeFive, {{"max_n", 4}});
  } catch (const std::runtime_error &) {
    threwCustom = true;
  }
  check(threwCustom, "BruteForce powinien respektować parametr max_n");

  std::cout << "[OK] Test 5: limit rozmiaru max_n w BruteForce\n";
}

// Test 6: Wydajność i poprawność NEH na większych instancjach (50x10, 100x20).
void testNehScalability() {
  Identity id;
  Neh neh;

  for (auto [n, m] : std::vector<std::pair<int, int>>{{50, 10}, {100, 20}}) {
    Instance inst = makeTestInstance(n, m, 42);

    auto start = std::chrono::steady_clock::now();
    Result nehRes = neh.run(inst, {});
    auto end = std::chrono::steady_clock::now();
    auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    check(isValidPermutation(nehRes.permutation, n), "NEH: niepoprawna permutacja na dużej instancji");

    long long idCmax = evaluateMakespan(inst, id.run(inst, {}).permutation);
    long long nehCmax = evaluateMakespan(inst, nehRes.permutation);

    check(nehCmax < idCmax, "NEH powinien wyraźnie poprawić wynik względem Identity na dużej instancji");

    std::cout << "  duża instancja " << n << "x" << m << ": "
              << "Identity=" << idCmax
              << ", NEH=" << nehCmax
              << " (czas NEH: " << elapsedMs << " ms)\n";
  }

  std::cout << "[OK] Test 6: wydajność i jakość NEH na dużych instancjach\n";
}

} // namespace

int main() {
  try {
    std::cout << "=== Uruchamianie testów algorytmów NEH i BruteForce ===\n";
    testDocsFormatExample();
    testAsymmetricSetup();
    testEdgeCases();
    testCrossVerificationAndQuality();
    testBruteForceLimit();
    testNehScalability();
    std::cout << "=== Wszystkie testy zakończone sukcesem! ===\n";
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "BŁĄD TESTÓW: " << e.what() << "\n";
    return 1;
  }
}
