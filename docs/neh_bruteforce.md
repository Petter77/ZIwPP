# Algorytmy NEH i BruteForce — opis implementacji

Dokumentacja części zaimplementowanej dla **Tematu 2** (permutacyjny problem przepływowy z przezbrojeniami zależnymi od kolejności, PFSP-SDST, kryterium: makespan) zgodnie z interfejsem z `docs/algorytmy.md` i modelem z `docs/format.md`.

## 1. Pliki
| Plik | Rola |
|---|---|
| `alg/include/algorithms/neh.h`, `alg/src/algorithms/neh.cpp` | Algorytm heurystyczny `Neh` (`name() == "neh"`). |
| `alg/include/algorithms/bruteforce.h`, `alg/src/algorithms/bruteforce.cpp` | Algorytm dokładny `BruteForce` (`name() == "bruteforce"`). |
| `alg/tests/test_algorithms.cpp` | Samodzielny program testowy (poza `alg/src/`, więc nie koliduje z `Makefile`). |
| `benchmark_wyniki.txt` | Zestawienie czasów działania i jakości wyników z testów w WSL2 (Ubuntu, `g++ -O2`). |

## 2. Zgodność z interfejsem `Algorithm` i modelem

Oba algorytmy dziedziczą po `Algorithm` (`alg/include/algorithm.h`):
- są bezstanowe (`run` jest `const`), nie modyfikują `Instance`,
- nie czytają ani nie zapisują plików i niczego nie wypisują; zdarzenia zgłaszają przez `log` (`--verbose`),
- zwracają kompletną, poprawną permutację `0..n-1` i jej makespan w `Result`.

Makespan liczy wspólna funkcja `makespan` z `alg/include/evaluator.h` (wzór z `format.md`); obsługuje też częściowe
permutacje, z czego korzysta NEH. `BruteForce` liczy ten sam wzór przyrostowo w trakcie przeglądu (na `long long`).

Logi: NEH zgłasza posortowanie zadań i każdy krok wstawiania, BruteForce – początek, każdy nowy najlepszy wynik i koniec.

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

## 4. Budowanie i uruchamianie (Linux / WSL2, g++17)

```bash
cd alg
make

./generator 8 4 42 test_8x4.txt
./main instances/test_8x4.txt --algo neh
./main instances/test_8x4.txt --algo bruteforce --verbose
./main instances/test_8x4.txt --algo bruteforce --param max_n=12
```

### Testy jednostkowe i weryfikacyjne
```bash
cd alg
mkdir -p build
g++ -std=c++17 -Wall -Wextra -O2 -Iinclude \
  tests/test_algorithms.cpp src/instance.cpp src/evaluator.cpp \
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
7. Czasy bliskie zakresowi `int`: makespan większy niż zakres `int` bez przepełnienia.
8. Wczytywanie: dane po ostatniej wartości (liczba, tekst, ułamek) są odrzucane, białe znaki dozwolone.

## 5. Wybrane wyniki z benchmarku (`benchmark_wyniki.txt`)

Średnie z **10 niezależnych instancji** na każdy rozmiar (ziarna `1..10`, `g++ -std=c++17 -O2`, WSL2 Ubuntu):
- **`8x4`:** śr. Identity `899.2`, śr. NEH `731.8` (`0.005 ms`, śr. błąd `+3.43%`), śr. BruteForce `707.9` (`2.66 ms`).
- **`10x5`:** śr. Identity `1202.8`, śr. NEH `974.7` (`0.010 ms`, śr. błąd `+4.05%`), śr. BruteForce `936.5` (`281.74 ms`).
- **`11x5`:** śr. Identity `1209.9`, śr. NEH `1060.7` (`0.009 ms`, śr. błąd `+5.16%`), śr. BruteForce `1009.0` (`3.10 s`).
- **`11x10`:** śr. Identity `1702.3`, śr. NEH `1434.0` (`0.016 ms`, śr. błąd `+4.38%`), śr. BruteForce `1374.0` (`5.26 s`).
- **`100x20`:** śr. Identity `10651.8`, śr. NEH `9021.0` (`-15.30%`, `29.12 ms`).
- **`500x20`:** śr. Identity `43876.0`, śr. NEH `38150.6` (`-13.04%`, `13.10 s`).
