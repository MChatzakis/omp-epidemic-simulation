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
#define MORTALITY 0.34    /* Mortality rate is 34% */
#define DURATION 10      /* Virus infection duration is 10 days */

Graph *g; /* The graph is declared globally, as every function uses it */

void readFile(char *filename);
void writeCSVFIle(char *filename);
void epidemic(long cases, int threads, int days, FILE *stream);
int isGoingToDie(int day, unsigned short *seed);
int isGoingToContaminate(unsigned short *seed);
long readSeedFile(char *filename);

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

    //srand(getpid());
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

    for (int k = 0; k < 10; k++)
    {
        //isGoingToDie(k);
        //isGoingToContaminate();
    }

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    if (printGraph)
        Graph_print(g);

    if (calcTime)
    {
        printf("Calculation Time: %fs\n", elapsed);
        fprintf(timeStream, "%f\n", elapsed);
    }

    fclose(outstream);
    fclose(timeStream);

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

void writeCSVFIle(char *filename)
{
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
#pragma omp parallel num_threads(threads)
        {
#ifdef _OPENMP
            seed[0] = omp_get_thread_num();
            seed[1] = omp_get_num_threads();
            seed[2] = 5;
#endif

#pragma omp for firstprivate(seed) private(j, conn, case_found) schedule(static) reduction(+ \
                                                                                           : active)
            for (j = 0; j < g->currSize; j++)
            {
                //printf("Threads: %d\n", omp_get_num_threads());
                //printf("j: %ld\n", j);
                if (!nodes[j].isDead && !nodes[j].hasAnosia && !nodes[j].isContaminated)
                {
                    conn = nodes[j].connectionsHead;
                    while (conn)
                    {
                        if (!nodes[conn->indexTo].isDead && nodes[conn->indexTo].isContaminated && isGoingToContaminate(seed))
                        {
                            //printf("Node %ld contaminates node %ld\n", nodes[conn->indexTo].id, nodes[j].id);
                            case_found = 1;
                            conn->contaminates = 1;
                        }
                        conn = conn->next;
                    }

                    if (case_found) //check this shit again..
                    {
                        active++;
                        case_found = 0;
                    }
                }
            }
        }

#pragma omp parallel num_threads(threads)
        {

#ifdef _OPENMP
            seed[0] = omp_get_thread_num();
            seed[1] = omp_get_num_threads();
            seed[2] = 5;
#endif

#pragma omp for schedule(static) private(j, conn)                        \
    reduction(+                                                          \
              : totalDeaths, newDeaths, recovered, newCases, totalCases) \
        reduction(-                                                      \
                  : active)
            for (j = 0; j < g->currSize; j++)
            {
                if (nodes[j].hasAnosia || nodes[j].isDead)
                {
                    continue;
                }

                if (nodes[j].isContaminated)
                {
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
                        nodes[j].daysRecovering++;
                        if (nodes[j].daysRecovering == DURATION)
                        {
                            nodes[j].isContaminated = 0;
                            nodes[j].hasAnosia = 1;
                            recovered++;
                            active--;
                            //printf("Node %ld got well!\n", nodes[j].id);
                        }
                    }
                }
                else
                {
                    //find if it got contaminated today
                    conn = nodes[j].connectionsHead;
                    while (conn)
                    {
                        if (conn->contaminates == 1)
                        {
                            nodes[j].isContaminated = 1;
                        }

                        conn->contaminates = 0;
                        conn = conn->next;
                    }

                    if (nodes[j].isContaminated)
                    {
                        newCases++;
                        totalCases++;
                        //active++;
                    }
                }
            }
        }

        fprintf(stream, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", i + 1, newCases, totalCases, recovered, active, newDeaths, totalDeaths);
        newCases = 0;
        newDeaths = 0;
    }

    //fprintf(stream, "Day,NewCases,TotalCases,Recovered,Active,NewDeaths,TotalDeaths\n");
}

int isGoingToContaminate(unsigned short *seed)
{
    double num;

    num = erand48(seed);
    //printf("isGoingToTransmit: Generated:    %f\n", num);

    return num < TRANSMISSION;
}

int isGoingToDie(int day, unsigned short *seed)
{
    double num = 0;
    num = erand48(seed);
    //printf("isGoingToDIe: Generated: %f < %f\n", num, (1 - pow(1 - (MORTALITY), day)));

    return num < (1 - pow(1 - MORTALITY, day));
}