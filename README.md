# omp-epidemic-simulation
Epidemic spread simulation using OpenMP

## Compilation
To compile:
```
cd build
make
```
To compile the sequential program:
```
cd build
make nomp
```
To run the tests (Python 3 required):
```
cd build
make tests
```

## Configure and Run
To configure and run use:
```
./epidemic  -f [<string> inputfile]
            -s [<string> seedfile]
            -d [<int> days]
            -t [<int> threads]
            -c Set this to calculate running time 
            -p Set this to print the graph

eg: ./epidemic -f ../datasets/facebook.txt -s ../datasets/fbSeed.txt -d 180 -t 2 -c          
```

## Output
The executable produces two output files: "simulation.csv" (the results of the epidemic) and "timeCalculations.txt" (the time measurements used by the test script)


