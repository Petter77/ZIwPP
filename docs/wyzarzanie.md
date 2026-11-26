# Symulowane wyżarzanie (`sa`)

Druga metaheurystyka dla tematu 2. Kod: `alg/include/algorithms/annealing.h`, `alg/src/algorithms/annealing.cpp`.

## 1. Idea
Algorytm trzyma **jedno** rozwiązanie (kolejność zadań) i w kółko próbuje je trochę zmienić. Lepszy sąsiad jest
przyjmowany zawsze, gorszy **czasem**, co pozwala wyjść z lokalnego optimum. Szansę na przyjęcie gorszego opisuje
temperatura `T`: na początku jest wysoka (algorytm dużo błądzi), potem maleje (algorytm przyjmuje prawie tylko poprawy).

## 2. Przebieg
```
bieżące = losowa kolejność, najlepsze = bieżące
dobierz temperaturę początkową i końcową oraz współczynnik chłodzenia
powtórz `iterations` razy:
    sąsiad = bieżące po jednym losowym ruchu (przeniesienie albo zamiana)
    delta = makespan(sąsiad) - makespan(bieżące)
    jeśli delta <= 0:                       bieżące = sąsiad
    w przeciwnym razie z szansą exp(-delta / T):  bieżące = sąsiad
    jeśli bieżące lepsze od najlepsze:      najlepsze = bieżące
    T = T * chłodzenie
zwróć najlepsze
```
Zwracane jest **najlepsze znalezione** rozwiązanie, które nie musi być ostatnim bieżącym.

## 3. Elementy

| Funkcja | Działanie |
|---|---|
| `neighbour` | kopia kolejności z jednym ruchem: z szansą `insert`% przeniesienie zadania na inne miejsce, w przeciwnym razie zamiana dwóch zadań |
| `typicalWorsening` | 1000 próbnych ruchów z rozwiązania początkowego; średnia z tych, które pogarszają makespan (gdy żaden nie pogarsza: 1) |
| `temperatureFor` | temperatura, przy której pogorszenie o `delta` przechodzi z daną szansą |
| `accept` | decyzja o przyjęciu sąsiada |

### Ruchy
```
kolejność:               0 1 2 3 4 5
przeniesienie (1 -> 4):  0 2 3 4 1 5
zamiana (1, 4):          0 4 2 3 1 5
```

### Przyjmowanie gorszych rozwiązań
Szansa wynosi `exp(-delta / T)`: maleje, gdy pogorszenie `delta` jest większe albo temperatura niższa. Przykład:

| pogorszenie | T = 15,2 | T = 2 |
|---|---|---|
| 1 | 94% | 61% |
| 10 | 52% | 0,7% |
| 35 | 10% | prawie 0 |
| 100 | 0,14% | prawie 0 |

### Automatyczny dobór temperatury
Skala makespanu i różnic zależy od instancji, więc temperatury nie są wpisane na stałe. Podaje się **prawdopodobieństwa**,
a temperatury wynikają z przekształcenia wzoru `exp(-delta / T) = szansa`, czyli `T = -delta / ln(szansa)`:

- **początkowa:** typowe pogorszenie ma przechodzić z szansą `start_prob` (domyślnie 10%),
- **końcowa:** pogorszenie o 1 ma przechodzić z szansą `end_prob` (domyślnie 0,1%); nie może być wyższa od początkowej.

Przykład (instancja 20×5, ziarno generatora 1): typowe pogorszenie 35,009, więc
`T_pocz = 35,009 / ln(10) = 15,204`, a `T_końc = 1 / ln(1000) = 0,145`.

### Chłodzenie
Geometryczne: po każdej iteracji `T = T * c`, gdzie `c = (T_końc / T_pocz)^(1 / (iterations - 1))`. Dzięki temu
ostatnia iteracja ma zawsze temperaturę końcową, niezależnie od liczby iteracji. Dla powyższego przykładu i miliona
iteracji `c ≈ 0,9999953`.

## 4. Parametry
Podawane jako `--param nazwa=liczba`. Nieznana nazwa albo zła wartość kończy program komunikatem błędu.

