CXX = g++
FLAGS = -Wall -pedantic -O3 -std=c++17 -fopenmp
ENTITIES = src/entities/vector.o
OPTIMIZERS = src/optimization/optimizers/vector_covering_optimizer.o

OBJECTS = $(ENTITIES) $(OPTIMIZERS)

all: main

main: $(OBJECTS)
	$(CXX) $(FLAGS) $(OBJECTS) main.cpp -o main

%.o: %.cpp
	$(CXX) $(FLAGS) -c $< -o $@

clean:
	rm -rf $(OBJECTS)
