#pragma once

#include <vector>

#include "instance.h"

// Ocena rozwiązania: makespan, czyli czas zakończenia ostatniego zadania na ostatniej maszynie,
// gdy zadania wykonywane są w kolejności z permutacji. Model i wzór: docs/format.md.
// Przezbrojenie zaczyna się od razu po zakończeniu poprzedniego zadania na maszynie.
// Permutacja może zawierać tylko część zadań (np. przy budowaniu rozwiązania krok po kroku);
// dla pustej wynikiem jest 0. Indeksy muszą być z zakresu 0..n-1, bez powtórzeń.
// Czasy w instancji są typu int, ale ich suma może się w nim nie zmieścić, dlatego wynik to long long.
long long makespan(const Instance &inst, const std::vector<int> &permutation);
