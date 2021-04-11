#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct Connection
{
    long indexTo;
    short contaminates;
    struct Connection *next;
} Connection;

typedef struct Node
{
    long id;

    short isDead;
    short isContaminated;
    short hasAnosia;
    short daysRecovering;

    long connections;
    Connection *connectionsHead;
} Node;

typedef struct Graph
{
    long maxSize;
    long currSize;
    Node *nodes;
} Graph;

Graph *Graph_init(unsigned int size);
Connection *Graph_initConn(Node *nodes, long indexFrom, long indexTo);
Node Graph_initNode(Node *nodes, long index, long id);
Node *Graph_addConnection(Graph *g, long ID1, long ID2);
Node *Graph_freeUnused(Graph *g);
int Graph_setContaminated(Graph *g, long ID);
void Graph_print(Graph *g);
void Graph_free(Graph *g);