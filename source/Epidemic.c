#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "Graph.h"

#define LINE_SIZE 100

int TRANSMISSION = 50; /* Transmission rate is 50% */
int MORTALITY = 34;    /* Mortality rate is 34% */
int DURATION = 10;
int DAYS = 200;

Graph *g;

void readFile(char *filename);
void readSeedFile(char *filename);
void writeCSVFIle(char *filename);
void epidemic();

int isGoingToTransmit()
{
    int min = 0, max = 100, num;

    srand(time(0));
    num = (rand() % (max - min + 1)) + min;
    printf("isGoingToTransmit: Generated: %d\n", num);

    return num < TRANSMISSION;
}

int isGoingToDie(int day)
{
    int min = 0, max = 100, num;

    srand(time(0));
    num = (rand() % (max - min + 1)) + min;
    printf("isGoingToDIe: Generated: %d\n", num);

    return num < (1 - pow(1 - MORTALITY, day));
}

int main()
{

    //printf("Tr: %d\n", isGoingToTransmit());

    readFile("../datasets/test1");

    Graph_print(g);

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

void readSeedFile(char *filename)
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

        tok = strtok(line, "\r \t\v");
        from = atol(tok);

    }


    fclose(stream);
}

void writeCSVFIle(char *filename)
{
}
