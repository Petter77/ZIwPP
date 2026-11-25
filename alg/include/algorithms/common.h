#pragma once

#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Drobne funkcje wspólne dla metaheurystyk (genetic, sa).

// Liczba z zakresu 0..limit-1. Bez std::uniform_int_distribution, żeby wynik dla danego ziarna
// nie zależał od kompilatora.
inline int randomInt(std::mt19937 &rng, int limit) { return static_cast<int>(rng() % limit); }

// Dwie różne pozycje z zakresu 0..n-1 (n >= 2).
inline std::pair<int, int> twoPositions(std::mt19937 &rng, int n) {
  int a = randomInt(rng, n);
  int b = randomInt(rng, n - 1);
  if (b >= a)
    ++b;
  return {a, b};
}

// Losowa kolejność zadań 0..n-1 (tasowanie Fishera-Yatesa).
inline std::vector<int> randomPermutation(std::mt19937 &rng, int n) {
  std::vector<int> jobs;
  for (int job = 0; job < n; ++job)
    jobs.push_back(job);
  for (int i = n - 1; i > 0; --i)
    std::swap(jobs[i], jobs[randomInt(rng, i + 1)]);
  return jobs;
}

// Przenosi zadanie z pozycji `from` na pozycję `to`, reszta zachowuje kolejność.
inline void moveJob(std::vector<int> &jobs, int from, int to) {
  int job = jobs[from];
  jobs.erase(jobs.begin() + from);
  jobs.insert(jobs.begin() + to, job);
}

// Parametr całkowity z zakresu min..max; inaczej std::invalid_argument.
inline int toInt(const std::string &name, double value, int min, int max) {
  if (!(value >= min && value <= max) || value != static_cast<int>(value))
    throw std::invalid_argument("parametr " + name + " musi być liczbą całkowitą od " + std::to_string(min) + " do " +
                                std::to_string(max));
  return static_cast<int>(value);
}
