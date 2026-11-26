# Algorytm genetyczny (`genetic`)

Metaheurystyka dla tematu 2. Kod: `alg/include/algorithms/genetic.h`, `alg/src/algorithms/genetic.cpp`.
Adaptacja własnego algorytmu dla problemu komiwojażera (https://github.com/Petter77/genetic-tsp-solver, w C).

## 1. Przebieg

```
wylosuj populację
dla każdego pokolenia:
    elita = kopia najlepszego osobnika
    resztę populacji wypełnij potomkami:
        rodzic A = turniej, rodzic B = turniej (inny osobnik niż A)
        dziecko = krzyżowanie(A, B)
        z prawdopodobieństwem "mutacja %" zmutuj dziecko
        z prawdopodobieństwem "local_search %" ulepsz dziecko lokalnie
    czy jest nowy najlepszy wynik?
        tak -> mutacja wraca do wartości początkowej
        nie -> po `stagnation` pokoleniach bez poprawy mutacja rośnie o `mutation_step` (do `mutation_max`)
zwróć najlepszego osobnika
```

Osobnik (`Chromosome`) to permutacja zadań i jej makespan (funkcja celu, im mniej, tym lepiej).
Elita przechodzi do następnego pokolenia, więc najlepszy wynik nigdy się nie pogarsza.

### Model wyspowy (opcjonalny, `islands` > 1)
Zamiast jednej populacji działa kilka mniejszych („wysp”). Każda ma własną elitę, turnieje i potomków, a także
własny licznik zastoju i własny poziom mutacji. `population` to liczba osobników **łącznie** (dzielona po równo;
resztę dostają pierwsze wyspy), więc liczba ocen na pokolenie jest podobna jak przy jednej populacji.

Co `migration_interval` pokoleń następuje **migracja po pierścieniu** (1 → 2 → … → ostatnia → 1): kopia najlepszego
z wyspy zastępuje najgorszego na następnej. Najpierw zbierani są wszyscy migranci, więc jeden osobnik nie przechodzi
od razu przez kilka wysp. Migrant lepszy od dotychczasowej elity wyspy liczy się jako jej poprawa (mutacja wraca do
wartości początkowej). Najlepszy wynik nie może zginąć, bo zastępowany jest tylko najgorszy.

W kodzie: struktura `Island`, funkcje `migrate` i `adaptMutation`; `nextGeneration` i operatory są te same co dla jednej
populacji. Domyślnie `islands=1`, czyli algorytm działa jak opisany wyżej.

## 2. Operatory

| Funkcja | Działanie |
|---|---|
| `randomChromosome` | losowa permutacja (tasowanie Fishera-Yatesa) |
| `tournamentSelect` | losuje `tournament` różnych osobników, wygrywa ten z najmniejszym makespanem |
| `crossover` | dwa losowe cięcia; fragment między nimi z rodzica A zostaje na swoich pozycjach, reszta pozycji jest wypełniana od lewej brakującymi zadaniami w kolejności z rodzica B |
| `mutate` | zamiana miejscami dwóch zadań |
| `improveRandomJob` | lokalne ulepszanie: jedno losowe zadanie jest sprawdzane na każdej innej pozycji i trafia na najlepszą, jeśli skraca to makespan (`n-1` ocen) |
| `nextGeneration` | tworzy nowe pokolenie: elita + potomkowie (mutacja i lokalne ulepszanie) |

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
| `population` | 500 | liczba osobników (łącznie na wszystkich wyspach) | 2 – 100000 |
| `islands` | 1 | liczba wysp; 1 – jedna populacja | 1 – 1000; każda wyspa ma mieć co najmniej 2 osobniki i więcej niż `tournament` |
| `migration_interval` | 50 | co ile pokoleń migracja (przy `islands` > 1); 0 – bez migracji | 0 – 1000000 |
| `generations` | 1000 | liczba pokoleń | 1 – 1000000 |
| `tournament` | 3 | liczba kandydatów w turnieju | od 1, mniej niż `population` |
| `mutation` | 5 | początkowe prawdopodobieństwo mutacji [%] | 0 – 100, nie więcej niż `mutation_max` |
| `mutation_step` | 2 | przyrost przy stagnacji [punkty procentowe] | 0 – 100 |
| `mutation_max` | 50 | górna granica mutacji [%] | 0 – 100 |
| `stagnation` | 20 | po tylu pokoleniach bez poprawy mutacja zaczyna rosnąć | 0 – 1000000 |
| `local_search` | 1 | jaki procent potomków przechodzi lokalne ulepszanie; 0 wyłącza | 0 – 100 |
| `seed` | 1 | ziarno losowania | 0 – 2000000000 |

Turniej o rozmiarze równym populacji jest zabroniony: zawsze wygrywałby ten sam osobnik i nie dałoby się wybrać
dwóch różnych rodziców. To samo ziarno i te same parametry dają zawsze ten sam wynik.

```
./main instances/b.txt --param generations=2000 --param seed=7 --verbose
```

Z `--verbose` widać: parametry startowe, najlepszy wynik populacji losowej, każdą poprawę i każdą zmianę prawdopodobieństwa mutacji.

Dodatkowo w pokoleniach 1, 10, 30 i co 1/10 wszystkich pokoleń pojawia się linia ze statystykami:
```
[genetic] statystyki, pokolenie 100: elita 16246 | najlepsze dziecko 16250 | średnie dziecko 16589 | różnych dzieci 499/499
```
| Pole | Znaczenie |
|---|---|
| elita | makespan osobnika na pozycji 0 (najlepszy z poprzedniego pokolenia) |
| najlepsze dziecko | najlepszy z pozostałych osobników; mniej niż elita oznacza, że potomek ją pobił |
| średnie dziecko | średni makespan potomków (zaokrąglony); gdy jest bliski elity, populacja się do niej upodobniła |
| różnych dzieci | liczba różnych permutacji wśród potomków (nie mówi, jak bardzo się różnią) |

## 4. Różnice względem wersji w C

| Zmiana | Powód |
|---|---|
| funkcja celu: makespan zamiast długości trasy | inny problem |
| tasowane są wszystkie pozycje (nie ma ustalonego pierwszego miasta) | trasę można zacząć w dowolnym mieście, a kolejność zadań nie jest cyklem |
| zamiast pełnego 2-opt na elicie: lekkie ulepszanie (przeniesienie jednego zadania) 1% potomków | pełne przeszukiwanie elity prowadziło do wczesnego zastoju na dużych instancjach (punkt 5); przeniesienie zadania pasuje do flowshopu bardziej niż odwracanie fragmentu trasy (2-opt nie mierzyliśmy) |
| rozmiar populacji 500 zamiast 1000 | dobrane pomiarami (niżej) |
| krzyżowanie z tablicą „zadanie użyte” | szybsze; działa też dla 2–4 zadań, gdzie poprzednie losowanie cięć się zapętlało |
| `std::vector` i `int` zamiast `malloc` i `uint8_t` | brak limitu 255 i ręcznego zwalniania pamięci |
| `std::mt19937` z ziarnem zamiast `arc4random` | powtarzalne wyniki, działa też poza BSD/macOS |
| mutacja nie przekracza `mutation_max` | poprzednio mogła dojść do 51% przy granicy 50% |

## 5. Pomiary
Instancje z generatora (`p` z 1..99, `s` z 1..50). Czasy zależą od komputera.

### Skąd obecna postać
Pierwsza wersja robiła **pełne lokalne przeszukiwanie elity** (przenoszenie zadań aż do braku poprawy). Na dużych
instancjach elita już w 1. pokoleniu była dużo lepsza od reszty, a dalsze pokolenia dawały niewiele (np. 100×100:
15734 po 1 pokoleniu, 15730 po 1000). Sprawdzone warianty (instancje o ziarnach 11–13, po 3 ziarna algorytmu):

1. **Inne krzyżowania** (SBOX, SJOX, dwupunktowe, zachowujące sąsiadów) – żadne nie było wyraźnie lepsze od obecnego,
   więc krzyżowanie zostało bez zmian.
2. **Łagodniejsze ulepszanie** – przy zbliżonym czasie wszystkie warianty bez pełnego przeszukiwania elity miały lepsze
   średnie od pierwszej wersji. Różnice między nimi były małe (w tym: bez żadnego przeszukiwania, ulepszanie 1% lub
   5% potomków, mutacja przez przeniesienie zamiast zamiany). Wybrane zostało ulepszanie 1% potomków.

| Wariant (ok. ten sam czas) | 20×5 | 50×10 | 100×10 | 100×20 |
|---|---|---|---|---|
| pierwsza wersja, 3300 pokoleń | 1580,2 | 4204,2 | 7786,2 | 8977,2 |
| bez przeszukiwania, 4500 pokoleń | 1570,2 | 4172,1 | 7753,2 | 8932,9 |
| 1% potomków, 2500 pokoleń | 1575,7 | 4164,8 | 7712,9 | 8939,6 |

### Sprawdzenie na innych instancjach
Instancje nieużyte do wyboru wariantu, parametry domyślne, ziarno algorytmu 1.

30 instancji 9×5 (ziarna 1–30), optimum z `bruteforce`:

| Algorytm | optimum znalezione | średnia luka do optimum |
|---|---|---|
| NEH | 2 z 30 | 3,62% |
| GA, pierwsza wersja | 27 z 30 | 0,03% |
| GA, obecny | 27 z 30 | 0,05% |
| SA | 30 z 30 | 0,00% |

Średni makespan z 10 instancji (ziarna 1–10); w nawiasie średni czas:

| Rozmiar | NEH | GA, pierwsza wersja | GA, obecny | obecny lepszy od pierwszej | SA |
|---|---|---|---|---|---|
| 20×5 | 1678,6 | 1593,7 (0,2 s) | 1587,5 (0,2 s) | 6 z 10 | 1574,9 |
| 50×10 | 4270,5 | 4189,2 (0,5 s) | 4109,5 (0,6 s) | 10 z 10 | 4075,6 |
| 100×10 | 7807,2 | 7735,8 (1,3 s) | 7644,6 (1,7 s) | 9 z 10 | 7523,2 |
| 100×20 | 9021,0 | 8882,1 (2,6 s) | 8845,0 (3,5 s) | 7 z 10 | 8723,4 |
| 200×20 | 16394,6 | 16285,7 (9,4 s) | 16360,6 (12,4 s) | 4 z 10 | 16054,6 |
| 100×100 | 15691,2 | 15656,4 (18,2 s) | 15563,4 (24,6 s) | 8 z 10 | 15452,6 |

Obecna wersja ma lepsze średnie dla pięciu z sześciu rozmiarów (o 0,4–1,9%), ale jest ok. 1,3 raza wolniejsza dla
dużych instancji i **gorsza dla 200×20** (o 0,5%). SA ma lepsze średnie od obu wersji.

### Znana słabość: zastój
Problem jest mniejszy, ale nie zniknął. W sprawdzonych przebiegach 100×100 i 200×20 poprawy trwały do kilkuset
pokoleń, a potem wynik stał: średni potomek jest blisko elity i kolejne pokolenia jej nie pobijają, nawet przy mutacji
50%. Dla 200×20 (ziarno generatora 11) 2000 pokoleń dało to samo co 1000.

**Czas przy parametrach domyślnych:** 100×20 ok. 3,5 s, 200×20 ok. 12 s, 100×100 ok. 25 s.

### Model wyspowy – wynik pomiarów
Strojenie na instancjach o ziarnach 11–13 (po 3 ziarna algorytmu), łącznie 500 osobników. „Zmiana” to średnia
z procentowych zmian średniego makespanu względem jednej populacji przy tej samej liczbie pokoleń (minus = lepiej).

| Pokolenia | Wyspy / migracja co | 20×5 | 50×10 | 100×10 | 100×20 | 200×20 | zmiana |
|---|---|---|---|---|---|---|---|
| 1000 | 1 (bez wysp) | 1577,9 | 4172,0 | 7739,0 | 8951,0 | 16429,9 | – |
| 1000 | 2 / 50 | 1579,8 | 4204,9 | 7785,8 | 8960,6 | 16489,4 | +0,40% |
| 1000 | 5 / bez migracji | 1573,9 | 4198,8 | 7804,1 | 9000,2 | 16524,0 | +0,47% |
| 1000 | 5 / 25 | 1570,7 | 4200,3 | 7780,8 | 8977,4 | 16498,6 | +0,29% |
| 1000 | 5 / 200 | 1565,8 | 4214,3 | 7799,1 | 8957,6 | 16512,9 | +0,32% |
| 1000 | 10 / 50 | 1566,3 | 4196,7 | 7788,7 | 8991,3 | 16591,7 | +0,39% |
| 3000 | 1 (bez wysp) | 1575,4 | 4161,6 | 7710,6 | 8939,6 | 16350,3 | – |
| 3000 | 5 / 50 | 1568,2 | 4187,6 | 7745,7 | 8949,2 | 16389,1 | +0,19% |
| 3000 | 5 / 200 | 1565,8 | 4183,3 | 7740,0 | 8917,0 | 16325,7 | −0,02% |
| 3000 | 5 / 500 | 1569,8 | 4168,9 | 7712,1 | 8916,3 | 16341,3 | −0,10% |

Sprawdzone były też warianty 1000 pokoleń: 5 / 10, 5 / 50, 5 / 100, 10 / 25 i 10 / 100 (zmiana od +0,36% do +0,43%).

**Wniosek:** wyspy nie dały wyraźnej poprawy. Przy 1000 pokoleń pomagały tylko na 20×5, a średnio były gorsze. Przy 3000
pokoleń z rzadką migracją wypadały mniej więcej na równi z jedną populacją (różnice w granicach szumu). Dlatego domyślnie
są wyłączone. Test dla 6000 pokoleń został przerwany, zanim policzył warianty z wyspami.

## 6. Czego nie ma
Limitu czasu i przerywania obliczeń (potrzebne w etapie 2).
