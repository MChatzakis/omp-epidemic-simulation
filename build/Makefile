GRAPH = ../source/graph/Graph.h
GRAPH_C = ../source/graph/Graph.c
EPID = ../source/Epidemic.c

all: gcc

nomp: gccn

gccn: $(EPID) $(GRAPH) $(GRAPH_C)
	gcc $(GRAPH_C) $(EPID) -lm -o epidemic 

gcc: $(EPID) $(GRAPH) $(GRAPH_C)
	gcc $(GRAPH_C) $(EPID) -lm -fopenmp -o epidemic 

tests: script.py
	clear
	python3 script.py

clean:	
	rm -f epidemic
	rm -f simulation.csv
	rm -f timeCalculations.txt