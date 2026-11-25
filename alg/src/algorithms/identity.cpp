#include "algorithms/identity.h"

#include <numeric>

#include "evaluator.h"

Result Identity::run(const Instance &inst, const Params &, const Logger &log) const {
  log("zwracam kolejność z pliku (" + std::to_string(inst.n) + " zadań)");

  Result result;
  result.permutation.resize(inst.n);
  std::iota(result.permutation.begin(), result.permutation.end(), 0);
  result.makespan = makespan(inst, result.permutation);
  return result;
}
