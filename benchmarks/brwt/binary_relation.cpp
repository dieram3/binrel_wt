#include <benchmark/benchmark_api.h>
#include <brwt/binary_relation.h>

#include <algorithm> // minmax
#include <cassert>   // assert
#include <cstddef>   // size_t
#include <random>    // mt19937, uniform_int_distribution
#include <vector>    // vector

using benchmark::DoNotOptimize;
using brwt::binary_relation;
using object_id = binary_relation::object_id;
using label_id = binary_relation::label_id;
using pair_type = binary_relation::pair_type;
using brwt::obj_major;
using brwt::lab_major;
using std::size_t;

namespace {
class binrel_generator {
public:
  binrel_generator(const size_t max_object, const size_t max_label)
      : object_dist(0, max_object), label_dist(0, max_label) {}

  auto gen_dataset(const size_t max_size) {
    std::vector<pair_type> pairs;
    pairs.reserve(max_size);
    for (size_t i = 0; i < max_size; ++i) {
      pairs.push_back(gen_pair());
    }
    return pairs;
  }

  auto gen_binary_relation(const size_t max_size) {
    return binary_relation(gen_dataset(max_size));
  }

  auto gen_object(const binary_relation& br) {
    assert(br.object_alphabet_size() > 0);
    using param_t = decltype(object_dist)::param_type;

    const auto max_obj = static_cast<size_t>(br.object_alphabet_size()) - 1;
    const auto object = object_dist(engine, param_t(0, max_obj));
    return static_cast<object_id>(object);
  }

  auto gen_label(const binary_relation& br) {
    assert(br.label_alphabet_size() > 0);
    using param_t = decltype(label_dist)::param_type;

    const auto max_lab = static_cast<size_t>(br.label_alphabet_size()) - 1;
    const auto label = label_dist(engine, param_t(0, max_lab));
    return static_cast<label_id>(label);
  }

  auto gen_pair(const binary_relation& br) {
    return pair_type{gen_object(br), gen_label(br)};
  }

  std::pair<object_id, object_id> gen_object_range(const binary_relation& br) {
    return std::minmax(gen_object(br), gen_object(br));
  }

  std::pair<label_id, label_id> gen_label_range(const binary_relation& br) {
    return std::minmax(gen_label(br), gen_label(br));
  }

private:
  object_id gen_object() {
    return static_cast<object_id>(object_dist(engine));
  }
  label_id gen_label() {
    return static_cast<label_id>(label_dist(engine));
  }
  pair_type gen_pair() {
    return pair_type{gen_object(), gen_label()};
  }

private:
  std::mt19937 engine{std::random_device{}()};
  std::uniform_int_distribution<size_t> object_dist;
  std::uniform_int_distribution<size_t> label_dist;
};
} // end namespace

template <typename T>
static T clamp(const T& v, const T& lo, const T& hi) {
  // return v < lo ? lo : hi < v ? hi : v;
  return static_cast<T>(
      long(v) < long(lo) ? long(lo) : long(hi) < long(v) ? long(hi) : long(v));
}

static void bm_rank(benchmark::State& state) {
  binrel_generator gen(/*max_object=*/100'000, /*max_label=*/1'000);
  const auto br = gen.gen_binary_relation(/*size=*/1'000'000);

  while (state.KeepRunning()) {
    auto max_object = gen.gen_object(br);
    auto max_label = gen.gen_label(br);
    br.rank(max_object, max_label);
  }
}
BENCHMARK(bm_rank);

static void bm_nth_element_lab_maj(benchmark::State& state) {
  binrel_generator gen(/*max_object=*/100'000, /*max_label=*/1'000);
  const auto br = gen.gen_binary_relation(/*size=*/1'000'000);

  while (state.KeepRunning()) {
    auto obj_range = gen.gen_object_range(br);
    auto min_label = gen.gen_label(br);
    br.nth_element(obj_range.first, obj_range.second, min_label, 42,
                   brwt::lab_major);
  }
}
BENCHMARK(bm_nth_element_lab_maj);

static void bm_nth_element_obj_maj(benchmark::State& state) {
  binrel_generator gen(/*max_object=*/100'000, /*max_label=*/1'000);
  const auto br = gen.gen_binary_relation(/*size=*/1'000'000);

  while (state.KeepRunning()) {
    auto min_object = gen.gen_object(br);
    auto lab_range = gen.gen_label_range(br);
    br.nth_element(min_object, lab_range.first, lab_range.second, 42,
                   obj_major);
  }
}
BENCHMARK(bm_nth_element_obj_maj);

static void bm_lower_bound(benchmark::State& state) {
  binrel_generator gen(/*max_object=*/100'000, /*max_label=*/1'000);
  const auto br = gen.gen_binary_relation(/*size=*/1'000'000);

  while (state.KeepRunning()) {
    auto lab_range = gen.gen_label_range(br);
    auto start = gen.gen_pair(br);
    start.label = clamp(start.label, lab_range.first, lab_range.second);

    br.lower_bound(start, lab_range.first, lab_range.second, obj_major);
  }
}
BENCHMARK(bm_lower_bound);

static void bm_obj_select(benchmark::State& state) {
  binrel_generator gen(/*max_object=*/100'000, /*max_label=*/1'000);
  const auto br = gen.gen_binary_relation(/*size=*/1'000'000);

  while (state.KeepRunning()) {
    auto start = gen.gen_object(br);
    auto label = gen.gen_label(br);
    br.obj_select(start, label, 42);
  }
}
BENCHMARK(bm_obj_select);

static void bm_count_distinct_labels(benchmark::State& state) {
  binrel_generator gen(/*max_object=*/100'000, /*max_label=*/1'000);
  const auto br = gen.gen_binary_relation(/*size=*/1'000'000);

  while (state.KeepRunning()) {
    auto obj_range = gen.gen_object_range(br);
    auto lab_range = gen.gen_label_range(br);

    br.count_distinct_labels(obj_range.first, obj_range.second, lab_range.first,
                             lab_range.second);
  }
}
BENCHMARK(bm_count_distinct_labels);
