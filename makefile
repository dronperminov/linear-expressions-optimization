CXX = g++
FLAGS = -Wall -pedantic -O3 -std=c++17 -fopenmp -Iinclude

LEO_ENTITIES = src/leo/entities/vector.o \
           src/leo/entities/vector_index.o \
           src/leo/entities/solution.o \
           src/leo/entities/expressions_system.o

LEO_SELECTORS = src/leo/optimization/selection/score_selector.o \
            src/leo/optimization/selection/greedy_selector.o \
            src/leo/optimization/selection/greedy_alternative_selector.o \
            src/leo/optimization/selection/greedy_random_selector.o

LEO_SOLVERS = src/leo/optimization/solvers/solver.o \
          src/leo/optimization/solvers/vector_covering/scorers/default_scorer.o \
          src/leo/optimization/solvers/vector_covering/vector_covering_solver.o \
          src/leo/optimization/solvers/cse/scorers/default_scorer.o \
          src/leo/optimization/solvers/cse/scorers/potential_scorer.o \
          src/leo/optimization/solvers/cse/scorers/intersections_scorer.o \
          src/leo/optimization/solvers/cse/common_subexpression_solver.o

LEO_OPTIMIZATION = src/leo/optimization/strategy_pool.o \
                   src/leo/optimization/reducer.o

LEO_FORMATTERS = src/leo/formatters/json_solution_formatter.o \
             src/leo/formatters/plain_text_solution_formatter.o \
             src/leo/formatters/slp_solution_formatter.o

LEO_UTILS = src/leo/utils/solution_validator.o \
        src/leo/utils/solution_transposer.o


UTILS = src/utils.o

CLI = src/cli/argument.o src/cli/arg_parser.o

IO = src/io/expressions_readers/txt_expressions_reader.o \
     src/io/expressions_readers/sms_expressions_reader.o

LEO_OBJECTS = $(LEO_ENTITIES) $(LEO_SELECTORS) $(LEO_SOLVERS) $(LEO_OPTIMIZATION) $(LEO_FORMATTERS) $(LEO_UTILS)
OBJECTS = $(LEO_OBJECTS) $(UTILS) $(CLI) $(IO)

all: main reduce

reduce: $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) reduce.cpp -o reduce

main: $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) main.cpp -o main

%.o: %.cpp
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS)
