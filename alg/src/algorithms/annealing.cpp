#include "algorithms/annealing.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "algorithms/common.h"
#include "evaluator.h"

namespace {

// ---------- parametry ----------

struct Settings {
  int iterations = 1000000; // liczba sprawdzonych sąsiadów
  double startProb = 10;    // [%] szansa przyjęcia typowego pogorszenia na początku
  double endProb = 0.1;     // [%] szansa przyjęcia pogorszenia o 1 na końcu
  int insert = 80;          // [%] udział ruchów "przenieś"; reszta to "zamień"
  int seed = 1;             // ziarno losowania (to samo ziarno = ten sam wynik)
};

double toPercent(const std::string &name, double value) {
  if (!(value > 0 && value < 100))
    throw std::invalid_argument("parametr " + name + " musi być większy od 0 i mniejszy od 100");
  return value;
}

Settings readSettings(const Params &params) {
  Settings cfg;
  for (const auto &[name, value] : params) {
    if (name == "iterations")
      cfg.iterations = toInt(name, value, 1, 2000000000);
    else if (name == "start_prob")
      cfg.startProb = toPercent(name, value);
    else if (name == "end_prob")
      cfg.endProb = toPercent(name, value);
    else if (name == "insert")
      cfg.insert = toInt(name, value, 0, 100);
    else if (name == "seed")
      cfg.seed = toInt(name, value, 0, 2000000000);
    else
      throw std::invalid_argument("nieznany parametr symulowanego wyżarzania: " + name);
  }
  return cfg;
}

// ---------- elementy algorytmu ----------

// Sąsiad: losowo przeniesione jedno zadanie albo zamienione dwa (n >= 2).
std::vector<int> neighbour(const std::vector<int> &jobs, int insertPercent, std::mt19937 &rng) {
  std::vector<int> next = jobs;
  auto [i, j] = twoPositions(rng, next.size());
  if (randomInt(rng, 100) < insertPercent)
    moveJob(next, i, j);
  else
    std::swap(next[i], next[j]);
  return next;
}

// Średnie pogorszenie makespanu po losowym ruchu z rozwiązania początkowego. Mówi, jaka jest "skala"
// różnic w tej instancji. Gdy żaden z próbnych ruchów nie pogarsza wyniku, zwraca 1.
double typicalWorsening(const Instance &inst, const std::vector<int> &jobs, int insertPercent, std::mt19937 &rng) {
  const int samples = 1000;
  int value = makespan(inst, jobs);
  double sum = 0;
  int count = 0;
  for (int k = 0; k < samples; ++k) {
    int delta = makespan(inst, neighbour(jobs, insertPercent, rng)) - value;
    if (delta > 0) {
      sum += delta;
      ++count;
    }
  }
  return count > 0 ? sum / count : 1.0;
}

// Temperatura, przy której pogorszenie o `delta` jest przyjmowane z prawdopodobieństwem `percent` %.
// Wynika z przekształcenia wzoru exp(-delta / T) = percent / 100.
double temperatureFor(double delta, double percent) { return -delta / std::log(percent / 100.0); }

// Lepszy lub równy sąsiad przechodzi zawsze, gorszy z prawdopodobieństwem exp(-delta / T).
bool accept(int delta, double temperature, std::mt19937 &rng) {
  if (delta <= 0)
    return true;
  double draw = rng() / 4294967296.0; // liczba z [0, 1); rng() daje 32 bity
  return draw < std::exp(-delta / temperature);
}

std::string format(double value) {
  char text[32];
  std::snprintf(text, sizeof text, "%.3f", value);
  return text;
}

} // namespace

Result Annealing::run(const Instance &inst, const Params &params, const Logger &log) const {
  Settings cfg = readSettings(params);

  if (inst.n < 2) // jedno zadanie: nie ma czego optymalizować
    return {{0}, makespan(inst, {0})};

  std::mt19937 rng(cfg.seed);
  std::vector<int> current = randomPermutation(rng, inst.n);
  int currentValue = makespan(inst, current);
  std::vector<int> best = current;
  int bestValue = currentValue;

  // Dobór temperatur i chłodzenia geometrycznego: po każdej iteracji T = T * cooling,
  // tak żeby w ostatniej iteracji T było równe temperaturze końcowej.
  double delta = typicalWorsening(inst, current, cfg.insert, rng);
  double startTemperature = temperatureFor(delta, cfg.startProb);
  double endTemperature = std::min(temperatureFor(1.0, cfg.endProb), startTemperature);
  double cooling =
      cfg.iterations > 1 ? std::pow(endTemperature / startTemperature, 1.0 / (cfg.iterations - 1)) : 1.0;

  log("start: makespan " + std::to_string(currentValue) + ", iteracje " + std::to_string(cfg.iterations) +
      ", ziarno " + std::to_string(cfg.seed));
  log("typowe pogorszenie " + format(delta) + ", temperatura od " + format(startTemperature) + " do " +
      format(endTemperature));

  double temperature = startTemperature;
  int reportEvery = std::max(1, cfg.iterations / 10); // 10 raportów w trakcie pracy
  int acceptedWorse = 0;                               // od ostatniego raportu

  for (int iteration = 1; iteration <= cfg.iterations; ++iteration) {
    std::vector<int> candidate = neighbour(current, cfg.insert, rng);
    int value = makespan(inst, candidate);

    if (accept(value - currentValue, temperature, rng)) {
      if (value > currentValue)
        ++acceptedWorse;
      current = std::move(candidate);
      currentValue = value;
      if (currentValue < bestValue) {
        best = current;
        bestValue = currentValue;
      }
    }

    if (iteration % reportEvery == 0) {
      log("iteracja " + std::to_string(iteration) + ": T = " + format(temperature) + ", bieżący " +
          std::to_string(currentValue) + ", najlepszy " + std::to_string(bestValue) + ", przyjęte gorsze " +
          std::to_string(acceptedWorse));
      acceptedWorse = 0;
    }
    temperature *= cooling;
  }

  log("koniec: najlepszy " + std::to_string(bestValue));
  return {best, bestValue};
}
