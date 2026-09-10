CXX = g++
FLAGS = -Wall -pedantic -O3 -std=c++17 -fopenmp

ENTITIES = src/entities/vector.o

SELECTORS = src/optimization/selection/score_selector.o \
            src/optimization/selection/greedy_selector.o \
            src/optimization/selection/greedy_alternative_selector.o \
            src/optimization/selection/greedy_random_selector.o

SOLVERS = src/optimization/solvers/abstract_solver.o \
          src/optimization/solvers/vector_covering/scorers/default_scorer.o \
          src/optimization/solvers/vector_covering/vector_covering_solver.o

VALIDATORS = src/validation/solution_validator.o

OBJECTS = $(ENTITIES) $(SELECTORS) $(VALIDATORS) $(SOLVERS)

all: main

main: $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) main.cpp -o main

%.o: %.cpp
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS)
