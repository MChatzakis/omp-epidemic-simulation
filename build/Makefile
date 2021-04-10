GRAPH = ../source/Graph.h
EPID = ../source/Epidemic.c

all: gcc

nomp: gccn

gccn: $(EPID) $(GRAPH)
	gcc $(EPID) -lm -o epidemic 

gcc: $(EPID) $(GRAPH)
	gcc $(EPID) -lm -fopenmp -o epidemic 

clean:	
	rm -f epidemic
	rm -f simulation.csv
	rm -f timeCalculations.txt