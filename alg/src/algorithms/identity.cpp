#include "algorithms/identity.h"

#include <numeric>

Result Identity::run(const Instance &inst, const Params &) const {
  Result result;
  result.permutation.resize(inst.n);
  std::iota(result.permutation.begin(), result.permutation.end(), 0);
  return result;
}
