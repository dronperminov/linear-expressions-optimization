# LEO: Linear Expressions Optimization

LEO is a C++17 library and command-line tool for **reducing the number of additions and subtractions** needed to evaluate a system of linear expressions with
integer coefficients. The main focus is on expressions with small coefficients (mostly ternary, `-1, 0, 1`), although formally nothing in the framework is
restricted to them: with larger coefficients the results may simply be worse.

The primary use case is optimizing fast matrix multiplication schemes. Their encoding and decoding matrices are systems of linear expressions, and the number of
additions in them directly affects the practical cost of the algorithm. Some schemes also use fractional coefficients; these are not supported yet, but may be
in the future. The library itself is not tied to matrix multiplication and accepts **any** system of integer linear expressions.

**Contents:** [Quick start](#quick-start) · [Features](#features) · [The `reduce` tool](#the-reduce-tool) · [Using LEO as a library](#using-leo-as-a-library) ·
[Architecture](#architecture) · [Extending LEO](#extending-leo) · [Contributing](#contributing) · [References](#references) · [License](#license)

## Quick start

You need `g++` with C++17 and OpenMP support, and `make`.

```bash
git clone https://github.com/dronperminov/linear-expressions-optimization
cd linear-expressions-optimization
make -j$(nproc) all
```

The build produces:
- The static library (`build/lib/libleo.a`).
- The command-line reduction tool (`./reduce` in the repository root).

Create an `input.txt` file. The first line is the number of expressions and the number of variables, followed by one line of coefficients per expression:

```
5 4
1 1 1 1
1 1 1 0
1 1 0 1
1 0 1 1
0 1 1 1
```

**The shortest run:**

```bash
./reduce -i input.txt -o output.json
```

**Run with the recommended parameters:**

```bash
./reduce -i input.txt -o output.txt -T -b -p -r 25 --vec-iterations 100 --cse-iterations 50 --cse-first-round-only --validate --vectors-ratio 0.7 --substitutions-ratio 0.25 --exchange-max-remove 2
```

| Option                       | Meaning                                                                                                             |
|------------------------------|---------------------------------------------------------------------------------------------------------------------|
| `-T`                         | also solve the transposed system and transpose the solution back                                                    |
| `-b`                         | stop solvers that can no longer beat the best solution found so far                                                 |
| `-p`                         | print all diagnostics (arguments, system statistics, per-round results)                                             |
| `-r 25`                      | run 25 optimization rounds                                                                                          |
| `--vec-iterations 100`       | 100 vector covering solver runs per round                                                                           |
| `--cse-iterations 50`        | 50 CSE solver runs per round                                                                                        |
| `--cse-first-round-only`     | run CSE solver only in the first round                                                                              |
| `--vectors-ratio 0.7`        | 70% of tasks in later rounds start from the vectors of earlier solutions                                            |
| `--substitutions-ratio 0.25` | 25% of tasks start from a chain of substitutions taken from earlier solutions (the remaining 5% start from scratch) |
| `--exchange-max-remove 2`    | enable polishing, removing up to 2 helpers at a time                                                                |
| `--validate`                 | independently verify the final solution                                                                             |

## Features

**Reduction tool (`reduce`)**

- Reads systems from `.txt` and `.sms` files.
- Writes solutions as `.txt`, `.slp` or `.json`, or prints them to stdout. The format is detected from the output extension automatically.
- Templates in the output path (`{additions}`, `{substitutions}`, ...).
- Multi-round optimization: later rounds restart solvers from earlier solutions.
- Solving the transposed system and transposing the solution back.
- Solution polishing, sign-inversion optimization, inlining of single-use substitutions, validation.
- Multi-threaded (OpenMP); runs are reproducible with `--seed` when `--threads 1` is used.

**Library**

- A universal solver interface with just two methods: `solve()` and `getSolution()`.
- Two solver families: common subexpression elimination (CSE) and vector covering.
- Heuristics (scorers) and candidate selection rules (selectors) are independent of the solver algorithms and freely combinable.
- A parallel `Reducer` that runs pools of strategies over several rounds and polishes results.
- Ready-made strategy presets, formatters, and solution utilities (transposition, inlining, sign optimization, validation).

## The `reduce` tool

```bash
./reduce -i <input> [-o <output>] [-f <format>] [other options]
```

If `-o` is omitted, the result is written to `output.txt`. Run `./reduce --help` for the list of options.

### Input formats

The format is chosen by the input file extension: `.sms` is read as SMS, everything else as plain text. In both formats any number of blank lines and comment
lines starting with `#` is allowed.

**`.txt`**: a dense matrix. Each row is an expression, each column is a variable.

```
expressions dimension
a_11 a_12 ... a_1d
a_21 a_22 ... a_2d
...
a_e1 a_e2 ... a_ed
```

**`.sms`**: a sparse matrix. Indices are **1-based**, omitted entries are zero, and the list is terminated by the line `0 0 0`.

```
expressions dimension R
i_1 j_1 a_{i_1 j_1}
i_2 j_2 a_{i_2 j_2}
...
i_n j_n a_{i_n j_n}
0 0 0
```

Here is the same system as in the quick start, in SMS format:

```
5 4 R
1 1 1
1 2 1
1 3 1
1 4 1
2 1 1
2 2 1
2 3 1
3 1 1
3 2 1
3 4 1
4 1 1
4 3 1
4 4 1
5 2 1
5 3 1
5 4 1
0 0 0
```

### Output formats

The output format is set with `-f` / `--format` and defaults to `auto`: the format is chosen from the output file extension, so **you do not have to specify it**
for the standard extensions. Specify it explicitly only when the extension is different.

```bash
# the format is detected from the extension
./reduce -i examples/cse_counterexample.txt -o output.txt
./reduce -i examples/cse_counterexample.txt -o output.slp
./reduce -i examples/cse_counterexample.txt -o output.json

# explicit format for a non-standard extension
./reduce -i examples/cse_counterexample.txt -o output.md -f txt
```

#### `txt`: human-readable

Input variables are `x1..xd`, substitutions continue the numbering, outputs are `y1..ye`:

```
# 6 additions
# substitutions
x5 = x1 + x2
x6 = x3 + x5
x7 = x4 + x6
x8 = x7 - x2
x9 = x4 + x5
x10 = x7 - x1

# expressions
y1 = x7
y2 = x6
y3 = x9
y4 = x8
y5 = x10
```

#### `slp`: straight-line program

Zero-based numbering: inputs `i0..`, temporaries `t0..`, outputs `o0..`.

```
t0:=i0+i1;
t1:=i2+t0;
t2:=i3+t1;
t3:=t2-i1;
t4:=i3+t0;
t5:=t2-i0;
o0:=t2;
o1:=t1;
o2:=t4;
o3:=t3;
o4:=t5;
```

#### `json`: for programmatic use

`i` and `j` are the indices of the operands of a substitution (input variables are numbered from zero, new ones continue the numbering), `ai` and `aj` are their
coefficients.

```json
{
    "additions": 6,
    "substitutions": [
        {"i": 0, "ai": 1, "j": 1, "aj": 1},
        {"i": 2, "ai": 1, "j": 4, "aj": 1},
        {"i": 3, "ai": 1, "j": 5, "aj": 1},
        {"i": 6, "ai": 1, "j": 1, "aj": -1},
        {"i": 3, "ai": 1, "j": 4, "aj": 1},
        {"i": 6, "ai": 1, "j": 0, "aj": -1}
    ],
    "expressions": [
        [{"index": 6, "value": 1}],
        [{"index": 5, "value": 1}],
        [{"index": 8, "value": 1}],
        [{"index": 7, "value": 1}],
        [{"index": 9, "value": 1}]
    ]
}
```

The exact solution you get may differ from the ones shown above, since it depends on the seed and the strategies. The number of additions for this system is 6.

#### Printing to stdout

Use `-o stdout` and **always** specify the format (`auto` has no extension to rely on). Add `-q` so that nothing but the solution is printed:

```bash
./reduce -i examples/cse_counterexample.txt -q -o stdout -f slp
```

This makes `reduce` convenient to use in pipelines and scripts.

### Output path templates

The output path may contain templates that are replaced by properties of the final solution:

| Template          | Replaced with                                                        |
|-------------------|----------------------------------------------------------------------|
| `{additions}`     | number of additions in the solution                                  |
| `{inversions}`    | number of inversions (sign changes) in expressions and substitutions |
| `{substitutions}` | number of substitutions used                                         |
| `{dimension}`     | number of variables of the input system                              |

```bash
./reduce -i examples/cse_counterexample.sms -o output_{additions}.slp
# the result is saved to output_6.slp
```

Missing parent directories are created automatically, so paths like `results/{dimension}/solution_{additions}.json` work too.

### Multiple optimization rounds

Option `-r N` runs `N` rounds.

1. **First round.** Solvers of the selected strategies are run from scratch, each with its own random seed. All solutions found are collected in a pool.
2. **Subsequent rounds.** The vector covering solver can start not from scratch but from earlier solutions. Each task randomly picks one of three modes; the
   shares are controlled by options:
    - **from scratch**: share `1 - substitutions-ratio - vectors-ratio`;
    - **from a chain of substitutions** (`--substitutions-ratio`): the starting chain is built randomly from substitutions used in other solutions, with weights
      based on the number of additions, so better solutions contribute more. The chain length is a share of the best solution's additions
      (`--min-additions-ratio`, `--max-additions-ratio`);
    - **from a set of vectors** (`--vectors-ratio`): one earlier solution is picked at random (better solutions are more likely to be chosen), and from
      `--min-remove-vectors` to `--max-remove-vectors` of its intermediate vectors are removed. The solver then rebuilds the rest: each remaining vector is built
      as soon as it can be obtained with a single addition, and the greedy search completes whatever is still missing.

The sum of `--substitutions-ratio` and `--vectors-ratio` must not exceed 1. With the defaults (`0.5` and `0.5`) no task starts from scratch after the first
round. Tasks of solvers that cannot start from earlier solutions (such as CSE) always run from scratch.

Optimization stops early once the lower bound on the number of additions is reached (the solution is then optimal). The bound counts the distinct targets
(rows with at least two nonzero entries, up to sign) and adds one if the targets cannot all be built from one another starting from the input variables; it is
printed by `--print-system-stats`. Use `--print-rounds-stats` (or `--print-all` / `-p`) to follow progress round by round.

### Polishing

Polishing (`--exchange-max-remove K`) is based on an [algorithm](https://github.com/Joshua-Stapleton/fmm-helper-exchange) proposed
by [Joshua Stapleton](https://github.com/Joshua-Stapleton). LEO pools the intermediate vectors of all solutions found so far. For a solution, up to `K`
intermediate vectors (never targets) are removed, and at most `min(K - 1, 2)` vectors from the pool are inserted in their place. All other vectors are then
rebuilt in whatever order works, so the dependencies between them may change. If all targets are still reachable, the solution becomes strictly shorter.

The more diverse the solutions in the pool, the more exchanges exist, so polishing works best after several rounds with different strategies. `K = 3` finds more
exchanges than `K = 2`, but its running time grows roughly cubically with the number of intermediate vectors.

Polishing runs after every round, and every solution is polished once. Related options:

- `--exchange-slack S`: also polish solutions that are up to `S` additions worse than the best one (default 1), since a worse solution can sometimes end up
  better after polishing;
- `--exchange-max-time T`: time limit in seconds for polishing one solution (default 10).

`--exchange-max-remove 0` (the default) disables polishing.

### Post-processing

| Option                   | Effect                                                                                                |
|--------------------------|-------------------------------------------------------------------------------------------------------|
| `--inline-substitutions` | inline substitutions that are used exactly once                                                       |
| `--optimize-inversions`  | minimize the number of sign inversions with simulated annealing                                       |
| `--validate`             | recompute the solution and compare it with the input system; `reduce` fails with an error on mismatch |

### Options reference

**General**

| Option      | Short | Default   | Description                                                           |
|-------------|-------|-----------|-----------------------------------------------------------------------|
| `--threads` | `-t`  | all cores | number of OpenMP threads                                              |
| `--seed`    | `-s`  | `0`       | random seed; `0` uses a time-based seed                               |
| `--quiet`   | `-q`  |           | suppress all stdout output (explicit `--print-*` options still print) |

**Input / output**

| Option          | Short | Default      | Description                    |
|-----------------|-------|--------------|--------------------------------|
| `--input-path`  | `-i`  | required     | input file (`.txt` or `.sms`)  |
| `--output-path` | `-o`  | `output.txt` | output file, or `stdout`       |
| `--format`      | `-f`  | `auto`       | `slp`, `txt`, `json` or `auto` |

**Solving strategy**

| Option                   | Short | Default | Description                                                |
|--------------------------|-------|---------|------------------------------------------------------------|
| `--rounds`               | `-r`  | `1`     | number of rounds                                           |
| `--bound-by-best`        | `-b`  |         | stop solvers that cannot beat the best result so far       |
| `--try-transpose`        | `-T`  |         | also solve the transposed system                           |
| `--cse-first-round-only` |       |         | run CSE in the first round only                            |
| `--substitutions-ratio`  |       | `0.5`   | share of tasks starting from substitutions                 |
| `--vectors-ratio`        |       | `0.5`   | share of tasks starting from vectors                       |
| `--min-additions-ratio`  |       | `0.1`   | minimum starting chain length (share of the best solution) |
| `--max-additions-ratio`  |       | `0.5`   | maximum starting chain length (share of the best solution) |
| `--min-remove-vectors`   |       | `1`     | minimum number of removed intermediate vectors             |
| `--max-remove-vectors`   |       | `8`     | maximum number of removed intermediate vectors             |

**Vector covering solver**

| Option                   | Default    | Description                                                                          |
|--------------------------|------------|--------------------------------------------------------------------------------------|
| `--vec-preset`           | `distance` | `default`, `distance` or `all`                                                       |
| `--vec-iterations`       | `10`       | runs per round                                                                       |
| `--vec-sampling`         | `sample`   | `sample`: a random strategy per iteration; `each`: every strategy for all iterations |
| `--vec-add-target-pairs` |            | precompute target vectors reachable with one addition or subtraction (`xi ± xj`)     |
| `--vec-max-time`         | `0`        | time limit of one run in seconds (`0` means unlimited)                               |

**CSE solver**

| Option             | Default  | Description                                      |
|--------------------|----------|--------------------------------------------------|
| `--cse-preset`     | `all`    | `vanilla`, `potential`, `intersections` or `all` |
| `--cse-iterations` | `100`    | runs per round                                   |
| `--cse-sampling`   | `sample` | `sample` or `each` (as above)                    |
| `--cse-max-time`   | `0`      | time limit of one run in seconds                 |

Setting `--vec-iterations 0` or `--cse-iterations 0` disables the corresponding family.

**Polishing**

| Option                  | Default | Description                                                             |
|-------------------------|---------|-------------------------------------------------------------------------|
| `--exchange-max-remove` | `0`     | maximum number of helpers removed per exchange (`0` disables polishing) |
| `--exchange-slack`      | `1`     | polish solutions up to this many additions worse than the best          |
| `--exchange-max-time`   | `10`    | time limit per solution in seconds                                      |

**Post-processing and diagnostics**

| Option                   | Short | Description                            |
|--------------------------|-------|----------------------------------------|
| `--inline-substitutions` |       | inline substitutions used only once    |
| `--optimize-inversions`  |       | optimize sign inversions               |
| `--validate`             |       | validate the resulting solution        |
| `--print-args`           |       | print parsed arguments                 |
| `--print-system-stats`   |       | print statistics of the input system   |
| `--print-rounds-stats`   |       | print the best result after each round |
| `--print-all`            | `-p`  | all of the above                       |

### Practical tips

- **Horizontal systems (fewer expressions than variables): always use `-T`.** The vector covering solver works only for non-horizontal systems; for a horizontal
  one `reduce` runs CSE solvers only. With `-T` the transposed system is vertical, so the vector covering solver can be applied to it, and its solution is then
  transposed back. `-T` is also often useful for other shapes, since the transposed problem is sometimes easier.
- Fix `--seed` for reproducibility, and use `--threads 1` if you need identical runs: with several threads, the order in which solutions arrive may still
  influence later rounds.
- For large systems set `--vec-max-time` and `--cse-max-time` so that a single run cannot take too long.

## Using LEO as a library

Include a single header, `<leo/leo.h>`; everything lives in the `leo` namespace. To build your own program:

```bash
g++ -std=c++17 -fopenmp -Iinclude my_program.cpp build/lib/libleo.a -o my_program
```

### Solver contract

Every solver provides two methods:

```cpp
std::optional<size_t> solve(); // run the solver
Solution getSolution() const;  // retrieve the solution
```

- `solve()` returns the number of additions obtained, or `std::nullopt` if the solver could not finish (for example, it ran out of time or did not satisfy the
  given constraints).
- `getSolution()` returns the substitutions and final expressions found by the **last successful** call of `solve()`.
- Each call of `solve()` starts solving **from scratch**. A solver can therefore be constructed once (so any preparation done in the constructor is paid only
  once), then have its scorer or selector swapped and `solve()` called again.
- `void setBound(size_t bound)` sets an upper bound on the number of additions: the solver stops as soon as it is clear that the result cannot be less than or
  equal to `bound`, and `solve()` then returns `std::nullopt`.

### Example: one solver, several heuristics

```cpp
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include <leo/leo.h>

using namespace leo;

int main() {
    ExpressionsSystem expressionsSystem({
        {1, 1, 1, 1},
        {1, 1, 1, 0},
        {1, 1, 0, 1},
        {1, 0, 1, 1},
        {0, 1, 1, 1}
    });

    uint32_t seed = 42;

    cse::Parameters parameters;
    parameters.maxTimeInSeconds = 0; // unlimited

    // selectors: how to choose a candidate from its scores
    auto greedy = std::make_shared<GreedySelector>();
    auto greedyAlternative = std::make_shared<GreedyAlternativeSelector>();

    // scorers: how to evaluate each candidate
    auto defaultScorer = std::make_shared<cse::DefaultScorer>();
    auto potentialScorer = std::make_shared<cse::PotentialScorer>(0.3);
    auto intersectionsScorer = std::make_shared<cse::IntersectionsScorer>(0.3, 0.7);

    cse::CommonSubexpressionSolver solver(expressionsSystem.getExpressions(), parameters, defaultScorer, greedy, seed);
    std::optional<size_t> additions1 = solver.solve(); // 7

    solver.setScorer(potentialScorer);
    std::optional<size_t> additions2 = solver.solve(); // 7

    solver.setScorer(intersectionsScorer);
    solver.setSelector(greedyAlternative);
    std::optional<size_t> additions3 = solver.solve(); // 7

    return 0;
}
```

All three CSE variants give 7 additions here, while the vector covering solver finds a solution with 6. This is a classic case where CSE cannot reach the
optimum (see `examples/cse_counterexample.txt`).

### Example: vector covering and printing the solution

```cpp
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include <leo/leo.h>

using namespace leo;

int main() {
    ExpressionsSystem expressionsSystem({
        {1, 1, 1, 1},
        {1, 1, 1, 0},
        {1, 1, 0, 1},
        {1, 0, 1, 1},
        {0, 1, 1, 1}
    });

    uint32_t seed = 42;

    vec::Parameters parameters;
    parameters.maxAbsValue = expressionsSystem.getMaxAbsValue();
    parameters.naiveFallback = false;
    parameters.removeUnused = true;
    parameters.ignoreUnusedInBound = false;
    parameters.addTargetPairs = false;
    parameters.maxTimeInSeconds = 0; // unlimited

    auto scorer = std::make_shared<vec::DistanceScorer>();
    auto selector = std::make_shared<GreedyAlternativeSelector>();

    vec::VectorCoveringSolver solver(expressionsSystem.getExpressions(), parameters, scorer, selector, seed);
    std::optional<size_t> additions = solver.solve(); // 6

    if (additions) {
        Solution solution = solver.getSolution();

        bool ok = expressionsSystem.validateSolution(solution); // independent check

        SlpSolutionFormatter formatter; // or PlainTextSolutionFormatter, JsonSolutionFormatter
        formatter.format(std::cout, solution);
    }

    return 0;
}
```

### Example: `Reducer`, strategy pools, rounds and polishing

`Reducer` is the engine behind `reduce`. It runs tasks from a strategy pool in parallel, stores the solutions it finds and restarts solvers from them.

```cpp
cse::Parameters cseParameters;
cseParameters.maxTimeInSeconds = 30;

vec::Parameters vecParameters;
vecParameters.maxAbsValue = expressionsSystem.getMaxAbsValue();
vecParameters.naiveFallback = false; // recommended: a failed or bounded run returns nullopt instead of the naive solution
vecParameters.removeUnused = true;
vecParameters.ignoreUnusedInBound = false;
vecParameters.addTargetPairs = false;
vecParameters.maxTimeInSeconds = 0;

StrategyPool strategies;
strategies.add(presets::vectorCoveringDistance(vecParameters));
strategies.add(presets::cseAll(cseParameters));

ReduceParameters reduceParameters{};
reduceParameters.boundByBest = true;
reduceParameters.substitutionsRatio = 0.25;
reduceParameters.vectorsRatio = 0.7;
reduceParameters.minAdditionsRatio = 0.1;
reduceParameters.maxAdditionsRatio = 0.5;
reduceParameters.minRemoveVectors = 1;
reduceParameters.maxRemoveVectors = 8;

PolishParameters polishParameters{};
polishParameters.maxRemoved = 2;
polishParameters.slack = 1;
polishParameters.maxTimeInSeconds = 10.0;

size_t threads = 8;

std::mt19937 generator(42);

Reducer reducer(threads);
reducer.addGroup(expressionsSystem);

for (size_t round = 0; round < 10; round++) {
    reducer.reduce({strategies.sample(100, generator)}, reduceParameters);
    reducer.polish(polishParameters);

    if (reducer.isOptimal(0))
        break;
}

Solution best = reducer.getSolution(0);
std::cout << reducer.getAdditions(0) << " additions, strategy: " << reducer.getStrategyName(0) << std::endl;
```

Several groups can be added to one `Reducer` with `addGroup`, for example the original and the transposed system, as `reduce -T` does. `reduce` then receives
one `TaskPool` per group, and the solution of the transposed group is brought back with `SolutionTransposer`.

## Architecture

```
include/leo/            public headers (include via <leo/leo.h>)
src/leo/                library implementation
src/tools/reduce.cpp    the command-line tool
src/cli/                command-line argument parsing
src/io/                 input format readers (.txt, .sms)
examples/               sample systems
main.cpp                a small example of using the library
makefile
```

Layers inside `leo`:

| Layer                            | Contents                                                                                                      |
|----------------------------------|---------------------------------------------------------------------------------------------------------------|
| `entities`                       | `ExpressionsSystem`, `Solution`, `Substitution`, `Term`, `Vector` / `TernaryVector`, helper indexes and pools |
| `optimization/solvers`           | the base `Solver`; `cse::CommonSubexpressionSolver` and `vec::VectorCoveringSolver`                           |
| `optimization/solvers/*/scorers` | scorers: heuristics evaluating candidates                                                                     |
| `optimization/selection`         | selectors: rules choosing a candidate from scores                                                             |
| `optimization`                   | `StrategyPool` / `TaskPool`, presets, `Reducer`, `HelperExchange` (polishing)                                 |
| `utils`                          | transposition, substitution inlining, sign optimization, validation                                           |
| `formatters`                     | `txt`, `slp`, `json`                                                                                          |

### Three independent building blocks

```
        Solver                     Scorer                    Selector
 (search algorithm)  +  (evaluates each candidate)  +  (picks one by score)
```

At each step the solver builds a list of candidates, the scorer assigns a score to each, and the selector picks one. This decomposition lets you write a new
heuristic without touching the main algorithm.

### Solvers

**CSE (`cse::CommonSubexpressionSolver`).** At each step it considers pairs of variables `xi ± xj` that occur in more than one expression, scores them, and
substitutes the chosen pair as a new variable. Scorers:

| Scorer                                  | Heuristic                                                                                          |
|-----------------------------------------|----------------------------------------------------------------------------------------------------|
| `cse::DefaultScorer`                    | the classic greedy scoring by number of occurrences                                                |
| `cse::PotentialScorer(alpha)`           | greedy potential from the paper by Erik Mårtensson and Paul Wagner (see [References](#references)) |
| `cse::IntersectionsScorer(alpha, beta)` | greedy intersections from the author's 2025 paper (see [References](#references))                  |

**Vector covering (`vec::VectorCoveringSolver`).** Every expression is a vector. The solver maintains the set of vectors already built and the set of still
uncovered targets, and at each step adds a new vector as a `xi ± xj` combination of two built ones until all targets are covered. Unlike CSE, it can build
intermediate vectors that are not common subexpressions of the original expressions. It is similar in spirit to the Boyer–Peralta algorithm. Scorers:

| Scorer                | Heuristic                                                                                             |
|-----------------------|-------------------------------------------------------------------------------------------------------|
| `vec::DefaultScorer`  | looks one step ahead and uses several similarity measures between vectors (weights are configurable)  |
| `vec::DistanceScorer` | estimates distances to the targets two steps ahead; closest to Boyer–Peralta and generally works best |

The vector covering solver can additionally start from given substitutions (`solve(const std::vector<Substitution>&)`) or given vectors
(`solve(const std::vector<Vector>&)`), which is the basis of multi-round optimization. A given vector is built as soon as it can be obtained with a single
addition from the vectors built so far; vectors that never become reachable are silently skipped, and (with `removeUnused` enabled) vectors that no target
depends on do not appear in the result. Use `canStartFromSubstitutions()` and `canStartFromVectors()` to query this.

### Selectors

| Selector                    | Behavior                                                         |
|-----------------------------|------------------------------------------------------------------|
| `GreedySelector`            | takes the first candidate with the maximum score                 |
| `GreedyAlternativeSelector` | takes a random one among the candidates with the maximum score   |
| `GreedyRandomSelector(p)`   | greedy choice, but with probability `p` takes a random candidate |
| `WeightedRandomSelector`    | picks a candidate at random with probability based on its score  |

### Strategy presets

`presets::*` return ready-made `StrategyPool`s; these are exactly what `reduce` uses.

| Preset                   | Contents                                        | `reduce` option              |
|--------------------------|-------------------------------------------------|------------------------------|
| `vectorCoveringDistance` | `vec/dst1`, `vec/dst2`                          | `--vec-preset distance`      |
| `vectorCoveringDefault`  | `vec/1` ... `vec/9`, `vec/rnd` (random weights) | `--vec-preset default`       |
| `vectorCoveringAll`      | both sets                                       | `--vec-preset all`           |
| `cseVanilla`             | `cse/vanilla`                                   | `--cse-preset vanilla`       |
| `csePotential`           | `cse/potential`                                 | `--cse-preset potential`     |
| `cseIntersections`       | `cse/intersections`                             | `--cse-preset intersections` |
| `cseAll`                 | three CSE strategies with weights 3 : 1 : 1     | `--cse-preset all`           |

The `alpha` / `beta` parameters of the `potential` and `intersections` scorers are drawn randomly for each run in the presets, so the pool explores different
heuristics.

A `StrategyPool` produces tasks in two ways: `sample(n, generator)` gives `n` tasks with random strategies (respecting weights), and `each(n, generator)` gives
`n` tasks for every strategy.

## Extending LEO

LEO is designed so that new algorithms can be added without rewriting existing ones.

### A custom scorer

Inherit from `cse::CommonSubexpressionScorer` or `vec::VectorCoveringScorer` and implement `score`. The scorer receives candidates and a context and writes
scores into `scores`. **The scorer must size `scores` itself** (`scores.resize(...)` or `scores.assign(...)`). A higher score means a better candidate.

```cpp
class RowsCountScorer : public leo::cse::CommonSubexpressionScorer {
public:
    void score(const std::vector<leo::cse::Subexpression>& subexpressions, const leo::cse::Context& context, std::vector<double>& scores) const override {
        scores.resize(subexpressions.size());

        for (size_t k = 0; k < subexpressions.size(); k++)
            scores[k] = subexpressions[k].rows.size();
    }
};
```

It can be used right away:

```cpp
cse::CommonSubexpressionSolver solver(expressions, parameters, std::make_shared<RowsCountScorer>(), selector, seed);
// or: solver.setScorer(std::make_shared<RowsCountScorer>());
```

`score` is `const` because scorers in a pool may be shared between threads running concurrently under `Reducer`; do not keep mutable state in them.

### A custom selector

Inherit from `ScoreSelector` and return the index of the chosen candidate. The base class provides `getMaxScore` and `getMaxScoreIndices`.

```cpp
class MySelector : public leo::ScoreSelector {
public:
    size_t selectIndex(const std::vector<double>& scores, std::mt19937& generator) const override {
        // your selection logic
    }
};
```

### A custom solver

Inherit from `Solver` and implement `solve()` and `getSolution()`. Optionally override `solve(substitutions)` / `solve(seeds)` and `canStartFrom*()` so that
your solver can continue from existing solutions. Respect `bound` (set through `setBound`): when the result is certain to exceed it, return `std::nullopt`.

```cpp
class MySolver : public leo::Solver {
public:
    std::optional<size_t> solve() {
        // your solving logic
    }

    Solution getSolution() const {
        // return your solution
    }
};
```

### Adding to a strategy pool

Any solver becomes a strategy through a factory and then takes part in `Reducer` rounds on equal terms with the built-in ones:

```cpp
StrategyPool strategies;
strategies.add(presets::vectorCoveringDistance(vecParameters));

strategies.add("cse/mine", /* weight */ 1.0,
    [=](const std::vector<std::vector<int>>& expressions, uint32_t seed) {
        return std::make_unique<cse::CommonSubexpressionSolver>(
            expressions, cseParameters,
            std::make_shared<RowsCountScorer>(),
            std::make_shared<MySelector>(),
            seed);
    });
```

### Other extension points

- **Output format**: a subclass of `SolutionFormatter` with the method `format(std::ostream&, const Solution&)`.
- **Input format**: a subclass of `ExpressionsReader` (`src/io/expressions_reader.h`).

## Contributing

Contributions are welcome: new solvers, scorers, selectors, input and output formats, example systems and results on them, documentation improvements.

1. Fork the repository and create a branch for your change.
2. Follow the existing style: C++17, headers in `include/leo/...`, implementation in `src/leo/...` with a mirrored directory structure.
3. Add new `.cpp` files to `LIB_SOURCES` in the `makefile` and new public headers to `include/leo/leo.h`.
4. Check results with `--validate`: every solution found must pass the independent check.
5. If you propose a new heuristic, include a comparison with existing strategies on several systems (number of additions and running time).
6. Open a pull request describing the idea and the results.
   Use issues for questions and discussions.

## References

- Erik Mårtensson, Paul Stankovski Wagner. *The Number of the Beast: Reducing Additions in Fast Matrix Multiplication Algorithms for Dimensions up to
  666.* https://eprint.iacr.org/2024/2063.pdf (Greedy Potential, `cse::PotentialScorer`).
- Andrey Perminov. *Parallel Heuristic Exploration for Additive Complexity Reduction in Fast Matrix Multiplication.* https://arxiv.org/abs/2512.13365 (greedy
  intersections, `cse::IntersectionsScorer`).
- Joshua Stapleton, *fmm-helper-exchange.* https://github.com/Joshua-Stapleton/fmm-helper-exchange (idea of the solution polishing algorithm).

## License

LEO is released under the MIT License. See the [LICENSE](LICENSE) file for details.
