# Format pliku instancji

Status: **propozycja do zatwierdzenia przez zespół**. Dotyczy tematu 2 z `files/tematy.pdf`:
permutacyjny problem przepływowy z przezbrojeniami zależnymi od kolejności, kryterium: makespan.

Format jest własny (wymóg etapu 1: „konkretny format danych wejściowych”) i jest wspólny dla generatora,
wczytywania w `Instance` i plików referencyjnych.

## Oznaczenia (jak w dokumencie prowadzącego)
- `n` – liczba zadań, `m` – liczba maszyn,
- `p[i][j]` – czas operacji zadania `j` na maszynie `i`,
- `s[i][j][k]` – czas przezbrojenia na maszynie `i` z zadania `j` na zadanie `k`,
- `s[i][k][k]` – przezbrojenie przed zadaniem `k` wykonywanym na maszynie jako pierwsze (w dokumencie: „j = k”).

Indeksy `i`, `j`, `k` w pliku są niejawne (kolejność liczb), w kodzie liczone od 0.

## Układ pliku
Plik tekstowy z liczbami całkowitymi oddzielonymi białymi znakami (spacje, tabulatory, nowe linie, także CRLF).
Kolejność:

```
n m
p[0][0] ... p[0][n-1]            <- m linii po n liczb (wiersz = maszyna)
...
p[m-1][0] ... p[m-1][n-1]
s[0][0][0] ... s[0][0][n-1]      <- dla każdej maszyny n linii po n liczb (wiersz = zadanie j, kolumna = zadanie k)
...
s[m-1][n-1][0] ... s[m-1][n-1][n-1]
```
Razem `1 + m + m·n` linii z danymi. Między częściami (nagłówek, `p`, `s`) wstawiamy dla czytelności jedną pustą linię;
puste linie są ignorowane przy wczytywaniu, tak jak każde inne białe znaki.

## Walidacja przy wczytywaniu
- `n ≥ 1`, `m ≥ 1`,
- `p[i][j] ≥ 1`,
- `s[i][j][k] ≥ 1` (także przekątna `s[i][k][k]`: dokument wymaga `s ∈ N+`),
- dokładnie tyle liczb, ile wynika z `n` i `m` – brak liczb lub nadmiar to błąd,

Wczytywanie rzuca błąd z czytelnym komunikatem (wymaganie „odporna na błędy” w etapie 2).

## Przykład (n = 3, m = 2)
```
3 2
10 20 30
15 25 5
1 2 3
2 1 3
3 2 1
4 5 6
5 4 6
6 5 4
```

## Model (z dokumentu prowadzącego)
- Zadanie przechodzi przez maszyny 1 → 2 → … → m; kolejność zadań jest taka sama na każdej maszynie.
- Przed każdą operacją odbywa się przezbrojenie `s[i][poprzednie][bieżące]`, dla pierwszego zadania `s[i][k][k]`.
- Maszyna wykonuje naraz jedno przezbrojenie albo jedno zadanie albo jest bezczynna; brak wywłaszczeń.
- Operacja zadania zaczyna się po zakończeniu poprzedniej operacji tego zadania.
- Kryterium: makespan.

## Zgodność z dokumentem prowadzącego
Format i walidacja nie dopuszczają niczego, czego dokument nie przewiduje: wszystkie `p` i `s` są liczbami
naturalnymi dodatnimi, przezbrojenie początkowe `s[i][k][k]` jest dodatnie. Zewnętrzne benchmarki
z zerowym przezbrojeniem początkowym nie spełniają tego warunku, więc nie są używane jako instancje ani jako źródło
wartości referencyjnych.

## Przyjęte założenie: kiedy zaczyna się przezbrojenie
Dokument prowadzącego tego nie rozstrzyga, więc przyjmujemy wariant, który stosuje literatura dla PFSP-SDST
(przezbrojenia „anticipatory”): **maszyna zaczyna przezbrojenie od razu po zakończeniu poprzedniego zadania**,
nie czekając na nadejście zadania z maszyny `i-1`. Zadanie czeka na maszynie, jeśli przezbrojenie skończyło się
wcześniej, niż zadanie nadeszło.

Czas zakończenia zadania na pozycji `k` permutacji `π` na maszynie `i`:

```
C[i][k] = max( C[i][k-1] + s[i][π(k-1)][π(k)] , C[i-1][k] ) + p[i][π(k)]
```
- `C[-1][k] = 0` (zadanie dostępne od początku na pierwszej maszynie),
- `C[i][-1] = 0` (maszyna wolna od chwili 0),
- dla pierwszego zadania `π(k-1) = π(k)`, czyli przezbrojenie `s[i][π(0)][π(0)]`,
- makespan = `C[m-1][n-1]`.

Źródło wzoru: Santos i współautorzy, tabu search dla PFSP z SDST (strona 3), z tą różnicą, że tam
przezbrojenie początkowe jest zerowe, a u nas dodatnie (dokument prowadzącego).

Nie zmienia to formatu pliku, ale zmienia wartości makespanu. 
