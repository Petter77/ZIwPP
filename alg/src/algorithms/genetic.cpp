#include "algorithms/genetic.h"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

#include "evaluator.h"

namespace {

struct Settings {
  int population = 500;   // liczba osobników
  int generations = 1000; // liczba pokoleń
  int tournament = 3;     // liczba kandydatów w turnieju
  int mutation = 5;       // początkowe prawdopodobieństwo mutacji [%]
  int mutationStep = 2;   // o ile rośnie przy stagnacji [punkty procentowe]
  int mutationMax = 50;   // górna granica [%]
  int stagnation = 20;    // po tylu pokoleniach bez poprawy mutacja zaczyna rosnąć
  int localSearch = 1;    // 1 - poprawiaj elitę lokalnym przeszukiwaniem, 0 - nie
  int seed = 1;           // ziarno losowania (to samo ziarno = ten sam wynik)
};

int toInt(const std::string &name, double value, int min, int max) {
  if (!(value >= min && value <= max) || value != static_cast<int>(value))
    throw std::invalid_argument("parametr " + name + " musi być liczbą całkowitą od " + std::to_string(min) + " do " +
                                std::to_string(max));
  return static_cast<int>(value);
}

Settings readSettings(const Params &params) {
  Settings cfg;
  for (const auto &[name, value] : params) {
    if (name == "population")
      cfg.population = toInt(name, value, 2, 100000);
    else if (name == "generations")
      cfg.generations = toInt(name, value, 1, 1000000);
    else if (name == "tournament")
      cfg.tournament = toInt(name, value, 1, 100000);
    else if (name == "mutation")
      cfg.mutation = toInt(name, value, 0, 100);
    else if (name == "mutation_step")
      cfg.mutationStep = toInt(name, value, 0, 100);
    else if (name == "mutation_max")
      cfg.mutationMax = toInt(name, value, 0, 100);
    else if (name == "stagnation")
      cfg.stagnation = toInt(name, value, 0, 1000000);
    else if (name == "local_search")
      cfg.localSearch = toInt(name, value, 0, 1);
    else if (name == "seed")
      cfg.seed = toInt(name, value, 0, 2000000000);
    else
      throw std::invalid_argument("nieznany parametr algorytmu genetycznego: " + name);
  }
  // Przy turnieju z całą populacją zawsze wygrywa ten sam osobnik i nie da się wybrać dwóch różnych rodziców.
  if (cfg.tournament >= cfg.population)
    throw std::invalid_argument("parametr tournament musi być mniejszy niż population");
  if (cfg.mutation > cfg.mutationMax)
    throw std::invalid_argument("parametr mutation nie może być większy niż mutation_max");
  return cfg;
}

// ---------- osobnik i populacja ----------

struct Chromosome {
  std::vector<int> jobs;  // permutacja zadań
  int fitness = 0;        // makespan (im mniej, tym lepiej)
  bool optimized = false; // true - po lokalnym przeszukiwaniu, nie ma czego powtarzać
};

using Population = std::vector<Chromosome>;

// Liczba z zakresu 0..limit-1. Bez std::uniform_int_distribution, żeby wynik dla danego ziarna
// nie zależał od kompilatora.
int randomInt(std::mt19937 &rng, int limit) { return static_cast<int>(rng() % limit); }

// Dwie różne pozycje z zakresu 0..n-1 (n >= 2).
std::pair<int, int> twoPositions(std::mt19937 &rng, int n) {
  int a = randomInt(rng, n);
  int b = randomInt(rng, n - 1);
  if (b >= a)
    ++b;
  return {a, b};
}

Chromosome randomChromosome(const Instance &inst, std::mt19937 &rng) {
  Chromosome chr;
  for (int job = 0; job < inst.n; ++job)
    chr.jobs.push_back(job);
  for (int i = inst.n - 1; i > 0; --i)
    std::swap(chr.jobs[i], chr.jobs[randomInt(rng, i + 1)]);
  chr.fitness = makespan(inst, chr.jobs);
  return chr;
}

int bestIndex(const Population &pop) {
  int best = 0;
  for (int i = 1; i < static_cast<int>(pop.size()); ++i)
    if (pop[i].fitness < pop[best].fitness)
      best = i;
  return best;
}

// ---------- operatory genetyczne ----------

// Losuje `size` różnych osobników i zwraca indeks najlepszego z nich.
int tournamentSelect(const Population &pop, int size, std::mt19937 &rng) {
  std::vector<int> candidates;
  while (static_cast<int>(candidates.size()) < size) {
    int index = randomInt(rng, pop.size());
    if (std::find(candidates.begin(), candidates.end(), index) == candidates.end())
      candidates.push_back(index);
  }

  int best = candidates[0];
  for (int index : candidates)
    if (pop[index].fitness < pop[best].fitness)
      best = index;
  return best;
}

// Fragment od pierwszego do drugiego cięcia pochodzi z rodzica A i zostaje na tych samych pozycjach.
// Pozostałe pozycje są wypełniane od lewej brakującymi zadaniami w kolejności z rodzica B.
Chromosome crossover(const Instance &inst, const Chromosome &a, const Chromosome &b, std::mt19937 &rng) {
  auto [cut1, cut2] = twoPositions(rng, inst.n);
  if (cut1 > cut2)
    std::swap(cut1, cut2);

  Chromosome child;
  child.jobs.assign(inst.n, -1); // -1 - pozycja jeszcze pusta
  std::vector<bool> used(inst.n, false);

  for (int i = cut1; i <= cut2; ++i) {
    child.jobs[i] = a.jobs[i];
    used[a.jobs[i]] = true;
  }

  int position = 0;
  for (int job : b.jobs) {
    if (used[job])
      continue;
    while (child.jobs[position] != -1)
      ++position;
    child.jobs[position] = job;
  }

  child.fitness = makespan(inst, child.jobs);
  return child;
}

// Zamienia miejscami dwa zadania.
void mutate(const Instance &inst, Chromosome &chr, std::mt19937 &rng) {
  auto [i, j] = twoPositions(rng, inst.n);
  std::swap(chr.jobs[i], chr.jobs[j]);
  chr.fitness = makespan(inst, chr.jobs);
  chr.optimized = false;
}

// Przenosi zadanie z pozycji `from` na pozycję `to`, reszta zachowuje kolejność.
void moveJob(std::vector<int> &jobs, int from, int to) {
  int job = jobs[from];
  jobs.erase(jobs.begin() + from);
  jobs.insert(jobs.begin() + to, job);
}

// Lokalne przeszukiwanie przez wstawianie: próbuje przenieść zadanie na inną pozycję. Ruch zostaje,
// jeśli skraca makespan, w przeciwnym razie jest cofany. Kończy, gdy pełne przejście nic nie poprawi.
void localSearch(const Instance &inst, Chromosome &chr) {
  bool improved = true;
  while (improved) {
    improved = false;
    for (int from = 0; from < inst.n; ++from) {
      for (int to = 0; to < inst.n; ++to) {
        if (from == to)
          continue;
        moveJob(chr.jobs, from, to);
        int fitness = makespan(inst, chr.jobs);
        if (fitness < chr.fitness) {
          chr.fitness = fitness;
          improved = true;
        } else {
          moveJob(chr.jobs, to, from);
        }
      }
    }
  }
  chr.optimized = true;
}

// Nowe pokolenie: na pozycji 0 elita (najlepszy z poprzedniego, ewentualnie ulepszony),
// reszta to potomkowie rodziców z turniejów, czasem zmutowani.
Population nextGeneration(const Instance &inst, const Population &pop, const Settings &cfg, int mutationRate,
                          std::mt19937 &rng) {
  Population next;
  next.reserve(pop.size());

  Chromosome elite = pop[bestIndex(pop)];
  if (cfg.localSearch && !elite.optimized)
    localSearch(inst, elite);
  next.push_back(std::move(elite));

  while (next.size() < pop.size()) {
    int first = tournamentSelect(pop, cfg.tournament, rng);
    int second = first;
    while (second == first)
      second = tournamentSelect(pop, cfg.tournament, rng);

    Chromosome child = crossover(inst, pop[first], pop[second], rng);
    if (randomInt(rng, 100) < mutationRate)
      mutate(inst, child, rng);
    next.push_back(std::move(child));
  }
  return next;
}

} // namespace

