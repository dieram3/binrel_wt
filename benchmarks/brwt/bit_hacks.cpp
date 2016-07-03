#include <benchmark/benchmark_api.h>
#include <brwt/bit_hacks.h>

#include <random> // mt19937, uniform_int_distribution

using benchmark::DoNotOptimize;

static void bm_8_pop_counts(benchmark::State& state) {
  using brwt::pop_count;
  using int_t = unsigned long long;
  const auto value = [] {
    std::random_device rd;
    std::mt19937 engine{rd()};
    std::uniform_int_distribution<int_t> gen_int;
    return gen_int(engine);
  }();

  auto ten_pop_count = [value] {
    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));

    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));
    DoNotOptimize(pop_count(value));
  };

  while (state.KeepRunning()) {
    ten_pop_count();
    ten_pop_count();
    ten_pop_count();
    ten_pop_count();
    ten_pop_count();

    ten_pop_count();
    ten_pop_count();
    ten_pop_count();
    ten_pop_count();
    ten_pop_count();
  }
}

BENCHMARK(bm_8_pop_counts);
