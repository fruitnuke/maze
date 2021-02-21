#include "maze.h"
#include "algorithms.h"
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

unsigned int dset_root(unsigned int dset[], unsigned int member)
{
    while (dset[member] != member)
        member = dset[member];
    return member;
}

error_code kruskal(unsigned int width, unsigned int height, uint8_t* graph)
{
    typedef struct
    {
        unsigned int a;
        unsigned int b;
    } edge;

    // Most cells have two edges - to their eastern and southern
    // neighbors - except for the cells on the right and bottom edges,
    // which have one edge each, and the bottom-right-most cell which
    // has no edges.
    unsigned int nedges = (2 * (width - 1) * (height - 1)) + (width + height) - 2;

    // Initialize the array of walls.
    edge* walls = calloc(nedges, sizeof(edge));
    if (walls == NULL)
        return ERR_MEMORY;

    size_t i = 0;
    for (unsigned int cell = 0; cell < width * height; ++cell)
    {
        if (cell % width < width - 1)
        {
            walls[i].a = cell;
            walls[i].b = cell + 1;
            ++i;
        }
        if (cell / width < height - 1)
        {
            walls[i].a = cell;
            walls[i].b = cell + width;
            ++i;
        }
    }
    assert(i == nedges);

    // Shuffle the array of walls.
    --i;
    for (; i > 0; --i)
    {
        size_t j = rand() % i;
        edge tmp = walls[i];
        walls[i] = walls[j];
        walls[j] = tmp;
    };

    // A disjoint set data structure.
    unsigned int* sets = calloc(width * height, sizeof(sets[0]));
    if (sets == NULL)
        return ERR_MEMORY;
    for (size_t i = 0; i < width * height; ++i)
        sets[i] = i;

    // The actual algorithm: for each edge merge the disjoint sets of
    // the two nodes it connects, if they haven't already been merged,
    // and remove the "wall".
    for (size_t i = 0; i < nedges; ++i)
    {
        unsigned int node_a = walls[i].a;
        unsigned int node_b = walls[i].b;
        unsigned int set_a = dset_root(sets, node_a);
        unsigned int set_b = dset_root(sets, node_b);
        if (set_a != set_b)
        {
            graph[node_a] |= (node_b == node_a + 1 ? EAST : SOUTH);
            sets[set_b] = set_a;
        }
    }

    free(sets);
    free(walls);

    return NO_ERROR;
}

