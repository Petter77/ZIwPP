# Algorytmy NEH i BruteForce — opis implementacji

Dokumentacja części zaimplementowanej dla **Tematu 2** (permutacyjny problem przepływowy z przezbrojeniami zależnymi od kolejności, PFSP-SDST, kryterium: makespan) zgodnie z interfejsem z `docs/algorytmy.md` i modelem z `docs/format.md`.

## 1. Struktura dodanych i zmienionych plików

### Nowe pliki
| Plik | Rola |
|---|---|
| `alg/include/algorithms/sdst_eval.h` | Tymczasowa funkcja pomocnicza `evaluateMakespan(inst, perm)` licząca makespan wg `docs/format.md` (do czasu pojawienia się modułu `Evaluator`). |
| `alg/include/algorithms/neh.h`, `alg/src/algorithms/neh.cpp` | Algorytm heurystyczny `Neh` (`name() == "neh"`). |
| `alg/include/algorithms/bruteforce.h`, `alg/src/algorithms/bruteforce.cpp` | Algorytm dokładny `BruteForce` (`name() == "bruteforce"`). |
| `alg/tests/test_algorithms.cpp` | Samodzielny program testowy (umieszczony poza `alg/src/`, więc nie koliduje z `Makefile`). |
| `benchmark_wyniki.txt` | Zestawienie czasów działania i jakości wyników z testów w WSL2 (Ubuntu, `g++ -O2`). |

### Zmiany w istniejących plikach
- **`alg/src/main.cpp`** — jedyny zmodyfikowany plik z dotychczasowego kodu. Dodano opcjonalny drugi argument linii poleceń:
  ```bash
  ./main <plik_instancji> [identity|neh|bruteforce]
  ```
  Bez drugiego argumentu nadal uruchamia domyślny `identity` (100% zgodności wstecznej).
- **Pozostałe pliki bez zmian:** `Makefile`, `algorithm.h`, `instance.h`, `instance.cpp`, `identity.h`, `identity.cpp`, `generator.cpp`. `Makefile` automatycznie wykrywa i kompiluje nowe pliki `.cpp` z `alg/src/algorithms/`.

## 2. Zgodność z interfejsem `Algorithm` i modelem

Oba algorytmy dziedziczą po `Algorithm` (`alg/include/algorithm.h`):
- są bezstanowe (`run` jest `const`), nie modyfikują `Instance`,
- nie czytają ani nie zapisują plików, nie wypisują nic na `stdout`/`stderr`,
- zawsze zwracają kompletną, poprawną permutację `0..n-1` w `Result::permutation`.

### Model oceny (`sdst_eval.h`)
Zaimplementowano dokładnie wzór z `docs/format.md` (wariant przezbrojeń *anticipatory* oraz dodatnie przezbrojenie początkowe `s[i][k][k]`):
```text
C[i][k] = max( C[i][k-1] + s[i][π(k-1)][π(k)], C[i-1][k] ) + p[i][π(k)]
```
Obliczenia wewnętrzne używają typu `long long`, aby uniknąć przepełnienia przy dużych wartościach `p` i `s`. Funkcja `evaluateMakespan` obsługuje zarówno pełne, jak i częściowe permutacje (wykorzystywane w NEH).

> **Uwaga integracyjna (dla przyszłego modułu `Evaluator`):**
> Nagłówek `sdst_eval.h` celowo umieszczono w `include/algorithms/` pod inną nazwą, żeby nie kolidował z planowanym `include/evaluator.h`. Gdy dojdzie docelowy `Evaluator` i pole `Result::makespan`, wystarczy:
> 1. w `neh.cpp` podmienić wywołanie `evaluateMakespan` na `Evaluator` i zapisać `result.makespan`,
> 2. w `bruteforce.cpp` dopisać `result.makespan = bestMakespan;`,
> 3. usunąć `sdst_eval.h` (po aktualizacji `tests/test_algorithms.cpp`).

## 3. Opis algorytmów

### `Neh` (heurystyczny)
- **Nazwa:** `"neh"` (`alg/include/algorithms/neh.h`)
- **Parametry (`Params`):** brak (nieużywane).
- **Działanie:**
  1. Dla każdego zadania `j` liczy łączny czas przetwarzania na wszystkich maszynach: `sum_i p[i][j]`.
  2. Sortuje zadania malejąco po tej sumie (remisy rozstrzygane deterministycznie po mniejszym indeksie zadania).
  3. Buduje permutację przyrostowo: startuje od pierwszego zadania, a każde kolejne wstawia na tę spośród wszystkich możliwych pozycji `0..len`, która minimalizuje częściowy makespan (z uwzględnieniem przezbrojeń SDST).
