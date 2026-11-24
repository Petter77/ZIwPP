#include <iostream>

#include "instance.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Użycie: " << argv[0] << " <plik_instancji>\n";
    return 1;
  }

  try {
    Instance inst = Instance::load(argv[1]);
    std::cout << "Wczytano instancję: " << inst.n << " zadań, " << inst.m << " maszyn\n";
  } catch (const std::exception &e) {
    std::cerr << "Błąd: " << e.what() << "\n";
    return 1;
  }
  return 0;
}
