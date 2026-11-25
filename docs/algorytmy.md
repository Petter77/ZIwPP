# Interfejs algorytmów: jak z niego korzystać

Opis klasy `Algorithm` (`alg/include/algorithm.h`) i sposobu użycia. Krótkie podsumowanie jest też w `README.md`,
a ogólny podział modułów w `architektura.md`.

## 1. Zasada
Algorytm bierze instancję i zwraca rozwiązanie. Nie czyta plików, nie zapisuje wyniku i nie wypisuje niczego na ekran:
wczytywaniem zajmuje się `Instance::load`, a wypisywaniem i zapisem kod, który algorytm wywołuje (dziś `main.cpp`).
To, co dzieje się wewnątrz algorytmu, zgłasza on przez funkcję `log` (punkt 6).

```
plik ──Instance::load──> Instance ──Algorithm::run(instancja, parametry, log)──> Result
```

## 2. Elementy interfejsu

```cpp
using Params = std::map<std::string, double>;   // parametry algorytmu: nazwa -> wartość

using Logger = std::function<void(const std::string &)>;   // logowanie (--verbose)
inline const Logger noLog = [](const std::string &) {};    // logger, który nic nie robi

struct Result {
  std::vector<int> permutation;                  // kolejność zadań (indeksy od 0), ta sama na każdej maszynie
  int makespan = 0;                              // makespan tej permutacji
};

class Algorithm {
public:
  virtual ~Algorithm() = default;
  virtual std::string name() const = 0;
  virtual Result run(const Instance &inst, const Params &params, const Logger &log) const = 0;
};
```

| Element | Znaczenie |
|---|---|
| `name()` | krótka nazwa algorytmu (np. `"identity"`), do wypisywania i późniejszego wyboru z linii poleceń |
| `run(inst, params, log)` | uruchamia algorytm na instancji; `const`, więc algorytm nie ma stanu między uruchomieniami |
| `Params` | parametry (np. liczba iteracji); algorytm bierze z mapy to, czego potrzebuje, brakujące wartości ustawia sam |
| `Logger` / `log` | funkcja, którą algorytm wywołuje, żeby zgłosić, co robi; wywołujący zawsze przekazuje poprawną (`noLog`, gdy logowanie wyłączone) |
| `Result::permutation` | permutacja `0..n-1`: każde zadanie dokładnie raz |
| `Result::makespan` | wartość funkcji celu dla tej permutacji; algorytm liczy ją funkcją `makespan` |

### Ocena rozwiązania (Evaluator)
`alg/include/evaluator.h` udostępnia jedną funkcję, wspólną dla wszystkich algorytmów:

```cpp
int makespan(const Instance &inst, const std::vector<int> &permutation);
```

Liczy makespan według wzoru z `format.md`. Nie zmienia instancji i nie sprawdza permutacji, więc indeksy muszą być
z zakresu `0..n-1`, bez powtórzeń. Permutacja może być częściowa (np. gdy algorytm buduje kolejność zadanie po zadaniu),
dla pustej wynikiem jest `0`. Jedno wywołanie to `O(m·liczba zadań)`. Algorytmy nie liczą makespanu po swojemu,
dzięki czemu ich wyniki są porównywalne.

## 3. Jak algorytm jest używany

W uproszczeniu tak robi to `alg/src/main.cpp` (algorytm wybiera tam funkcja `makeAlgorithm` na podstawie `--algo`):

```cpp
Instance inst = Instance::load(argv[1]);      // 1. wczytaj instancję z pliku

Identity algorithm;                            // 2. utwórz algorytm
Result result = algorithm.run(inst, {}, noLog); // 3. uruchom (puste parametry, bez logowania)

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
     Result run(const Instance &inst, const Params &params, const Logger &log) const override;
   };
   ```
3. W pliku `.cpp` zaimplementuj `run` i zwróć `Result` z pełną permutacją:

   ```cpp
   #include "algorithms/greedy.h"

   // na górze pliku także: #include "evaluator.h"
   Result Greedy::run(const Instance &inst, const Params &params, const Logger &log) const {
     log("start");
     Result result;
     // ... wyznacz result.permutation (n zadań, każde raz) ...
     result.makespan = makespan(inst, result.permutation);
     return result;
   }
   ```
4. W `main.cpp` dodaj `#include "algorithms/greedy.h"`, dopisz nazwę w `makeAlgorithm` i w komunikacie użycia.
5. Zbuduj `make` w katalogu `alg/` – Makefile sam dołącza nowe pliki z `src/`.

## 5. Zasady dla algorytmów
- Nie czytaj i nie zapisuj plików, nie wypisuj na ekran – tylko `Instance` na wejściu i `Result` na wyjściu.
- Nie losuj danych problemu. Losowość wewnątrz algorytmu (np. metaheurystyka) jest dozwolona, ale ziarno powinno być
  parametrem w `Params`, żeby wynik dało się powtórzyć.
- Zwracaj zawsze pełną, poprawną permutację (wszystkie `n` zadań, bez powtórzeń).
- Nie zmieniaj instancji (jest przekazywana jako `const`).

## 6. Logowanie (`--verbose`)
Program uruchomiony z `--verbose` pokazuje, co dzieje się wewnątrz algorytmu:

```
./main instances/plik.txt --algo identity --verbose
[identity] zwracam kolejność z pliku (8 zadań)
```

- Algorytm woła `log("tekst")`. Nie wie, dokąd tekst trafia: `main` wypisuje go na **stderr** z prefiksem `[nazwa_algorytmu]`,
  więc wynik na stdout można przekierować bez logów. Bez flagi `log` nic nie robi.
- `run` zawsze dostaje poprawną funkcję `log` (nigdy pustą), więc algorytm nie musi sprawdzać, czy jest ustawiona.
- Loguj **zdarzenia** (poprawa wyniku, początek lub koniec fazy, zmiana parametru), a nie każdą iterację wewnętrznej pętli:
  logi nie powinny spowalniać algorytmu ani zalewać ekranu.
- W etapie 2 ten sam mechanizm zasili „bieżące logi” w GUI: zamiast wypisywać na ekran, funkcja `log` wyśle tekst przez socket.

## 7. Algorytm wzorcowy: `Identity`
`algorithms/identity.h` i `src/algorithms/identity.cpp`. Zwraca zadania w kolejności z pliku (`0, 1, …, n-1`)
i loguje jeden komunikat jako przykład użycia `log`.
Służy jako wzór klasy pochodnej i punkt odniesienia dla prawdziwych algorytmów, a nie jako algorytm do oddania.
Przykład algorytmu z parametrami, ziarnem i sprawdzaniem wartości: `Genetic` (`genetyczny.md`).
Pozostałe algorytmy: `sa` (`wyzarzanie.md`) oraz `neh` i `bruteforce` (`neh_bruteforce.md`).
Wspólne funkcje metaheurystyk (losowanie, przeniesienie zadania, sprawdzanie parametru całkowitego) są w
`alg/include/algorithms/common.h`.

## 8. Czego jeszcze nie ma
- **Pełny harmonogram (czasy rozpoczęcia przezbrojeń i operacji)** – będzie potrzebny do diagramu Gantta w etapie 2.
- **Opis parametrów, strukturalny postęp (zamiast samego tekstu), przerywanie, czas pracy** – potrzebne GUI w etapie 2; będzie można je dodać bez zmiany
  istniejących algorytmów.
- **Rejestr algorytmów** – wybór po nazwie (`--algo`) to na razie prosta funkcja `makeAlgorithm` w `main.cpp`.
