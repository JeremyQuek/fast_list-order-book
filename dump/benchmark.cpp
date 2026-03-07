#include <benchmark/benchmark.h>
#include <random>
#include "fast_list.h"

// =========================================================
// BENCHMARK: fast_list randomized
// =========================================================

static void BM_FastList_Add(benchmark::State& state) {
    const int N = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();
        {
            fast_list warmup(N);
            for (int i = 0; i < N; i++) warmup.addNode(i);
        }
        state.ResumeTiming();

        fast_list list(N);
        for (int i = 0; i < N; i++)
            list.addNode(i);

        benchmark::DoNotOptimize(list);
    }

    state.SetItemsProcessed(state.iterations() * N);
}

static void BM_FastList_Remove(benchmark::State& state) {
    const int N = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();

        fast_list list(N);
        for (int i = 0; i < N; i++)
            list.addNode(i);

        state.ResumeTiming();

        for (int i = 0; i < N; i += 2)
            list.removeNode(i);

        benchmark::DoNotOptimize(list);
    }

    state.SetItemsProcessed(state.iterations() * (N / 2));
}

static void BM_FastList_Consume(benchmark::State& state) {
    const int N = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();

        fast_list list(N);
        for (int i = 0; i < N; i++)
            list.addNode(i);

        state.ResumeTiming();

        while (list.list.count > 0)
            list.consumeNode();

        benchmark::DoNotOptimize(list);
    }

    state.SetItemsProcessed(state.iterations() * N);
}


static void BM_FastList_Mixed(benchmark::State& state) {
    const int N = state.range(0);

    for (auto _ : state) {
        state.PauseTiming();

        fast_list list(N);
        std::vector<int> ids;
        ids.reserve(N);

        std::mt19937 rng(42);
        std::uniform_int_distribution<int> op_dist(0, 4);
        std::uniform_int_distribution<int> val_dist(0, N);

        state.ResumeTiming();

        for (int i = 0; i < N; ++i) {
            int op = op_dist(rng);

            switch (op) {
                case 0: { // add back
                    list.addNode(val_dist(rng));
                    break;
                }
                case 1: { // add front
                    list.addNodeFront(val_dist(rng));
                    break;
                }
                case 2: { // remove random id
                    if (!ids.empty()) {
                        int idx = rng() % ids.size();
                        list.removeNode(ids[idx]);
                    }
                    break;
                }
                case 3: { // consume front
                    list.consumeNode();
                    break;
                }
                case 4: { // consume back
                    list.consumeNodeBack();
                    break;
                }
            }
        }

        benchmark::DoNotOptimize(list);
    }

    state.SetItemsProcessed(state.iterations() * N);
}



BENCHMARK(BM_FastList_Add)    ->Arg(1000000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_FastList_Remove) ->Arg(1000000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_FastList_Consume)->Arg(1000000)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_FastList_Mixed)  ->Arg(1000000)->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();