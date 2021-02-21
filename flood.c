#include "algorithms.h"
#include "maze.h"
#include <assert.h>
#include <stdlib.h>

typedef struct
{
    unsigned int* buff;
    size_t p;
    size_t capacity;
} stack;

int stack_init(stack* s, size_t size)
{
    s->capacity = size;
    s->p = 0;
    s->buff = calloc(size, sizeof(unsigned int));
    if (s->buff == NULL)
        return -1;
    return 0;
}

void stack_destroy(stack* s)
{
    if (s && s->buff)
        free(s->buff);
}

void stack_push(stack* s, size_t value)
{
    assert(s);
    assert(s->p < s->capacity - 1);
    s->buff[s->p++] = value;
}

size_t stack_pop(stack* s)
{
    assert(s);
    assert(s->p > 0);
    return s->buff[s->p--];
}

size_t stack_size(stack* s)
{
    assert(s);
    return s->p;
}

#define MAX_NEIGHBORS 4
#define VISITED 0x10

/* precondition: width * height < UINT_MAX
 *
 */
error_code df_flood(unsigned int width, unsigned int height, uint8_t* edges)
{
    // The maze graph is always a regular lattice, so hold it directly in a (2D)
    // array (edges) which fully expresses the structure of the graph by
    // tracking the "south" and "east" edges for each node. Also tracks whether
    // a node has been visited.

    stack track;
    size_t area = height * width;

    if (stack_init(&track, area) == -1)
        return ERR_MEMORY;

    size_t cell = 0;

    // Compute the maze using a depth-first floodfill algorithm.

    do
    {
        edges[cell] |= VISITED;

        // find the set of unvisited neighbors
        size_t n = 0;
        char neighbors[MAX_NEIGHBORS] = {0};
        if ((cell - width < cell) && !(edges[cell - width] & VISITED))
            neighbors[n++] = 'n';
        if ((cell + 1 > cell) && ((cell + 1) % width > (cell % width)) && !(edges[cell + 1] & VISITED))
            neighbors[n++] = 'e';
        if ((cell + width < area) && (cell + width > cell) && !(edges[cell + width] & VISITED))
            neighbors[n++] = 's';
        if ((((cell - 1) % width) < (cell % width)) && (cell - 1 < cell) && !(edges[cell - 1] & VISITED))
            neighbors[n++] = 'w';

        // If there are unvisited neighbors, choose one at random and update the
        // state, otherwise backtrack one cell.

        if (n > 0)
        {
            size_t next = SIZE_MAX;
            switch (neighbors[rand() % n])
            {
            case 'n':
                next = cell - width;
                edges[next] |= SOUTH;
                break;
            case 'e':
                next = cell + 1;
                edges[cell] |= EAST;
                break;
            case 's':
                next = cell + width;
                edges[cell] |= SOUTH;
                break;
            case 'w':
                next = cell - 1;
                edges[next] |= EAST;
                break;
            default:
                assert(0);
            }
            assert(next < area);

            stack_push(&track, cell);
            cell = next;
        }
        else
        {
            cell = stack_pop(&track);
        }
    } while (stack_size(&track) > 0);

    stack_destroy(&track);
    return NO_ERROR;
}
