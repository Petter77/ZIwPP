# Interfejs algorytmów: jak z niego korzystać

Opis klasy `Algorithm` (`alg/include/algorithm.h`) i sposobu użycia. Krótkie podsumowanie jest też w `README.md`,
a ogólny podział modułów w `architektura.md`.

## 1. Zasada
Algorytm bierze instancję i zwraca rozwiązanie. Nie czyta plików, nie zapisuje wyniku i nie wypisuje niczego na ekran:
wczytywaniem zajmuje się `Instance::load`, a wypisywaniem i zapisem kod, który algorytm wywołuje (dziś `main.cpp`).

```
plik ──Instance::load──> Instance ──Algorithm::run(instancja, parametry)──> Result
```

## 2. Elementy interfejsu

```cpp
using Params = std::map<std::string, double>;   // parametry algorytmu: nazwa -> wartość

struct Result {
  std::vector<int> permutation;                  // kolejność zadań (indeksy od 0), ta sama na każdej maszynie
};

class Algorithm {
public:
  virtual ~Algorithm() = default;
  virtual std::string name() const = 0;
  virtual Result run(const Instance &inst, const Params &params) const = 0;
};
```

| Element | Znaczenie |
|---|---|
| `name()` | krótka nazwa algorytmu (np. `"identity"`), do wypisywania i późniejszego wyboru z linii poleceń |
| `run(inst, params)` | uruchamia algorytm na instancji; `const`, więc algorytm nie ma stanu między uruchomieniami |
| `Params` | parametry (np. liczba iteracji); algorytm bierze z mapy to, czego potrzebuje, brakujące wartości ustawia sam |
| `Result::permutation` | permutacja `0..n-1`: każde zadanie dokładnie raz |

## 3. Jak algorytm jest używany

Tak robi to dziś `alg/src/main.cpp`:

```cpp
Instance inst = Instance::load(argv[1]);      // 1. wczytaj instancję z pliku

Identity algorithm;                            // 2. utwórz algorytm
Result result = algorithm.run(inst, {});       // 3. uruchom (puste parametry)

for (int job : result.permutation)             // 4. użyj wyniku
  std::cout << ' ' << job;
```

Ta sama instancja może być użyta przez kilka algorytmów, bo `run` jej nie zmienia.

## 4. Jak napisać nowy algorytm

1. Utwórz dwa pliki o nazwie algorytmu, np. dla `Greedy`:
   - `alg/include/algorithms/greedy.h`,
   - `alg/src/algorithms/greedy.cpp`.
2. W nagłówku wyprowadź klasę z `Algorithm` (wzorem `algorithms/identity.h`):

   ```cpp
   #pragma once
   #include "algorithm.h"

   class Greedy : public Algorithm {
   public:
     std::string name() const override { return "greedy"; }
     Result run(const Instance &inst, const Params &params) const override;
   };
   ```
3. W pliku `.cpp` zaimplementuj `run` i zwróć `Result` z pełną permutacją:

   ```cpp
   #include "algorithms/greedy.h"

   Result Greedy::run(const Instance &inst, const Params &params) const {
     Result result;
     // ... wyznacz result.permutation (n zadań, każde raz) ...
     return result;
   }
   ```
4. W `main.cpp` dodaj `#include "algorithms/greedy.h"` i użyj klasy tak jak `Identity` w punkcie 3.
5. Zbuduj `make` w katalogu `alg/` – Makefile sam dołącza nowe pliki z `src/`.

## 5. Zasady dla algorytmów
- Nie czytaj i nie zapisuj plików, nie wypisuj na ekran – tylko `Instance` na wejściu i `Result` na wyjściu.
- Nie losuj danych problemu. Losowość wewnątrz algorytmu (np. metaheurystyka) jest dozwolona, ale ziarno powinno być
  parametrem w `Params`, żeby wynik dało się powtórzyć.
- Zwracaj zawsze pełną, poprawną permutację (wszystkie `n` zadań, bez powtórzeń).
- Nie zmieniaj instancji (jest przekazywana jako `const`).

## 6. Algorytm wzorcowy: `Identity`
`algorithms/identity.h` i `src/algorithms/identity.cpp`. Zwraca zadania w kolejności z pliku (`0, 1, …, n-1`).
Służy jako wzór klasy pochodnej i punkt odniesienia dla prawdziwych algorytmów, a nie jako algorytm do oddania.

## 7. Czego jeszcze nie ma
- **Makespan w `Result`** – dojdzie z modułem oceny rozwiązania (Evaluator).
- **Opis parametrów, postęp, przerywanie, czas pracy** – potrzebne GUI w etapie 2; będzie można je dodać bez zmiany
  istniejących algorytmów.
- **Wybór algorytmu po nazwie z linii poleceń** – `main` tworzy na razie `Identity` na stałe; rejestr algorytmów dojdzie,
  gdy będzie ich więcej.
