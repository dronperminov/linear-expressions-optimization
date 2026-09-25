CXX = g++
FLAGS = -Wall -pedantic -O3 -std=c++17 -fopenmp -Iinclude

ENTITIES = src/entities/vector.o \
           src/entities/vector_index.o \
           src/entities/solution.o \
           src/entities/expressions_system.o

SELECTORS = src/optimization/selection/score_selector.o \
            src/optimization/selection/greedy_selector.o \
            src/optimization/selection/greedy_alternative_selector.o \
            src/optimization/selection/greedy_random_selector.o

SOLVERS = src/optimization/solvers/solver.o \
          src/optimization/solvers/vector_covering/scorers/default_scorer.o \
          src/optimization/solvers/vector_covering/vector_covering_solver.o \
          src/optimization/solvers/cse/scorers/default_scorer.o \
          src/optimization/solvers/cse/scorers/potential_scorer.o \
          src/optimization/solvers/cse/common_subexpression_solver.o

UTILS = src/utils/solution_validator.o \
        src/utils/solution_transposer.o \
        src/utils/formatters/slp_formatter.o

OBJECTS = $(ENTITIES) $(SELECTORS) $(SOLVERS) $(UTILS)

all: main

main: $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) main.cpp -o main

%.o: %.cpp
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS)
