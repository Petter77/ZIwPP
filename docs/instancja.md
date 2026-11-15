# Instancja problemu: architektura

Status: **szkic do uzgodnienia z zespołem**. Szczegóły formatu pliku są w `format.md`, ogólna architektura w `architektura.md`.

Instancja to **tylko dane problemu**. Nie zna algorytmów, nie liczy rozwiązań i nie wie, jak jest zapisana.
Wszystko, co się z nią dzieje, robią osobne części, które się na niej opierają.

## 1. Elementy i podział odpowiedzialności

```
Generator ──zapis──> plik (format.md) ──Instance::load/parse──> Instance
                                                                   │
                         Evaluator(Instance, Permutation) ─────────┤
                         Algorithm::run(Instance, params) ─────────┘
                                         │
                                       Result ──> Evaluator (walidacja/harmonogram)
```

| Część | Co robi | Czego nie robi |
|---|---|---|
| **Instance** | przechowuje `n`, `m`, `p`, `s`; pilnuje niezmienników; wczytuje się z pliku i tekstu; zapisuje się do pliku | nie rozwiązuje, nie liczy makespanu |
| **Format pliku** | opisuje układ danych (jedyne źródło prawdy: `format.md`) | – |
| **Generator** | tworzy `Instance` z parametrów (n, m, ziarno, zakresy) i zapisuje ją przez `Instance::save` | nie jest częścią algorytmów ani aplikacji |
| **Evaluator** | z `Instance` i permutacji liczy makespan i pełny harmonogram (czasy startu przezbrojeń i operacji) | nie szuka rozwiązań |
| **Algorithm** | z `Instance` i parametrów zwraca `Result` | nie czyta ani nie zapisuje plików |

Generator i parser używają tego samego formatu, więc plik zapisany przez generator zawsze da się wczytać.

## 2. Model danych `Instance`

| Pole | Znaczenie | Wymiar |
|---|---|---|
| `n` | liczba zadań | ≥ 1 |
| `m` | liczba maszyn | ≥ 1 |
| `p[i][j]` | czas operacji zadania `j` na maszynie `i` | `m × n`, wartości ≥ 1 |
| `s[i][j][k]` | przezbrojenie na maszynie `i` z zadania `j` na `k` | `m × n × n`, wartości ≥ 1 |

Niezmienniki, których pilnuje klasa, tak że nie ma „częściowo poprawnej” instancji:
- wymiary macierzy zgodne z `n` i `m`,
- wszystkie `p` i `s` dodatnie (także przekątna `s[i][k][k]`, czyli przezbrojenie początkowe, jak w dokumencie prowadzącego),

Po utworzeniu instancja jest **niezmienna** (tylko do odczytu). Algorytmy mogą ją dzielić bez kopiowania.

## 3. Wejście i wyjście
- **`Instance::load(ścieżka)`** otwiera plik i wywołuje `parse`. Statyczna fabryka: zwraca poprawną instancję albo zgłasza błąd,
  nigdy obiekt w połowie wczytany.
- **`Instance::parse(strumień)`** czyta dane ze strumienia lub tekstu. Dzięki temu w etapie 2 serwer może przyjąć
  instancję z socketu bez zapisywania na dysk.
- **`Instance::save(ścieżka)`** zapisuje instancję w tym samym formacie. Używa tego generator.
- **Błędy** to czytelne komunikaty po polsku: co jest nie tak i gdzie (np. „oczekiwano liczby w macierzy p, wiersz 3”).
  Dotyczy to brakujących, nadmiarowych, niedodatnich lub nieliczbowych danych.

## 4. Powiązany obiekt: rozwiązanie
Instancja nie przechowuje rozwiązania. Rozwiązanie to osobne pojęcie:
- **`Permutation`**: kolejność zadań (indeksy od 0), wspólna dla wszystkich maszyn. Poprawna, gdy każde zadanie
  występuje dokładnie raz.
- **Evaluator** (osobny moduł) liczy na jej podstawie:
  - makespan: `C[i][k] = max( C[i][k-1] + s[i][π(k-1)][π(k)] , C[i-1][k] ) + p[i][π(k)]`, makespan to `C[m-1][n-1]`
    (pełny opis w `format.md`),
  - harmonogram: czasy startu przezbrojenia i operacji, `[maszyna][pozycja]` (dane dla diagramu Gantta),
  - walidację permutacji.
- **Result** (zwracany przez algorytm) niesie permutację, makespan i czas pracy. Makespan algorytmu zawsze można
  niezależnie sprawdzić Evaluatorem.

## 5. Format pliku (podsumowanie)
```
n m
p: m linii po n liczb (wiersz = maszyna)
s: dla każdej maszyny n linii po n liczb (wiersz = zadanie j, kolumna = zadanie k)
```
Pełny opis i przykład: `format.md`.

## 6. Założenia modelu
- Zadanie przechodzi przez maszyny 1 → m, kolejność zadań jest taka sama na każdej maszynie.
- Przezbrojenie zaczyna się zaraz po zakończeniu poprzedniego zadania na maszynie (przyjęty wariant „anticipatory”).
- Kryterium: makespan.

## 7. Decyzje jeszcze otwarte
- Język i dokładne parametry **generatora** (zakresy `p` i `s`, ziarno).
- Gdzie przechowywać wartości odniesienia (znane optimum, najlepszy znany wynik), skoro nie ma ich w pliku instancji.
- Czy `Instance` zapisujemy też w innej postaci niż tekst (np. JSON dla etapu 2). Rekomendacja: na razie jeden format
  tekstowy i `parse` ze strumienia, bo tego wymaga socket.
