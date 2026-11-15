# Dobór tematów i algorytmów
## Temat
W ramach projektu zdecydowano się na temat **2 - Permutacyjny problem przepływowy z przezbrojeniami zależnymi
od kolejności"

## Algorytmy
- Heurystyczny - (?)
- Metaheurystyczny - (?)
- Dodatkowy metaheurystyczny (5.0) - (?)
- Algorytm dokładny (5.0) - (?)

Wersje robocze w kodzie (do zatwierdzenia albo wymiany): NEH, symulowane wyżarzanie, przegląd zupełny (n ≤ 11).

## Struktura repozytorium
```
alg/include, alg/src   rdzeń w C++: model, ocena, algorytmy (jeden algorytm = klasa + plik w algorithms/), rejestr
app/                   GUI w Pythonie (etap 2)
data/                  przykładowe instancje
docs/                  format instancji, protokół GUI <-> algorytmy
files/                 materiały z zajęć
```

Nowy algorytm: klasa dziedzicząca po `Algorithm` (`alg/include/algorithm.h`) w `algorithms/` i wpis w `alg/src/registry.cpp`.
Algorytmy nie znają konsoli ani gniazd – postęp i anulowanie przechodzą przez `RunContext`.

## Uruchamianie
```
cd alg && make
./main --list
./main ../data/small_8x3.txt --algo sa --param max_iter=100000 --verbose
```

# Dane
- Format instancji i model: `docs/format.md`
- Algorytm generowania danych musi być osobnym modułem, nie integralną cześcią kodu (do zrobienia)
- Możliwość wgrania zewnętrznego zestawu danych testowych do weryfikacji (sprawdzic jakie mamy opcje)

# Komunikacja (etap 2)
Sockety TCP (protokół do zaprojektowania).
