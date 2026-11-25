#include <iostream>
#include <string>

#include "algorithms/identity.h"
#include "instance.h"

int main(int argc, char **argv) {
  std::string path;
  bool verbose = false;
  bool badUsage = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose")
      verbose = true;
    else if (arg[0] != '-' && path.empty())
      path = arg;
    else
      badUsage = true; // nieznana flaga albo drugi plik
  }
  if (badUsage || path.empty()) {
    std::cerr << "Użycie: " << argv[0] << " <plik_instancji> [--verbose]\n";
    return 1;
  }

  try {
    Instance inst = Instance::load(path);
    std::cout << "Wczytano instancję: " << inst.n << " zadań, " << inst.m << " maszyn\n";

    Identity algorithm;
    Logger log = noLog;
    if (verbose)
      log = [&](const std::string &message) { std::cerr << "[" << algorithm.name() << "] " << message << "\n"; };

    Result result = algorithm.run(inst, {}, log);
    std::cout << "Algorytm: " << algorithm.name() << "\nPermutacja:";
    for (int job : result.permutation)
      std::cout << ' ' << job;
    std::cout << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Błąd: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