Result Genetic::run(const Instance &inst, const Params &params, const Logger &log) const {
  Settings cfg = readSettings(params);

  if (inst.n < 2) // jedno zadanie: nie ma czego optymalizować
    return {{0}, makespan(inst, {0})};

  std::mt19937 rng(cfg.seed);
  log("start: populacja " + std::to_string(cfg.population) + ", pokolenia " + std::to_string(cfg.generations) +
      ", turniej " + std::to_string(cfg.tournament) + ", ziarno " + std::to_string(cfg.seed) +
      ", lokalne przeszukiwanie " + (cfg.localSearch ? "tak" : "nie"));

  Population pop;
  for (int i = 0; i < cfg.population; ++i)
    pop.push_back(randomChromosome(inst, rng));

  int bestSoFar = pop[bestIndex(pop)].fitness;
  int stagnation = 0;
  int mutationRate = cfg.mutation;
  log("najlepszy w populacji losowej: " + std::to_string(bestSoFar));

  for (int generation = 1; generation <= cfg.generations; ++generation) {
    pop = nextGeneration(inst, pop, cfg, mutationRate, rng);
    int best = bestIndex(pop);

    if (pop[best].fitness < bestSoFar) {
      bestSoFar = pop[best].fitness;
      stagnation = 0;
      mutationRate = cfg.mutation;
      // elita stoi na pozycji 0, więc inny indeks oznacza, że poprawę dał potomek
      log("pokolenie " + std::to_string(generation) + ": nowy najlepszy " + std::to_string(bestSoFar) +
          (best == 0 ? " (lokalne przeszukiwanie elity)" : " (krzyżowanie/mutacja)") + ", mutacja " +
          std::to_string(mutationRate) + "%");
    } else {
      ++stagnation;
      if (stagnation > cfg.stagnation) {
        int raised = std::min(mutationRate + cfg.mutationStep, cfg.mutationMax);
        if (raised != mutationRate) {
          mutationRate = raised;
          log("pokolenie " + std::to_string(generation) + ": brak poprawy od " + std::to_string(stagnation) +
              " pokoleń, mutacja " + std::to_string(mutationRate) + "%");
        }
      }
    }
  }

  // Elita przechodzi dalej w każdym pokoleniu, więc najlepszy z ostatniej populacji to najlepszy znaleziony.
  const Chromosome &best = pop[bestIndex(pop)];
  log("koniec: najlepszy " + std::to_string(best.fitness));
  return {best.jobs, best.fitness};
}
