#include <iostream>
#include <memory>
#include <string>

#include "algorithms/bruteforce.h"
#include "algorithms/identity.h"
#include "algorithms/neh.h"
#include "instance.h"

int main(int argc, char **argv) {
  if (argc < 2 || argc > 3) {
    std::cerr << "Użycie: " << argv[0] << " <plik_instancji> [identity|neh|bruteforce]\n";
    return 1;
  }

  std::string algoName = (argc == 3) ? argv[2] : "identity";
  std::unique_ptr<Algorithm> algorithm;
  if (algoName == "identity") {
    algorithm = std::make_unique<Identity>();
  } else if (algoName == "neh") {
    algorithm = std::make_unique<Neh>();
  } else if (algoName == "bruteforce") {
    algorithm = std::make_unique<BruteForce>();
  } else {
    std::cerr << "Nieznany algorytm: " << algoName << " (dostępne: identity, neh, bruteforce)\n";
    return 1;
  }

  try {
    Instance inst = Instance::load(argv[1]);
    std::cout << "Wczytano instancję: " << inst.n << " zadań, " << inst.m << " maszyn\n";

    Result result = algorithm->run(inst, {});
    std::cout << "Algorytm: " << algorithm->name() << "\nPermutacja:";
    for (int job : result.permutation)
      std::cout << ' ' << job;
    std::cout << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Błąd: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
