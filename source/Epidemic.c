#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "omp.h"
#include "Graph.h"

#define LINE_SIZE 100

int TRANSMISSION = 50; /* Transmission rate is 50% */
int MORTALITY = 34;    /* Mortality rate is 34% */
int DURATION = 10;     /* Virus infection duration is 10 days */
int DAYS = 3;          /* Total days for simulation */

Graph *g; /* The graph is declared globally, as every function uses it */

void readFile(char *filename);
long readSeedFile(char *filename);
void writeCSVFIle(char *filename);

void epidemic(long cases);

int isGoingToDie(int day);
int isGoingToContaminate();

int main(int argc, char **argv)
{
    long zeroPatients = 0;
    int opt, printGraph = 0, calcTime = 0;
    char *dataset = NULL, *seed = NULL;
    struct timespec start, finish;
    double elapsed;
    FILE *outstream = stdout;

    while ((opt = getopt(argc, argv, "f:s:d:o:tph")) != -1)
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
            DAYS = atoi(optarg);
            break;
        case 'o': /*outstream*/

            break;
        case 't': /*time*/
            calcTime = 1;
            break;
        case 'p': /*print (graph)*/
            printGraph = 1;
            break;
        case 'h': /*help*/
            printf(
                "Usage: ./page_rank -f file -t threads [-m]\n"
                "Options:\n"
                "   -f <string>         Specifies the filename of the dataset.\n"
                "   -t <int>            Determines how many threads the algorithm will use. Must be in range of [1,4].\n"
                "   -m                  When it is used, displays the time metrics about pagerank.\n"
                "   -g                  When -g is set, the graph structure is printed to stdout.\n"
                "   -h                  Prints this help\n");
            return 0;
        default:
            printf("Use [-h] for help\n");
            return 0;
        }
    }

    if (seed == NULL || dataset == NULL)
    {
        printf("No input dataset/seed provided.\nRun using [-h] for help\n");
        return 0;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    epidemic(zeroPatients);
    clock_gettime(CLOCK_MONOTONIC, &finish);

    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    if (printGraph)
        Graph_print(g);

    if (calcTime)
        printf("Time: %fs\n", elapsed);

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

void epidemic(long cases)
{
    long i, j, newCases = 0, totalCases = cases, recovered = 0, active = cases, newDeaths = 0, totalDeaths = 0;
    Node *nodes;
    Connection *conn;

    nodes = g->nodes;

    fprintf(stdout, "Day,NewCases,TotalCases,Recovered,Active,NewDeaths,TotalDeaths\n");

    for (i = 0; i < DAYS; i++)
    {
        //phase 1
        for (j = 0; j < g->currSize; j++)
        {
            if (!nodes[j].isDead && nodes[j].isContaminated)
            {
                //printf("Contaminated, phase 1\n");
                conn = nodes[j].connectionsHead;
                while (conn)
                {
                    if (isGoingToContaminate() &&
                        !nodes[conn->indexTo].isDead &&
                        !nodes[conn->indexTo].isContaminated &&
                        !nodes[conn->indexTo].hasAnosia)
                    {
                        printf("Node %ld got ill from node %ld!\n", nodes[conn->indexTo].id, nodes[j].id);
                        nodes[conn->indexTo].isContaminated = 1; //crazy data race <3
                        newCases++;
                        totalCases++;
                        active++;
                    }
                    conn = conn->next;
                }

                if (isGoingToDie(nodes[j].daysRecovering))
                {
                    printf("Node %ld died!\n", nodes[j].id);
                    nodes[j].isDead = 1;
                    nodes[j].daysRecovering = 0;
                    newDeaths++;
                    totalDeaths++;
                }
            }
        }

        //phase 2
        for (j = 0; j < g->currSize; j++)
        {
            if (!nodes[j].isDead)
            {
                if (nodes[j].isContaminated)
                {
                    nodes[j].daysRecovering++;
                    if (nodes[j].daysRecovering == DURATION)
                    {
                        printf("Case got anosia!\n");
                        nodes[j].hasAnosia = 1;
                        nodes[j].isContaminated = 0;
                        recovered++;
                        active--;
                    }
                }
            }
        }

        fprintf(stdout, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", i + 1, newCases, totalCases, recovered, active, newDeaths, totalDeaths);
        newCases = 0;
        newDeaths = 0;
    }
}

int isGoingToContaminate()
{
    int min = 0, max = 100, num;

    srand(time(0));
    num = (rand() % (max - min + 1)) + min;
    //printf("isGoingToTransmit: Generated: %d\n", num);

    return num < TRANSMISSION;
}

int isGoingToDie(int day)
{
    int min = 0, max = 100, num;

    srand(time(0));
    num = (rand() % (max - min + 1)) + min;
    //printf("isGoingToDIe: Generated: %d\n", num);

    return num < (1 - pow(1 - MORTALITY, day));
}