- **Złożoność:** czasowa `O(m * n^3)`, pamięciowa `O(n + m)`.
- **Przypadki brzegowe:** `n <= 0` → pusta permutacja; `n == 1` → `{0}`.

### `BruteForce` (dokładny — przegląd zupełny)
- **Nazwa:** `"bruteforce"` (`alg/include/algorithms/bruteforce.h`)
- **Parametry (`Params`):**
  | Klucz | Domyślnie | Znaczenie |
  |---|---|---|
  | `"max_n"` | `11` | Maksymalna dopuszczalna liczba zadań (zabezpieczenie przed eksplozją `n!`). |
- **Obsługa błędów:** gdy `inst.n > max_n`, rzuca `std::runtime_error` z czytelnym komunikatem po polsku.
- **Działanie:**
  - Rekurencyjnie (DFS z backtrackingiem) przegląda **wszystkie `n!` permutacji** w kolejności leksykograficznej — gwarantuje znalezienie optimum globalnego.
  - Zamiast przeliczać całą permutację od zera dla każdego liścia, aktualizuje tablicę `completion[depth][i]` przyrostowo wyłącznie dla bieżącej głębokości `depth` (czas `O(m)` na węzeł drzewa).
- **Złożoność:** czasowa `O(n! * m)`, pamięciowa `O(n * m)`.

## 4. Budowanie i uruchamianie (WSL2 / Linux, g++17)

```bash
cd alg
make

# Wygenerowanie przykładowej instancji i uruchomienie z CLI:
./generator 8 4 42 test_8x4.txt
./main instances/test_8x4.txt              # domyślnie identity
./main instances/test_8x4.txt neh
./main instances/test_8x4.txt bruteforce
```

### Testy jednostkowe i weryfikacyjne
```bash
cd alg
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude \
  tests/test_algorithms.cpp src/instance.cpp \
  src/algorithms/identity.cpp src/algorithms/neh.cpp src/algorithms/bruteforce.cpp \
  -o build/test_algorithms
./build/test_algorithms
```
Zakres testów w `alg/tests/test_algorithms.cpp`:
1. Przykład `3x2` z `docs/format.md` (weryfikacja wszystkich 6 permutacji względem ręcznych obliczeń; optimum = `71`).
2. Asymetria przezbrojeń (`2x2`, gdzie `[1, 0]` jest istotnie lepsze od `[0, 1]`).
3. Przypadki brzegowe (`1x1`, `1x4`, `4x1`).
4. Krzyżowa weryfikacja przyrostowego `BruteForce` z niezależnym `std::next_permutation` (`n = 5..7`) oraz porównanie `Identity` vs `NEH` vs `BruteForce` (`n = 5..8`).
5. Weryfikacja rzucania wyjątku dla domyślnego (`n = 12`) i własnego (`max_n = 4`) limitu w `BruteForce`.
6. Skalowalność i poprawność permutacji `NEH` dla `50x10` i `100x20`.

## 5. Wybrane wyniki z benchmarku (`benchmark_wyniki.txt`)

Średnie z **10 niezależnych instancji** na każdy rozmiar (ziarna `1..10`, `g++ -std=c++17 -O2`, WSL2 Ubuntu):
- **`8x4`:** śr. Identity `899.2`, śr. NEH `731.8` (`0.005 ms`, śr. błąd `+3.43%`), śr. BruteForce `707.9` (`2.66 ms`).
- **`10x5`:** śr. Identity `1202.8`, śr. NEH `974.7` (`0.010 ms`, śr. błąd `+4.05%`), śr. BruteForce `936.5` (`281.74 ms`).
- **`11x5`:** śr. Identity `1209.9`, śr. NEH `1060.7` (`0.009 ms`, śr. błąd `+5.16%`), śr. BruteForce `1009.0` (`3.10 s`).
- **`11x10`:** śr. Identity `1702.3`, śr. NEH `1434.0` (`0.016 ms`, śr. błąd `+4.38%`), śr. BruteForce `1374.0` (`5.26 s`).
- **`100x20`:** śr. Identity `10651.8`, śr. NEH `9021.0` (`-15.30%`, `29.12 ms`).
- **`500x20`:** śr. Identity `43876.0`, śr. NEH `38150.6` (`-13.04%`, `13.10 s`).

## 6. Punkty pod dalszą integrację z resztą zespołu
- **Registry / CLI:** po dodaniu rejestru algorytmów wystarczy zarejestrować `Neh` (`"neh"`) i `BruteForce` (`"bruteforce"`) oraz zastąpić tymczasowy `if/else` w `alg/src/main.cpp`.
- **Evaluator / `Result::makespan`:** patrz uwaga integracyjna w sekcji 2.
- **Etap 2 (GUI / parametry):** `BruteForce` przyjmuje opcjonalny parametr `"max_n"` (domyślnie `11`); `Neh` nie wymaga parametrów.
