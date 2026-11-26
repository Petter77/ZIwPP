#include "algorithms/genetic.h"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

#include "algorithms/common.h"
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
  int localSearch = 1;    // ile procent potomków przechodzi lokalne ulepszanie (0 - wyłączone)
  int seed = 1;           // ziarno losowania (to samo ziarno = ten sam wynik)
};

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
      cfg.localSearch = toInt(name, value, 0, 100);
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
  std::vector<int> jobs; // permutacja zadań
  long long fitness = 0; // makespan (im mniej, tym lepiej)
};

using Population = std::vector<Chromosome>;

Chromosome randomChromosome(const Instance &inst, std::mt19937 &rng) {
  Chromosome chr;
  chr.jobs = randomPermutation(rng, inst.n);
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
}

// Lokalne ulepszanie: bierze jedno losowe zadanie, sprawdza je na każdej innej pozycji i zostawia na tej,
// która daje najkrótszy makespan. Jeśli żadna nie jest lepsza, nic nie zmienia. Koszt: n-1 ocen.
void improveRandomJob(const Instance &inst, Chromosome &chr, std::mt19937 &rng) {
  int from = randomInt(rng, inst.n);
  int bestPosition = from;
  for (int to = 0; to < inst.n; ++to) {
    if (to == from)
      continue;
    moveJob(chr.jobs, from, to);
    long long fitness = makespan(inst, chr.jobs);
    moveJob(chr.jobs, to, from); // cofnij, zadanie przeniesiemy dopiero po sprawdzeniu wszystkich pozycji
    if (fitness < chr.fitness) {
      chr.fitness = fitness;
      bestPosition = to;
    }
  }
  if (bestPosition != from)
    moveJob(chr.jobs, from, bestPosition);
}

// Nowe pokolenie: na pozycji 0 elita (kopia najlepszego z poprzedniego), reszta to potomkowie rodziców
// z turniejów, czasem zmutowani i czasem lokalnie ulepszeni.
Population nextGeneration(const Instance &inst, const Population &pop, const Settings &cfg, int mutationRate,
                          std::mt19937 &rng) {
  Population next;
  next.reserve(pop.size());
  next.push_back(pop[bestIndex(pop)]);

  while (next.size() < pop.size()) {
    int first = tournamentSelect(pop, cfg.tournament, rng);
    int second = first;
    while (second == first)
      second = tournamentSelect(pop, cfg.tournament, rng);

    Chromosome child = crossover(inst, pop[first], pop[second], rng);
    if (randomInt(rng, 100) < mutationRate)
      mutate(inst, child, rng);
    if (randomInt(rng, 100) < cfg.localSearch)
      improveRandomJob(inst, child, rng);
    next.push_back(std::move(child));
  }
  return next;
}

// ---------- statystyki do --verbose ----------

// Pokazuje, czy potomkowie są w stanie dogonić elitę i czy populacja jest jeszcze różnorodna.
// Niczego nie zmienia w populacji i nie używa losowania.
void logStatistics(const Population &pop, int generation, const Logger &log) {
  long long bestChild = pop[1].fitness;
  long long sum = 0;
  std::vector<std::vector<int>> children;
  for (size_t i = 1; i < pop.size(); ++i) { // pozycja 0 to elita
    bestChild = std::min(bestChild, pop[i].fitness);
    sum += pop[i].fitness;
    children.push_back(pop[i].jobs);
  }
  std::sort(children.begin(), children.end());
  int different = std::unique(children.begin(), children.end()) - children.begin();
  int count = pop.size() - 1;

  log("statystyki, pokolenie " + std::to_string(generation) + ": elita " + std::to_string(pop[0].fitness) +
      " | najlepsze dziecko " + std::to_string(bestChild) + " | średnie dziecko " +
      std::to_string((sum + count / 2) / count) + " | różnych dzieci " + std::to_string(different) + "/" +
      std::to_string(count));
}

} // namespace

Result Genetic::run(const Instance &inst, const Params &params, const Logger &log) const {
  Settings cfg = readSettings(params);

  if (inst.n < 2) // jedno zadanie: nie ma czego optymalizować
    return {{0}, makespan(inst, {0})};

  std::mt19937 rng(cfg.seed);
  log("start: populacja " + std::to_string(cfg.population) + ", pokolenia " + std::to_string(cfg.generations) +
      ", turniej " + std::to_string(cfg.tournament) + ", ziarno " + std::to_string(cfg.seed) +
      ", lokalne ulepszanie " + std::to_string(cfg.localSearch) + "% potomków");

  Population pop;
  for (int i = 0; i < cfg.population; ++i)
    pop.push_back(randomChromosome(inst, rng));

  long long bestSoFar = pop[bestIndex(pop)].fitness;
  int stagnation = 0;
  int mutationRate = cfg.mutation;
  log("najlepszy w populacji losowej: " + std::to_string(bestSoFar));

  int reportEvery = std::max(1, cfg.generations / 10);

  for (int generation = 1; generation <= cfg.generations; ++generation) {
    pop = nextGeneration(inst, pop, cfg, mutationRate, rng);
    int best = bestIndex(pop);

    // częściej na początku, bo wtedy populacja zmienia się najszybciej
    if (generation == 1 || generation == 10 || generation == 30 || generation % reportEvery == 0)
      logStatistics(pop, generation, log);

    if (pop[best].fitness < bestSoFar) {
      bestSoFar = pop[best].fitness;
      stagnation = 0;
      mutationRate = cfg.mutation;
      log("pokolenie " + std::to_string(generation) + ": nowy najlepszy " + std::to_string(bestSoFar) + ", mutacja " +
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
