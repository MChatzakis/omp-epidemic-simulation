#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define BUCKETS

typedef struct Connection
{
    long id;
    struct Connection *next;
} Connection;

typedef struct Node
{
    long id;
} Node;

typedef struct Graph
{
    long maxSize;
    long currSize;
    Node **nodes;
} Graph;

Graph *Graph_init(long initSize)
{
    Graph *g;
    g = (Graph *)malloc(sizeof(Graph));
    if(!g){

    }

    return g;
}