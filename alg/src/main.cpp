#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "algorithms/annealing.h"
#include "algorithms/bruteforce.h"
#include "algorithms/genetic.h"
#include "algorithms/identity.h"
#include "algorithms/neh.h"
#include "instance.h"

// Dodaje do params tekst w postaci nazwa=liczba. Zwraca false, gdy tekst ma inną postać.
static bool addParam(const std::string &text, Params &params) {
  size_t eq = text.find('=');
  if (eq == std::string::npos || eq == 0)
    return false;
  std::string value = text.substr(eq + 1);
  try {
    size_t used = 0;
    double number = std::stod(value, &used);
    if (used != value.size())
      return false;
    params[text.substr(0, eq)] = number;
  } catch (const std::logic_error &) { // stod: to nie liczba albo poza zakresem
    return false;
  }
  return true;
}

static std::unique_ptr<Algorithm> makeAlgorithm(const std::string &name) {
  if (name == "genetic")
    return std::make_unique<Genetic>();
  if (name == "sa")
    return std::make_unique<Annealing>();
  if (name == "neh")
    return std::make_unique<Neh>();
  if (name == "bruteforce")
    return std::make_unique<BruteForce>();
  if (name == "identity")
    return std::make_unique<Identity>();
  return nullptr;
}

int main(int argc, char **argv) {
  std::string path;
  std::string algorithmName = "genetic";
  Params params;
  bool verbose = false;
  bool badUsage = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--verbose")
      verbose = true;
    else if (arg == "--algo" && i + 1 < argc)
      algorithmName = argv[++i];
    else if (arg == "--param" && i + 1 < argc) {
      if (!addParam(argv[++i], params))
        badUsage = true;
    } else if (arg[0] != '-' && path.empty())
      path = arg;
    else
      badUsage = true; // nieznana flaga, brak wartości po fladze albo drugi plik
  }

  std::unique_ptr<Algorithm> algorithm = makeAlgorithm(algorithmName);
  if (badUsage || path.empty() || !algorithm) {
    std::cerr << "Użycie: " << argv[0]
              << " <plik_instancji> [--algo genetic|sa|neh|bruteforce|identity] [--param nazwa=liczba]... [--verbose]\n";
    return 1;
  }

  try {
    Instance inst = Instance::load(path);
    std::cout << "Wczytano instancję: " << inst.n << " zadań, " << inst.m << " maszyn\n";

    Logger log = noLog;
    if (verbose)
      log = [&](const std::string &message) { std::cerr << "[" << algorithm->name() << "] " << message << "\n"; };

    Result result = algorithm->run(inst, params, log);
    std::cout << "Algorytm: " << algorithm->name() << "\nPermutacja:";
    for (int job : result.permutation)
      std::cout << ' ' << job;
    std::cout << "\nMakespan: " << result.makespan << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Błąd: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
