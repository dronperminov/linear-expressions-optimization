CXX := g++
AR  := ar

CPPFLAGS := -Iinclude -MMD -MP
CXXFLAGS := -Wall -pedantic -O3 -std=c++17 -fopenmp
LDFLAGS  := -fopenmp

BUILD_DIR := build
LIB_DIR   := $(BUILD_DIR)/lib
LIB_PATH  := $(LIB_DIR)/libleo.a

LIB_SOURCES := \
    src/leo/entities/expressions_system.cpp \
    src/leo/entities/hash_set.cpp \
    src/leo/entities/helper_pool.cpp \
    src/leo/entities/solution.cpp \
    src/leo/entities/substitution.cpp \
    src/leo/entities/vector.cpp \
    src/leo/entities/vector_index.cpp \
    src/leo/formatters/json_solution_formatter.cpp \
    src/leo/formatters/plain_text_solution_formatter.cpp \
    src/leo/formatters/slp_solution_formatter.cpp \
    src/leo/optimization/group.cpp \
    src/leo/optimization/helper_exchange.cpp \
    src/leo/optimization/reducer.cpp \
    src/leo/optimization/selection/greedy_alternative_selector.cpp \
    src/leo/optimization/selection/greedy_random_selector.cpp \
    src/leo/optimization/selection/greedy_selector.cpp \
    src/leo/optimization/selection/score_selector.cpp \
    src/leo/optimization/selection/weighted_random_selector.cpp \
    src/leo/optimization/solvers/cse/common_subexpression_solver.cpp \
    src/leo/optimization/solvers/cse/scorers/default_scorer.cpp \
    src/leo/optimization/solvers/cse/scorers/intersections_scorer.cpp \
    src/leo/optimization/solvers/cse/scorers/potential_scorer.cpp \
    src/leo/optimization/solvers/solver.cpp \
    src/leo/optimization/solvers/vector_covering/scorers/default_scorer.cpp \
    src/leo/optimization/solvers/vector_covering/scorers/distance_scorer.cpp \
    src/leo/optimization/solvers/vector_covering/vector_covering_solver.cpp \
    src/leo/optimization/strategy_pool.cpp \
    src/leo/optimization/strategy_presets.cpp \
    src/leo/utils/solution_sign_optimizer.cpp \
    src/leo/utils/solution_substitution_inliner.cpp \
    src/leo/utils/solution_transposer.cpp \
    src/leo/utils/solution_validator.cpp

APP_SOURCES := \
    src/cli/arg_parser.cpp \
    src/cli/argument.cpp \
    src/io/expressions_readers/sms_expressions_reader.cpp \
    src/io/expressions_readers/txt_expressions_reader.cpp \
    src/utils.cpp

LIB_OBJECTS := $(LIB_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
APP_OBJECTS := $(APP_SOURCES:%.cpp=$(BUILD_DIR)/%.o)
MAIN_OBJECT := $(BUILD_DIR)/main.o
REDUCE_OBJECT := $(BUILD_DIR)/src/tools/reduce.o

DEPS := $(LIB_OBJECTS:.o=.d) $(APP_OBJECTS:.o=.d) \
        $(MAIN_OBJECT:.o=.d) $(REDUCE_OBJECT:.o=.d)

.PHONY: all library clean

all: library main reduce

library: $(LIB_PATH)

$(LIB_PATH): $(LIB_OBJECTS) | $(LIB_DIR)
	$(AR) rcs $@ $^

main: $(MAIN_OBJECT) $(APP_OBJECTS) $(LIB_PATH)
	$(CXX) $(LDFLAGS) $^ -o $@

reduce: $(REDUCE_OBJECT) $(APP_OBJECTS) $(LIB_PATH)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(LIB_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) main reduce

-include $(DEPS)
