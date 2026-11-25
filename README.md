# Dobór tematów i algorytmów
## Temat
W ramach projektu zdecydowano się na temat **2 - Permutacyjny problem przepływowy z przezbrojeniami zależnymi
od kolejności"

## Algorytmy
- Heurystyczny - NEH (opis: `docs/neh_bruteforce.md`) // Bartek
- Metaheurystyczny - algorytm genetyczny (opis: `docs/genetyczny.md`)
- Dodatkowy metaheurystyczny (5.0) - symulowane wyżarzanie (opis: `docs/wyzarzanie.md`)
- Algorytm dokładny (5.0) - przegląd zupełny (opis: `docs/neh_bruteforce.md`) // Bartek

## Struktura repozytorium
```
alg/include, alg/src   C++: instancja, ocena rozwiązania, algorytmy (jeden algorytm = klasa + plik w algorithms/)
alg/tools              generator instancji (osobny program)
app/                   GUI w Pythonie (etap 2, jeszcze nie ma)
docs/                  architektura, instancja, format pliku, interfejs algorytmów
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
- `Params` – parametry algorytmu jako mapa `nazwa -> liczba`,
- `Logger` – funkcja, do której algorytm zgłasza, co robi (`log("tekst")`); działa przy `--verbose`, w przeciwnym razie nic nie robi,
- `Result` – wynik: `permutation`, czyli kolejność zadań (indeksy od 0), ta sama na każdej maszynie, oraz jej `makespan`.

Przykładowa implementacja to `Identity` (`algorithms/identity.h`): zwraca zadania w kolejności z pliku. Służy jako wzór
i punkt odniesienia.

Makespan liczy wspólna dla wszystkich algorytmów funkcja `makespan(inst, permutation)` (`alg/include/evaluator.h`),
według wzoru z `docs/format.md`.

Nowy algorytm: skopiować `Identity`, zmienić nazwę i `run`, a w `main.cpp` dopisać go w `makeAlgorithm`.

Szczegóły użycia i instrukcja dodania algorytmu: `docs/algorytmy.md`.

## Budowanie
```
cd alg && make        # buduje main i generator
make clean
```

## Uruchamianie
```
./main <plik_instancji> [--algo genetic|sa|neh|bruteforce|identity] [--param nazwa=liczba]... [--verbose]

./main instances/instancja_100x20.txt                              # algorytm genetyczny, wynik na stdout
./main instances/instancja_100x20.txt --verbose                    # dodatkowo logi algorytmu na stderr
./main instances/instancja_100x20.txt --param generations=300 --param seed=7
./main instances/instancja_100x20.txt --algo sa                    # symulowane wyżarzanie
./main instances/instancja_100x20.txt --algo neh                   # heurystyka NEH
./main instances/mala.txt --algo bruteforce                        # dokładny, domyślnie do 11 zadań (--param max_n=...)
./main instances/instancja_100x20.txt --algo identity              # kolejność z pliku
```
Wczytuje instancję, uruchamia wybrany algorytm (domyślnie `genetic`) i wypisuje permutację (zadania numerowane od 0)
oraz jej makespan. Parametry algorytmu genetycznego opisuje `docs/genetyczny.md`, wyżarzania – `docs/wyzarzanie.md`, a `max_n` dla `bruteforce` –
`docs/neh_bruteforce.md`; `neh` i `identity` nie mają parametrów i je ignorują. Z `--verbose` na stderr pojawiają się komunikaty algorytmu w postaci `[nazwa] tekst`; stdout jest
taki sam jak bez flagi, więc wynik można przekierować bez logów.

Testy NEH i przeglądu zupełnego (`alg/tests/`): polecenie w `docs/neh_bruteforce.md`.

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
