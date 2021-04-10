#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "omp.h"
#include "Graph.h"

#define LINE_SIZE 100

double TRANSMISSION = 50; /* Transmission rate is 50% */
double MORTALITY = 34;    /* Mortality rate is 34% */

int DURATION = 10; /* Virus infection duration is 10 days */
int DAYS = 30;    /* Total days for simulation */

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
    
    srand(getpid());

    clock_gettime(CLOCK_MONOTONIC, &start);
    //epidemic(zeroPatients);
    clock_gettime(CLOCK_MONOTONIC, &finish);


    for (int k = 0; k < 10; k++)
    {
        isGoingToDie(k);
        //isGoingToContaminate();
    }

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
        //phase1
        for (j = 0; j < g->currSize; j++)
        {
            if (!nodes[j].isDead && !nodes[j].hasAnosia && !nodes[j].isContaminated)
            {
                conn = nodes[j].connectionsHead;
                while (conn)
                {
                    if (!nodes[conn->indexTo].isDead && nodes[conn->indexTo].isContaminated && isGoingToContaminate())
                    {
                        printf("Node %ld contaminates node %ld\n", nodes[conn->indexTo].id, nodes[j].id);
                        conn->contaminates = 1;
                    }

                    conn = conn->next;
                }
            }
        }

        //phase2
        for (j = 0; j < g->currSize; j++)
        {
            if (nodes[j].hasAnosia || nodes[j].isDead)
            {
                continue;
            }

            if (nodes[j].isContaminated)
            {
                if (isGoingToDie(nodes[j].daysRecovering))
                {
                    nodes[j].isDead = 1;
                    totalDeaths++;
                    newDeaths++;
                    active--;
                    printf("Node %ld died!\n", nodes[j].id);
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
                        printf("Node %ld got well!\n", nodes[j].id);
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
                    active++;
                }
            }
        }

        fprintf(stdout, "%ld,%ld,%ld,%ld,%ld,%ld,%ld\n", i + 1, newCases, totalCases, recovered, active, newDeaths, totalDeaths);
        newCases = 0;
        newDeaths = 0;
    }

    fprintf(stdout, "Day,NewCases,TotalCases,Recovered,Active,NewDeaths,TotalDeaths\n");
}

int isGoingToContaminate()
{
    int min = 0, max = 100;
    double num;

    num = (rand() % (max - min + 1)) + min;

    printf("isGoingToTransmit: Generated:    %f\n", num);

    return num < TRANSMISSION;
}

int isGoingToDie(int day)
{
    double min = 0.0, max = 1.0;
    double num;

    //num = (rand() % (max - min + 1)) + min;
    printf("isGoingToDIe: Generated: %f < %f\n", num, (1 - pow(1 - (MORTALITY), day)));

    return num < (1 - pow(1 - MORTALITY, day));
}