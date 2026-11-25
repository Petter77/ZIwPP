# Algorytm genetyczny (`genetic`)

Metaheurystyka dla tematu 2. Kod: `alg/include/algorithms/genetic.h`, `alg/src/algorithms/genetic.cpp`.
Adaptacja własnego algorytmu dla problemu komiwojażera (https://github.com/Petter77/genetic-tsp-solver, w C).

## 1. Przebieg

```
wylosuj populację
dla każdego pokolenia:
    elita = kopia najlepszego osobnika -> lokalne przeszukiwanie
    resztę populacji wypełnij potomkami:
        rodzic A = turniej, rodzic B = turniej (inny osobnik niż A)
        dziecko = krzyżowanie(A, B)
        z prawdopodobieństwem "mutacja %" zmutuj dziecko
    czy jest nowy najlepszy wynik?
        tak -> mutacja wraca do wartości początkowej
        nie -> po `stagnation` pokoleniach bez poprawy mutacja rośnie o `mutation_step` (do `mutation_max`)
zwróć najlepszego osobnika
```

Osobnik (`Chromosome`) to permutacja zadań i jej makespan (funkcja celu, im mniej, tym lepiej).
Elita przechodzi do następnego pokolenia, więc najlepszy wynik nigdy się nie pogarsza.

## 2. Operatory

| Funkcja | Działanie |
|---|---|
| `randomChromosome` | losowa permutacja (tasowanie Fishera-Yatesa) |
| `tournamentSelect` | losuje `tournament` różnych osobników, wygrywa ten z najmniejszym makespanem |
| `crossover` | dwa losowe cięcia; fragment między nimi z rodzica A zostaje na swoich pozycjach, reszta pozycji jest wypełniana od lewej brakującymi zadaniami w kolejności z rodzica B |
| `mutate` | zamiana miejscami dwóch zadań |
| `localSearch` | przenoszenie jednego zadania na inną pozycję (wstawianie); ruch zostaje, jeśli skraca makespan, inaczej jest cofany; koniec, gdy pełne przejście nic nie poprawi |
| `nextGeneration` | tworzy nowe pokolenie: elita + potomkowie |

Przykład krzyżowania (cięcia na pozycjach 2 i 4):
```
rodzic A:   0 1 [2 3 4] 5 6
rodzic B:   6 5  4 3 2  1 0
dziecko:    6 5 [2 3 4] 1 0
```

## 3. Parametry
Podawane jako `--param nazwa=liczba` (w kodzie: mapa `Params`). Wszystkie są liczbami całkowitymi.
Nieznana nazwa albo zła wartość kończy program komunikatem błędu.

| Nazwa | Domyślnie | Znaczenie | Zakres |
|---|---|---|---|
| `population` | 500 | liczba osobników | 2 – 100000 |
| `generations` | 1000 | liczba pokoleń | 1 – 1000000 |
| `tournament` | 3 | liczba kandydatów w turnieju | od 1, mniej niż `population` |
| `mutation` | 5 | początkowe prawdopodobieństwo mutacji [%] | 0 – 100, nie więcej niż `mutation_max` |
| `mutation_step` | 2 | przyrost przy stagnacji [punkty procentowe] | 0 – 100 |
| `mutation_max` | 50 | górna granica mutacji [%] | 0 – 100 |
| `stagnation` | 20 | po tylu pokoleniach bez poprawy mutacja zaczyna rosnąć | 0 – 1000000 |
| `local_search` | 1 | lokalne przeszukiwanie elity: 1 włączone, 0 wyłączone | 0 lub 1 |
| `seed` | 1 | ziarno losowania | 0 – 2000000000 |

Turniej o rozmiarze równym populacji jest zabroniony: zawsze wygrywałby ten sam osobnik i nie dałoby się wybrać
dwóch różnych rodziców. To samo ziarno i te same parametry dają zawsze ten sam wynik.

```
./main instances/b.txt --param generations=2000 --param seed=7 --verbose
```

Z `--verbose` widać: parametry startowe, najlepszy wynik populacji losowej, każdą poprawę (z informacją, czy dało ją
lokalne przeszukiwanie, czy krzyżowanie/mutacja) i każdą zmianę prawdopodobieństwa mutacji.

## 4. Różnice względem wersji w C

| Zmiana | Powód |
|---|---|
| funkcja celu: makespan zamiast długości trasy | inny problem |
| tasowane są wszystkie pozycje (nie ma ustalonego pierwszego miasta) | trasę można zacząć w dowolnym mieście, a kolejność zadań nie jest cyklem |
| lokalne przeszukiwanie przez wstawianie zamiast 2-opt | odwracanie fragmentu pasuje do tras; we flowshopie typowym ruchem jest przeniesienie zadania. Obu wariantów nie porównywaliśmy na pomiarach |
| przeszukiwanie jest pomijane, gdy elita już przez nie przeszła | dałoby ten sam wynik, a kosztuje dużo czasu |
| rozmiar populacji 500 zamiast 1000 | dobrane pomiarami (niżej) |
| krzyżowanie z tablicą „zadanie użyte” | szybsze; działa też dla 2–4 zadań, gdzie poprzednie losowanie cięć się zapętlało |
| `std::vector` i `int` zamiast `malloc` i `uint8_t` | brak limitu 255 i ręcznego zwalniania pamięci |
| `std::mt19937` z ziarnem zamiast `arc4random` | powtarzalne wyniki, działa też poza BSD/macOS |
| mutacja nie przekracza `mutation_max` | poprzednio mogła dojść do 51% przy granicy 50% |

## 5. Pomiary
Instancje z generatora (`p` z 1..99, `s` z 1..50). Czasy zależą od komputera.

**Wpływ lokalnego przeszukiwania i liczby pokoleń** (populacja 200, jeśli nie podano inaczej): średni makespan
z 3 instancji (ziarna 11–13) po 3 ziarna algorytmu (1–3).

| Wariant | 20×5 | 50×10 | 100×10 | 100×20 |
|---|---|---|---|---|
| 300 pokoleń | 1607,9 | 4258,6 | 7787,0 | 9015,4 |
| 300 pokoleń, bez przeszukiwania | 1598,4 | 4266,7 | 8006,7 | 9237,9 |
| 1000 pokoleń | 1601,8 | 4243,0 | 7787,0 | 9010,1 |
| 1000 pokoleń, bez przeszukiwania | 1591,3 | 4230,8 | 7888,7 | 9109,0 |
| 2000 pokoleń | 1601,1 | 4239,4 | 7785,9 | 8999,2 |
| 1000 pokoleń, populacja 100 | 1606,7 | 4255,2 | 7840,0 | 9061,1 |
| **1000 pokoleń, populacja 500** (domyślne) | 1582,2 | 4216,7 | 7790,7 | 8981,2 |

Lokalne przeszukiwanie pomaga wyraźnie przy 100 zadaniach (o 1–3%). Przy 20–50 zadaniach i populacji 200 nie pomaga,
a czasem lekko szkodzi, prawdopodobnie przez szybsze upodobnienie populacji do elity (tego nie sprawdzaliśmy).

**Porównanie z optimum** (przegląd zupełny, 30 instancji 9×5, ziarna 1–30, parametry domyślne):
optimum znalezione w 27 z 30.

**Porównanie z prostymi punktami odniesienia** (po jednej instancji, ziarno 7; GA: 300 pokoleń, populacja 200):

| Instancja | kolejność z pliku | najlepsza z 200 000 losowych | GA |
|---|---|---|---|
| 20×5 | 2245 | 1908 | 1778 |
| 50×10 | 5186 | 4802 | 4353 |
| 100×10 | 9553 | 8831 | 7891 |
| 100×20 | 10891 | 10021 | 8942 |

**Czas przy parametrach domyślnych** (20 maszyn): 100 zadań ok. 3 s, 200 zadań ok. 9 s, 500 zadań ok. 155 s.
Dla bardzo dużych instancji czas rośnie głównie przez lokalne przeszukiwanie (jedno pełne przejście to około `n²`
ocen po `n·m` operacji); można je wyłączyć (`local_search=0`) albo zmniejszyć `generations`.

**Porównanie z NEH i optimum** (te same instancje, parametry domyślne GA, ziarno algorytmu 1).

30 instancji 9×5 (ziarna 1–30), optimum z `bruteforce`:

| Algorytm | optimum znalezione | średnia luka do optimum |
|---|---|---|
| NEH | 2 z 30 | 3,62% |
| GA | 27 z 30 | 0,03% |

Średni makespan z 10 instancji (ziarna 1–10) na rozmiar:

| Rozmiar | NEH (czas) | GA (czas) | GA lepszy od NEH |
|---|---|---|---|
| 20×5 | 1678,6 (<0,01 s) | 1593,7 (0,2 s) | 10 z 10 |
| 50×10 | 4270,5 (<0,01 s) | 4189,2 (0,5 s) | 8 z 10 |
| 100×20 | 9021,0 (0,02 s) | 8882,1 (2,6 s) | 10 z 10 |
| 200×20 | 16394,6 (0,12 s) | 16285,7 (9,8 s) | 8 z 10 |

Przewaga GA nad NEH maleje wraz z liczbą zadań (z ok. 5% do 0,7%), a NEH jest dużo szybszy.
Parametry domyślne GA były dobierane na innych instancjach z tego samego generatora.

## 6. Czego nie ma
Limitu czasu i przerywania obliczeń (potrzebne w etapie 2).
