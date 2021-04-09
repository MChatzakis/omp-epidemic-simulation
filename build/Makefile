GRAPH = ../source/Graph.h
EPID = ../source/Epidemic.c

gcc: $(EPID) $(GRAPH)
	gcc $(EPID) -lm -fopenmp -o epidemic 

clean:	
	rm -f epidemic