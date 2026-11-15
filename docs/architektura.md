# Architektura projektu

Status: **szkic do uzgodnienia z zespołem**. Rzeczy oznaczone (?) nie są jeszcze zdecydowane.
Plik `format.md` w tym katalogu jest propozycją formatu instancji i nie jest jeszcze zatwierdzony.

## 1. Przepływ danych

```
Generator ──> plik instancji ──> Instance::load ──> Algorithm::run(instance, params) ──> Result
 (osobny        (tekst)            (alg/, C++)         (klasy pochodne)                  (permutacja,
  program)                                                                                Cmax)
```

Zasady z wymagań, które z tego wynikają:
- generator jest **osobnym modułem**; algorytmy i aplikacja go nie wywołują ani nie zawierają,
- algorytm dostaje wyłącznie gotową `Instance` z pliku, nigdy nie losuje danych problemu,
- plik instancji jest jedynym łączem między generatorem a resztą (można podstawić zewnętrzny benchmark).

## 2. Moduły

| Moduł | Odpowiedzialność | Wejście → wyjście | Gdzie | Zależy od |
|---|---|---|---|---|
| Generator | tworzy pliki instancji z parametrów (n, m, ziarno, zakresy) | parametry → plik | `tools/` (język?) | nic |
| Instance | model danych: `n`, `m`, `p[i][j]`, `s[i][j][k]`; wczytuje i waliduje się sama (`Instance::load(path)`, `Instance::parse(strumień)`) | plik/tekst → `Instance` | `alg/` | nic |
| Evaluator | liczy makespan permutacji; opcjonalnie pełny harmonogram do Gantta | `Instance` + permutacja → Cmax | `alg/` | `Instance` |
| Algorithm | klasa bazowa algorytmów | `Instance` + parametry → `Result` | `alg/` | `Instance`, Evaluator |
| Algorytmy | klasy pochodne, po jednej na plik | jak wyżej | `alg/algorithms/` | Algorithm |
| Registry | lista dostępnych algorytmów (?) | nazwa → algorytm | `alg/` | Algorithm |
| Zapis wyniku | zapisuje `Result` (plik, ekran); poza `Algorithm` | `Result` → tekst | `alg/` | `Result` |
| Uruchamianie (CLI) | wczytaj plik, wybierz algorytm, zapisz wynik | argumenty → wynik | `alg/` | wszystko wyżej |
| Aplikacja GUI | wybór instancji i algorytmu, parametry, wynik, Gantt, logi | – | `app/` (Python) | komunikacja |
| Komunikacja | GUI ↔ algorytmy, **przez sockety** (etap 2) | – | `alg/` + `app/` | Registry |

## 3. Klasa `Algorithm` – co musi zapewnić

Ustalone:
- jedna klasa bazowa, każdy algorytm jako osobna klasa w osobnym pliku,
- wejście: `Instance` i parametry algorytmu, wyjście: `Result`,
- **instancja jest argumentem uruchomienia** (`run(instance, params)`), a nie stanem algorytmu: algorytm nie ma pamięci między uruchomieniami i może pracować na wielu instancjach,
- `Algorithm` **nie wczytuje danych i nie zapisuje wyniku**: wczytywanie robi `Instance::load`, zapis i przesyłanie robią warstwy wokół (CLI w etapie 1, serwer w etapie 2),
- `Instance::load` to statyczna fabryka: zwraca poprawną instancję albo zgłasza błąd, nie ma obiektu „w połowie wczytanego”.

Do ustalenia (?), bo od etapu 2 GUI tego potrzebuje:
- sposób opisu parametrów (nazwa, opis, wartość domyślna), żeby GUI mogło zbudować formularz,
- raportowanie postępu (aktualnie najlepszy Cmax) bez zależności od konsoli czy gniazda,
- możliwość przerwania obliczeń,
- co dokładnie zawiera `Result`: permutacja, Cmax, czas pracy, ewentualnie czasy startu.

Rdzeń algorytmów nie powinien nic wiedzieć o plikach, konsoli ani socketach – to robią warstwy wokół niego.

## 4. Etapy i granica etapu 1 / 2

- **Etap 1 (C++):** Generator, Instance (z wczytywaniem), Evaluator, Algorithm + algorytmy, zapis wyniku, CLI. Dane z pliku.
- **Etap 2 (Python):** GUI i komunikacja przez sockety. Etap 2 dokłada warstwę komunikacji, nie zmienia rdzenia.

## 5. Decyzje do podjęcia

1. **Algorytmy:** heurystyka, metaheurystyka, ewentualnie druga metaheurystyka i dokładny (?).
2. **Format pliku instancji:** własny czy zgodny z istniejącym benchmarkiem (?). Decyzja wpływa na generator i `Instance::load`. Jeśli kiedyś dojdzie drugi format, parsery można wydzielić bez zmiany reszty.
3. **Język generatora:** Python czy C++ (?).
4. **Model:** czy przezbrojenie może rozpocząć się przed nadejściem zadania z poprzedniej maszyny (do potwierdzenia z prowadzącym).
5. **Format komunikacji w etapie 2:** protokół wiadomości przez socket (?).
6. **Podział pracy** między osoby w zespole (?).
7. **Biblioteka GUI** w Pythonie (?).

## 6. Proponowana kolejność prac

1. Zatwierdzić ten dokument i decyzje 2–4.
2. Instance (z `load`/`parse`) + Evaluator, razem z testami.
3. Generator (równolegle, bo zależy tylko od formatu pliku).
4. Interfejs `Algorithm` i CLI.
5. Wybór i implementacja algorytmów.
6. Etap 2: protokół, komunikacja, GUI.