| Nazwa | Domyślnie | Znaczenie | Zakres |
|---|---|---|---|
| `iterations` | 1000000 | liczba sprawdzonych sąsiadów | całkowita, 1 – 2000000000 |
| `start_prob` | 10 | szansa przyjęcia typowego pogorszenia na początku [%] | więcej niż 0, mniej niż 100 |
| `end_prob` | 0.1 | szansa przyjęcia pogorszenia o 1 na końcu [%] | więcej niż 0, mniej niż 100 |
| `insert` | 80 | udział przeniesień wśród ruchów [%], reszta to zamiany | całkowita, 0 – 100 |
| `seed` | 1 | ziarno losowania | całkowita, 0 – 2000000000 |

```
./main instances/b.txt --algo sa --param iterations=3000000 --param seed=7 --verbose
```

Z `--verbose` widać: makespan startowy, dobrane temperatury i 10 raportów w trakcie pracy (temperatura, bieżący
i najlepszy makespan, liczba przyjętych gorszych rozwiązań od poprzedniego raportu). Przy mniej niż 10 iteracjach
raport pojawia się po każdej.

## 5. Dlaczego nie jest to najprostsza wersja
- temperatury dobierane automatycznie do instancji,
- chłodzenie dopasowane do liczby iteracji,
- dwa rodzaje ruchów w ustalonej proporcji.

## 6. Dobór parametrów
Instancje z generatora (`p` z 1..99, `s` z 1..50). Strojenie na instancjach o ziarnach 11–13 (po 3 ziarna algorytmu),
średni makespan:

| Wariant (1 mln iteracji, `end_prob=0.1`, jeśli nie podano inaczej) | 20×5 | 50×10 | 100×20 | 200×20 |
|---|---|---|---|---|
| `start_prob=50`, `insert=50` | 1563,8 | 4129,3 | 8837,8 | 16158,1 |
| `start_prob=50`, `insert=0` (tylko zamiana) | 1578,6 | 4174,2 | 8927,9 | 16288,1 |
| `start_prob=50`, `insert=100` (tylko przeniesienie) | 1562,0 | 4124,3 | 8800,8 | 16142,9 |
| `start_prob=10`, `insert=80` (**domyślne**) | 1559,1 | 4106,7 | 8809,1 | 16140,9 |
| `start_prob=10`, `insert=100` | 1561,6 | 4115,2 | 8799,1 | 16117,8 |
| 200 tys. iteracji (`start_prob=50`, `insert=50`) | 1571,8 | 4170,0 | 8933,9 | 16343,7 |
| 3 mln iteracji (`start_prob=50`, `insert=50`) | 1557,9 | 4099,0 | 8787,2 | 16077,8 |

Wyraźnie widać dwie rzeczy: sama zamiana jest gorsza, a więcej iteracji pomaga. Różnice między najlepszymi wariantami
(i dla `end_prob` od 0,001 do 30) są małe i niejednoznaczne.

## 7. Porównanie z innymi algorytmami
Instancje **inne niż użyte do strojenia**, parametry domyślne, ziarno algorytmu 1.

30 instancji 9×5 (ziarna 1–30), optimum z `bruteforce`:

| Algorytm | optimum znalezione | średnia luka do optimum |
|---|---|---|
| NEH | 2 z 30 | 3,62% |
| GA | 27 z 30 | 0,05% |
| SA | 30 z 30 | 0,00% |

Średni makespan z 10 instancji (ziarna 1–10); w nawiasie średni czas (zależy od komputera):

| Rozmiar | NEH | GA | SA | SA lepszy od GA |
|---|---|---|---|---|
| 20×5 | 1678,6 (<0,01 s) | 1587,5 (0,2 s) | 1574,9 (0,1 s) | 9 z 10 |
| 50×10 | 4270,5 (<0,01 s) | 4109,5 (0,6 s) | 4075,6 (0,6 s) | 7 z 10 |
| 100×20 | 9021,0 (0,02 s) | 8845,0 (3,5 s) | 8723,4 (2,8 s) | 10 z 10 |
| 200×20 | 16394,6 (0,1 s) | 16360,6 (12,4 s) | 16054,6 (7,0 s) | 10 z 10 |

Przy parametrach domyślnych i podobnym lub krótszym czasie SA daje lepsze średnie niż GA. Dotyczy to instancji z tego
generatora; na danych o innych zakresach nie sprawdzaliśmy.

**Czas** (20 maszyn, 1 mln iteracji): 100 zadań ok. 3 s, 200 ok. 7 s, 500 ok. 29 s. Czas rośnie w przybliżeniu
proporcjonalnie do `iterations · n · m`.
