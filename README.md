# Dobór tematów i algorytmów
## Temat
W ramach projektu zdecydowano się na temat **2 - Permutacyjny problem przepływowy z przezbrojeniami zależnymi
od kolejności"

## Algorytmy
- Heurystyczny - (?) // Bartek
- Metaheurystyczny - (?)
- Dodatkowy metaheurystyczny (5.0) - (?)
- Algorytm dokładny (5.0) - (?) // Bartek

Wersje robocze w kodzie (do zatwierdzenia albo wymiany): NEH, symulowane wyżarzanie, przegląd zupełny (n ≤ 11).

## Struktura repozytorium
```
alg/include, alg/src   rdzeń w C++: model, ocena, algorytmy (jeden algorytm = klasa + plik w algorithms/), rejestr
app/                   GUI w Pythonie (etap 2)
data/                  przykładowe instancje
docs/                  format instancji, protokół GUI <-> algorytmy
files/                 materiały z zajęć
```

## Interfejs algorytmów
Każdy algorytm to osobna klasa dziedzicząca po `Algorithm` (`alg/include/algorithm.h`), w osobnych plikach:
`alg/include/algorithms/<nazwa>.h` i `alg/src/algorithms/<nazwa>.cpp`. Makefile sam dołącza nowe pliki z `src/`.

```cpp
class Algorithm {
  virtual std::string name() const = 0;
  virtual Result run(const Instance &inst, const Params &params, const Logger &log) const = 0;
};
```
- `Instance` – wczytana instancja (algorytm nigdy nie czyta ani nie zapisuje plików),
- `Params` – parametry algorytmu jako mapa `nazwa -> liczba` (na razie nieużywane),
- `Logger` – funkcja, do której algorytm zgłasza, co robi (`log("tekst")`); działa przy `--verbose`, w przeciwnym razie nic nie robi,
- `Result` – wynik: `permutation`, czyli kolejność zadań (indeksy od 0), ta sama na każdej maszynie.

Przykładowa implementacja to `Identity` (`algorithms/identity.h`): zwraca zadania w kolejności z pliku. Służy jako wzór
i punkt odniesienia. Wynik nie zawiera jeszcze makespanu, bo nie ma jeszcze modułu liczącego go (kolejny krok).

Nowy algorytm: skopiować `Identity`, zmienić nazwę i `run`, a w `main.cpp` użyć nowej klasy.

Szczegóły użycia i instrukcja dodania algorytmu: `docs/algorytmy.md`.

## Budowanie
```
cd alg && make        # buduje main i generator
make clean
```

## Uruchamianie
```
./main instances/instancja_100x20.txt              # wynik na stdout
./main instances/instancja_100x20.txt --verbose    # dodatkowo logi algorytmu na stderr
```
Wczytuje instancję, uruchamia algorytm `Identity` i wypisuje permutację. Z `--verbose` na stderr pojawiają się komunikaty
algorytmu w postaci `[nazwa] tekst`; stdout jest taki sam jak bez flagi, więc wynik można przekierować bez logów. Kolejne kroki (ocena rozwiązania, kolejne algorytmy) dochodzą do tego samego przepływu.

## Generator instancji
Osobny program (`alg/tools/generator.cpp`), niezależny od algorytmów. Zapisuje instancję w formacie z `docs/format.md`
przy użyciu `Instance::save`.

```
./generator <n> <m> <ziarno> <nazwa_pliku> [p_max] [s_max]
```

| Argument | Znaczenie | Domyślnie |
|---|---|---|
| `n` | liczba zadań | – |
| `m` | liczba maszyn | – |
| `ziarno` | ziarno generatora liczb losowych (obowiązkowe) | – |
| `nazwa_pliku` | nazwa pliku wyjściowego; zapisywany w `instances/` (katalog tworzony automatycznie) | – |
| `p_max` | czasy operacji `p` losowane z 1..`p_max` | 99 |
| `s_max` | przezbrojenia `s` losowane z 1..`s_max` (także przekątna) | 50 |

Przykład: `./generator 100 20 42 instancja_100x20.txt` zapisuje `instances/instancja_100x20.txt`
(katalog `instances/` jest względem miejsca uruchomienia, np. `alg/instances/`, i jest w `.gitignore`).

To samo ziarno i te same argumenty dają zawsze ten sam plik, także na innym kompilatorze.
Algorytmy nie losują danych problemu: zawsze dostają instancję z pliku.

# Dane
- Format instancji i model: `docs/format.md`
- Algorytm generowania danych musi być osobnym modułem, nie integralną cześcią kodu (do zrobienia)
- Możliwość wgrania zewnętrznego zestawu danych testowych do weryfikacji (sprawdzic jakie mamy opcje)

# Komunikacja (etap 2)
Sockety TCP (protokół do zaprojektowania).
