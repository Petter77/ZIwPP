#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

#include "instance.h"

// Pomocnicza funkcja obliczająca makespan (Cmax) dla podanej permutacji (pełnej lub częściowej).
// Realizuje dokładnie wzór z docs/format.md (wariant przezbrojeń anticipatory):
//   C[i][k] = max( C[i][k-1] + s[i][perm[k-1]][perm[k]], C[i-1][k] ) + p[i][perm[k]]
// Dla pierwszego zadania (k == 0) przyjmujemy przezbrojenie początkowe s[i][perm[0]][perm[0]].
inline long long evaluateMakespan(const Instance &inst, const std::vector<int> &perm) {
  if (inst.m <= 0 || perm.empty()) {
    return 0;
  }

  // machineTime[i] przechowuje czas zakończenia poprzedniego zadania na maszynie i.
  std::vector<long long> machineTime(inst.m, 0);

  for (std::size_t k = 0; k < perm.size(); ++k) {
    int currentJob = perm[k];
    // Dla pierwszego zadania na maszynie zakładamy j = k (zgodnie z treścią Tematu 2).
    int prevJob = (k == 0) ? currentJob : perm[k - 1];

    for (int i = 0; i < inst.m; ++i) {
      // Chwila, w której maszyna i kończy przezbrojenie przed bieżącym zadaniem.
      long long readyMachine = machineTime[i] + inst.s[i][prevJob][currentJob];
      // Chwila, w której bieżące zadanie kończy operację na poprzedniej maszynie (0 dla pierwszej).
      long long readyJob = (i == 0) ? 0 : machineTime[i - 1];

      // Operacja może się zacząć dopiero, gdy gotowe są jednocześnie maszyna i zadanie.
      machineTime[i] = std::max(readyMachine, readyJob) + inst.p[i][currentJob];
    }
  }

  // Makespan to czas zakończenia ostatniego zadania na ostatniej maszynie.
  return machineTime[inst.m - 1];
}
