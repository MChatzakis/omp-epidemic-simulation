#include "Graph.h"

Graph *Graph_init(unsigned int size)
{
    Graph *g;

    g = (Graph *)malloc(sizeof(Graph));
    if (!g)
    {
        perror("Could not allocate memory for graph");
        exit(EXIT_FAILURE);
    }

    g->maxSize = size;
    g->currSize = 0;
    g->nodes = (Node *)malloc(size * sizeof(Node));
    if (!g->nodes)
    {
        perror("Could not allocate memory for the initial nodes");
        exit(EXIT_FAILURE);
    }

    return g;
}

Node Graph_initNode(Node *nodes, long index, long id)
{
    assert(nodes);

    nodes[index].id = id;
    nodes[index].connections = 0;
    nodes[index].connectionsHead = NULL;
    nodes[index].isContaminated = 0;
    nodes[index].isDead = 0;

    return nodes[index];
}

Connection *Graph_initConn(Node *nodes, long indexFrom, long indexTo)
{
    Connection *curr, *prev, *conn;
    assert(nodes);

    curr = nodes[indexFrom].connectionsHead;
    prev = NULL;
    while (curr)
    {
        if (curr->indexTo == indexTo)
        {
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }

    conn = (Connection *)malloc(sizeof(Connection));
    if (!conn)
    {
        perror("Could not allocate memory for new connection");
        exit(EXIT_FAILURE);
    }

    conn->indexTo = indexTo;
    conn->contaminates = 0;
    conn->next = NULL;

    if (prev == NULL)
    {
        nodes[indexFrom].connectionsHead = conn;
    }
    else
    {
        prev->next = conn;
    }

    nodes[indexFrom].connections++;
    return conn;
}

int Graph_setContaminated(Graph *g, long ID)
{
    Node *nodes;
    int i;

    assert(g);

    nodes = g->nodes;

    for (i = 0; i < g->currSize; i++)
    {
        if (nodes[i].id == ID)
        {
            nodes[i].isContaminated = 1;
            return 1;
        }
    }

    return 0;
}

Node *Graph_addConnection(Graph *g, long ID1, long ID2)
{
    long indexID1 = -1, indexID2 = -1, i;
    Connection *curr, *prev, *conn;
    Node *nodes;

    assert(g);

    nodes = g->nodes;

    for (i = 0; i < g->currSize; i++)
    {
        if (nodes[i].id == ID1)
        {
            indexID1 = i;
        }

        if (nodes[i].id == ID2)
        {
            indexID2 = i;
        }

        if (indexID1 != -1 && indexID2 != -1)
        {
            break;
        }
    }

    if (indexID1 == -1)
    {
        indexID1 = g->currSize;
        Graph_initNode(nodes, indexID1, ID1);
        g->currSize++;
    }

    if (indexID2 == -1)
    {
        if (g->currSize >= g->maxSize)
        {
            g->maxSize = g->maxSize * 2;
            g->nodes = (Node *)realloc(g->nodes, g->maxSize * sizeof(Node));
        }
        indexID2 = g->currSize;
        Graph_initNode(nodes, indexID2, ID2);
        g->currSize++;
    }

    /*Directed Graph*/
    Graph_initConn(nodes, indexID1, indexID2);
    Graph_initConn(nodes, indexID2, indexID1);

    return nodes;
}

void Graph_print(Graph *g)
{
    long i;
    Node *nodes;
    Connection *curr;

    assert(g);
    nodes = g->nodes;

    for (i = 0; i < g->currSize; i++)
    {
        printf("Node #%ld:\n    ID: %ld\n    isContaminated: %d\n    Connections: %ld\n    isDead: %d\n    hasAnosia: %d\n    daysRecovering: %d\n",
               i, nodes[i].id, nodes[i].isContaminated, nodes[i].connections, nodes[i].isDead, nodes[i].hasAnosia, nodes[i].daysRecovering);

        printf("    =>[ ");
        curr = nodes[i].connectionsHead;
        while (curr)
        {
            printf("%ld ", nodes[curr->indexTo].id);
            curr = curr->next;
        }
        printf("]\n");
    }
}

Node *Graph_freeUnused(Graph *g)
{
    long required_size;
    assert(g);

    g->nodes = realloc(g->nodes, g->currSize * sizeof(Node));
    g->maxSize = g->currSize;

    return g->nodes;
}

void Graph_free(Graph *g)
{
    Node *nodes;
    Connection *curr, *prev;
    long i;

    assert(g);
    nodes = g->nodes;

    for (i = 0; i < g->currSize; i++)
    {
        curr = nodes[i].connectionsHead;
        while (curr)
        {
            prev = curr;
            curr = curr->next;

            free(prev);
            prev = NULL;
        }
    }

    free(nodes);
    nodes = NULL;
}