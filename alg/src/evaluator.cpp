#include "evaluator.h"

#include <algorithm>

long long makespan(const Instance &inst, const std::vector<int> &permutation) {
  // end[i] - chwila zakończenia ostatniego dotąd zadania na maszynie i
  std::vector<long long> end(inst.m, 0);
  int previous = -1;

  for (int job : permutation) {
    // pierwsze zadanie ma przezbrojenie s[i][job][job]
    int from = previous < 0 ? job : previous;
    long long arrival = 0; // chwila, w której zadanie schodzi z poprzedniej maszyny

    for (int i = 0; i < inst.m; ++i) {
      long long setupDone = end[i] + inst.s[i][from][job];
      end[i] = std::max(setupDone, arrival) + inst.p[i][job];
      arrival = end[i];
    }
    previous = job;
  }
  return end.back();
}
