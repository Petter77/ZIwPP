#include "algorithms/neh.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <string>
#include <vector>

#include "evaluator.h"

Result Neh::run(const Instance &inst, const Params &, const Logger &log) const {
  Result result;
  if (inst.n <= 0) {
    return result;
  }
  if (inst.n == 1) {
    result.permutation = {0};
    result.makespan = makespan(inst, result.permutation);
    return result;
  }

  // Krok 1: Obliczamy łączny czas przetwarzania każdego zadania na wszystkich maszynach.
  std::vector<long long> totalProcessingTime(inst.n, 0);
  for (int j = 0; j < inst.n; ++j) {
    for (int i = 0; i < inst.m; ++i) {
      totalProcessingTime[j] += inst.p[i][j];
    }
  }

  // Tworzymy listę zadań [0, 1, ..., n-1] i sortujemy ją malejąco według łącznego czasu.
  // Przy remisie decyduje mniejszy numer zadania, aby wynik był zawsze powtarzalny.
  std::vector<int> sortedJobs(inst.n);
  for (int j = 0; j < inst.n; ++j) {
    sortedJobs[j] = j;
  }

  std::sort(sortedJobs.begin(), sortedJobs.end(), [&](int a, int b) {
    if (totalProcessingTime[a] != totalProcessingTime[b]) {
      return totalProcessingTime[a] > totalProcessingTime[b];
    }
    return a < b;
  });

  log("zadania posortowane malejąco po sumie czasów operacji");

  // Krok 2: Budujemy permutację przyrostowo — zaczynamy od zadania o największej sumie czasów.
  std::vector<int> currentPerm;
  currentPerm.reserve(inst.n);
  currentPerm.push_back(sortedJobs[0]);

  std::vector<int> candidate;
  candidate.reserve(inst.n);

  for (int step = 1; step < inst.n; ++step) {
    int nextJob = sortedJobs[step];
    int bestMakespan = std::numeric_limits<int>::max();
    std::size_t bestPos = 0;

    // Sprawdzamy wszystkie możliwe miejsca wstawienia kolejnego zadania (od początku do końca).
    for (std::size_t pos = 0; pos <= currentPerm.size(); ++pos) {
      candidate = currentPerm;
      candidate.insert(candidate.begin() + static_cast<std::ptrdiff_t>(pos), nextJob);

      int currentMakespan = makespan(inst, candidate);
      if (currentMakespan < bestMakespan) {
        bestMakespan = currentMakespan;
        bestPos = pos;
      }
    }

    // Zostawiamy zadanie na pozycji, która dała najmniejszy częściowy makespan.
    currentPerm.insert(currentPerm.begin() + static_cast<std::ptrdiff_t>(bestPos), nextJob);
    log("krok " + std::to_string(step + 1) + "/" + std::to_string(inst.n) + ": zadanie " + std::to_string(nextJob) +
        " na pozycję " + std::to_string(bestPos) + ", częściowy makespan " + std::to_string(bestMakespan));
  }

  result.permutation = std::move(currentPerm);
  result.makespan = makespan(inst, result.permutation);
  return result;
}
