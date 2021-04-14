#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "omp.h"
#include "graph/Graph.h"

#define LINE_SIZE 100
#define TRANSMISSION 0.5 /* Transmission rate is 50% */
#define MORTALITY 0.0034 /* Mortality rate is 0.34% */
#define DURATION 10      /* Virus infection duration is 10 days */

Graph *g; /* The graph is declared globally, as every function uses it */

void epidemic(long cases, int threads, int days, FILE *stream);
void readFile(char *filename);
long readSeedFile(char *filename);
int isGoingToDie(int day, unsigned short *seed);
int isGoingToContaminate(unsigned short *seed);

int main(int argc, char **argv)
{
    long zeroPatients = 0;
    int opt, printGraph = 0, calcTime = 0, threads = 1, days = 30, useOMP = 0;
    char *dataset = NULL, *seed = NULL, *outFilename = "simulation.csv", *timeMetrics = "timeCalculations.txt";
    struct timespec start, finish;
    double elapsed;
    FILE *outstream = stdout, *timeStream;

    while ((opt = getopt(argc, argv, "f:s:d:t:cph")) != -1)
    {
        switch (opt)
        {
        case 'f': /*filename*/
            dataset = strdup(optarg);
            readFile(dataset);
            free(dataset);
            break;
        case 's': /*seed*/
            seed = strdup(optarg);
            zeroPatients = readSeedFile(seed);
            free(seed);
            break;
        case 'd': /*days*/
            days = atoi(optarg);
            break;
        case 't': /*threads*/
            threads = atoi(optarg);
            break;
        case 'c': /*count time*/
            calcTime = 1;
            break;
        case 'p': /*print (graph)*/
            printGraph = 1;
            break;
        case 'h': /*help*/
            printf(
                "Usage: ./epidemic -f dataset -s seed -t threads [-m]\n"
                "Options:\n"
                "   -f <string>         Specifies the filename of the dataset.\n"
                "   -s <string>         Specifies the filename of the seed.\n"
                "   -t <int>            Determines how many threads the algorithm will use. Must be in range of [1,4].\n"
                "   -d <int>            Determines how many days will be simulated.\n"
                "   -c                  When it is used, the running time calculation is displayed.\n"
                "   -p                  When this is set, the graph structure is printed.\n"
                "   -h                  Prints this help\n");
            return 0;
        default:
            printf("Inavalid flags. Use [-h] for help\n");
            return 0;
        }
    }

    if (seed == NULL || dataset == NULL)
    {
        printf("No input dataset/seed provided.\nRun using [-h] for help\n");
        return 0;
    }

    if (!(outstream = fopen(outFilename, "w+")))
    {
        perror("Could not open the file");
        exit(EXIT_FAILURE);
    }

    if (!(timeStream = fopen(timeMetrics, "a")))
    {
        perror("Could not open the file");
        exit(EXIT_FAILURE);
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    epidemic(zeroPatients, threads, days, outstream);
    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    fprintf(timeStream, "%f\n", elapsed);

    if (printGraph)
        Graph_print(g);

    if (calcTime)
        printf("Calculation Time: %fs\n", elapsed);

    fclose(outstream);
    fclose(timeStream);

    Graph_free(g);
    g = NULL;

    return 0;
}

void readFile(char *filename)
{
    FILE *stream;
    char line[LINE_SIZE], *tok;
    long from, to, size = 0, edges = 0, counter = 0;

    if (!(stream = fopen(filename, "r+")))
    {
        perror("Could not open the file");
        exit(EXIT_FAILURE);
    }

    while ((fgets(line, LINE_SIZE, stream)) != NULL)
    {
        if (line[0] == '#')
        {
            continue;
        }
        edges++;
    }

    printf("Total edges: %ld\n", edges);

    rewind(stream);

    g = Graph_init(2 * edges);

    while ((fgets(line, LINE_SIZE, stream)) != NULL)
    {
        if (line[0] == '#')
        {
            continue;
        }

        tok = strtok(line, "\r \t\v");
        from = atol(tok);

        tok = strtok(NULL, "\r \t\v");
        to = atol(tok);

        Graph_addConnection(g, from, to);
    }

    Graph_freeUnused(g);

    printf("Total nodes: %ld\n", g->currSize);

    fclose(stream);
}

long readSeedFile(char *filename)
{
    FILE *stream;
    char line[LINE_SIZE], *tok;
    long patientID, cases = 0;

    if (!(stream = fopen(filename, "r+")))
    {
        perror("Could not open the file");
        exit(EXIT_FAILURE);
    }

    while ((fgets(line, LINE_SIZE, stream)) != NULL)
    {
        if (line[0] == '#')
        {
            continue;
        }

        tok = strtok(line, "\r \t\v");
        patientID = atol(tok);
        cases++;
        Graph_setContaminated(g, patientID);
    }

    fclose(stream);
    return cases;
}

void epidemic(long cases, int threads, int days, FILE *stream)
{
    long i, j, newCases = cases, totalCases = cases, recovered = 0, active = cases, newDeaths = 0, totalDeaths = 0;
    Node *nodes;
    Connection *conn;
    unsigned short seed[3] = {1, 5, 9}, case_found = 0;

    nodes = g->nodes;
    fprintf(stream, "Day,NewCases,TotalCases,Recovered,Active,NewDeaths,TotalDeaths\n");

    for (i = 0; i < days; i++)
    {
        /* Start of parallel region: g, seed and nodes are shared among the threads */
#pragma omp parallel num_threads(threads)
        //shared(g, nodes, seed)
        {
            /* As rand() is not thread-safe, the random number generator uses erand48(seed) with custom seed */
#ifdef _OPENMP
            seed[0] = omp_get_thread_num();
            seed[1] = omp_get_num_threads();
            seed[2] = 5;
#endif
            /* Phase 1*/
#pragma omp for firstprivate(seed, case_found) private(j, conn) schedule(static) reduction(+ \
                                                                                           : active)
            for (j = 0; j < g->currSize; j++)
            {
                //printf("case found = %d\n", case_found);

                /* For every no-yet contaminated, alive node, we check if he will get contaminated during the day */
                if (!nodes[j].isDead && !nodes[j].hasAnosia && !nodes[j].isContaminated)
                {
                    /* Iterating the connection list to find out if any neighbor is contaminated */
                    conn = nodes[j].connectionsHead;
                    while (conn)
                    {
                        if (!nodes[conn->indexTo].isDead && nodes[conn->indexTo].isContaminated && isGoingToContaminate(seed))
                        {
                            /* In this case, node[j] gets contaminated */
                            //printf("Node %ld contaminates node %ld\n", nodes[conn->indexTo].id, nodes[j].id);
                            case_found = 1;         /* Flag to add up active cases */
                            conn->contaminates = 1; /* This memory address is written only by the current thread, no data race */
                        }
                        conn = conn->next;
                    }
                    //printf("case found = %d\n", case_found);
                    /* If node[j] gets contaminated, the active cases are increased */
                    if (case_found)
                    {
                        //printf("Node %ld got contaminated, active++\n", nodes[j].id);
                        active++;
                        case_found = 0;
                    }
                }
            }
            /* Implicit (required) barrier between the two phases */

            /* As rand() is not thread-safe, the random number generator uses erand48(seed) with new custom seed */
#ifdef _OPENMP
            seed[0] = omp_get_num_threads();
            seed[1] = omp_get_thread_num();
            seed[2] = 3;
#endif

            /* Phase 2 */
#pragma omp for schedule(static) private(j, conn)                        \
    reduction(+                                                          \
              : totalDeaths, newDeaths, recovered, newCases, totalCases) \
        reduction(-                                                      \
                  : active)
            for (j = 0; j < g->currSize; j++)
            {
                /* We dont care for immune or dead nodes */
                if (nodes[j].hasAnosia || nodes[j].isDead)
                {
                    continue;
                }

                /* Actions for contaminated nodes */
                if (nodes[j].isContaminated)
                {
                    /* If the current node is going to die, the corresponding counters are updated */
                    if (isGoingToDie(nodes[j].daysRecovering, seed))
                    {
                        nodes[j].isDead = 1;
                        totalDeaths++;
                        newDeaths++;
                        active--;
                        //printf("Node %ld died!\n", nodes[j].id);
                    }
                    else
                    {
                        /* If the current node survives this day, the counter is incremented */
                        nodes[j].daysRecovering++;
                        /* In case the virus duration has passed, the current node survives and becomes immune */
                        if (nodes[j].daysRecovering == DURATION)
                        {
                            nodes[j].isContaminated = 0;
                            nodes[j].hasAnosia = 1; /* Immune nodes cannot get ill again */
                            recovered++;
                            active--;
                            //printf("Node %ld got well!\n", nodes[j].id);
                        }
                    }
                }
                /* Actions for not contaminated nodes */
                else
                {
                    /* Traversing the neighbour list to find out if the current node got contamited during phase 1 */
                    conn = nodes[j].connectionsHead;
                    while (conn)
                    {
                        /* Reading the data written in phase 1 */
                        if (conn->contaminates == 1)
                        {

                            nodes[j].isContaminated = 1; /* Contamination Case */
                        }

                        conn->contaminates = 0; /* Reseting the flag for the next days */
                        conn = conn->next;
                    }

                    /* If the node got contaminted, increase the counters */
                    if (nodes[j].isContaminated)
                    {
                        //printf("As node %ld gets contaminated, newCases++ and totalCases++\n", nodes[j].id);
                        newCases++;
                        totalCases++;
                    }
                }
            }
            /* Implicit (required) barrier */
        }
        /* Implicit (required) barrier */

        /* Update the output file and reset the counters: This results in some overhead addition but its inevitable */
        fprintf(stream, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", i + 1, newCases, totalCases, recovered, active, newDeaths, totalDeaths);
        newCases = 0;
        newDeaths = 0;

        /* At this point, current day calculations are done */
    }
}

int isGoingToContaminate(unsigned short *seed)
{
    /* 
        Generating propability: To simulate the coin flip propabilistic model using c and erand() the tactic is:
        Generate a random double number between [0 and 1]
        If the number < propability, the node contaminates
    */
    double num;

    num = erand48(seed);
    //printf("isGoingToTransmit: Generated:    %f\n", num);

    /* Returning true means that the node will contaminate the neighbour*/
    return num < TRANSMISSION;
}

int isGoingToDie(int day, unsigned short *seed)
{
    /* 
        Generating propability: To simulate the coin flip propabilistic model using c and erand() the tactic is:
        Generate a random double number between [0 and 1]
        If the number < propability, the node contaminates
    */
    double num;
    num = erand48(seed);
    //printf("isGoingToDIe: Generated: %f < %f\n", num, (1 - pow(1 - (MORTALITY), day)));

    /*
        Crucial point: For an epidemic of DURATIONS days and MORTALITY death rate, 
        the propability for an infected node to die during the day x where x in [0, DURATION]
        is 1 - (1 - M)^x.
        eg. Day 0 => p = 0 (he is not going to die)
        eg. Day 1 => p = M (MORTALITY rate)
        eg. Day 2 => p = 1 - (1 - M)^2 => p gets bigger over days
    */
    return num < (1 - pow(1 - MORTALITY, day));
}