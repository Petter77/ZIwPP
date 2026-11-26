#include "algorithms/bruteforce.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

// Domyślny limit rozmiaru instancji
constexpr int DEFAULT_MAX_N = 11;

// Rekurencyjnie przegląda wszystkie n! permutacji.
// Zamiast przeliczać całą permutację od zera dla każdego liścia, aktualizuje czasy zakończenia
// przyrostowo — tylko dla bieżącej głębokości 'depth' (jedna kolumna macierzy C).
void searchPermutations(const Instance &inst,
                        int depth,
                        std::vector<int> &currentPerm,
                        std::vector<char> &used,
                        std::vector<std::vector<long long>> &completion,
                        long long &bestMakespan,
                        std::vector<int> &bestPerm,
                        const Logger &log) {
  // Jeśli wstawiliśmy już wszystkie n zadań, sprawdzamy uzyskany makespan.
  if (depth == inst.n) {
    long long makespan = completion[inst.n - 1][inst.m - 1];
    if (makespan < bestMakespan) {
      bestMakespan = makespan;
      bestPerm = currentPerm;
      log("nowy najlepszy " + std::to_string(bestMakespan));
    }
    return;
  }

  // Próbujemy wstawić na pozycję 'depth' każde jeszcze nieużyte zadanie.
  for (int job = 0; job < inst.n; ++job) {
    if (used[job]) {
      continue;
    }

    used[job] = 1;
    currentPerm[depth] = job;

    // Dla pierwszego zadania (depth == 0) przezbrojenie początkowe wynosi s[i][job][job].
    int prevJob = (depth == 0) ? job : currentPerm[depth - 1];

    // Wyznaczamy czasy zakończenia tego zadania na każdej maszynie i = 0 .. m-1.
    for (int i = 0; i < inst.m; ++i) {
      long long prevOnMachine = (depth == 0) ? 0 : completion[depth - 1][i];
      long long readyMachine = prevOnMachine + inst.s[i][prevJob][job];
      long long readyJob = (i == 0) ? 0 : completion[depth][i - 1];

      completion[depth][i] = std::max(readyMachine, readyJob) + inst.p[i][job];
    }

    // Schodzimy poziom głębiej, aby dobrać kolejne zadanie w permutacji.
    searchPermutations(inst, depth + 1, currentPerm, used, completion, bestMakespan, bestPerm, log);

    // Cofamy wybór zadania (backtracking), aby móc użyć go na innej pozycji.
    used[job] = 0;
  }
}

} // namespace

Result BruteForce::run(const Instance &inst, const Params &params, const Logger &log) const {
  Result result;
  if (inst.n <= 0) {
    return result;
  }

  // Sprawdzamy, czy użytkownik przekazał własny limit rozmiaru w parametrach.
  int maxN = DEFAULT_MAX_N;
  auto it = params.find("max_n");
  if (it != params.end()) {
    maxN = static_cast<int>(it->second);
  }

  if (inst.n > maxN) {
    throw std::runtime_error(
        "BruteForce: zbyt duża liczba zadań (n = " + std::to_string(inst.n) +
        ", limit max_n = " + std::to_string(maxN) + ")");
  }

  std::vector<int> currentPerm(inst.n, 0);
  std::vector<int> bestPerm(inst.n, 0);
  std::vector<char> used(inst.n, 0);
  // completion[depth][i] oznacza czas zakończenia zadania z pozycji 'depth' na maszynie 'i'.
  std::vector<std::vector<long long>> completion(inst.n, std::vector<long long>(inst.m, 0));
  long long bestMakespan = std::numeric_limits<long long>::max();

  log("przegląd wszystkich permutacji " + std::to_string(inst.n) + " zadań");
  searchPermutations(inst, 0, currentPerm, used, completion, bestMakespan, bestPerm, log);
  log("koniec: optimum " + std::to_string(bestMakespan));

  result.permutation = std::move(bestPerm);
  result.makespan = bestMakespan;
  return result;
}
