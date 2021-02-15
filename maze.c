#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NEIGHBORS 4

#define VISITED 0x10
#define EAST 0x4
#define SOUTH 0x2

#define ARG_HEIGHT_LONG "--height"
#define ARG_HEIGHT_SHORT "-h"
#define ARG_WIDTH_LONG "--width"
#define ARG_WIDTH_SHORT "-w"

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

int main(int argc, char* argv[])
{
    srand(time(NULL) + clock());

    unsigned int width = 10;
    unsigned int height = 10;

    for (char** arg = &argv[1]; *arg != NULL;)
    {
        if (strncmp(*arg, ARG_HEIGHT_LONG, sizeof(ARG_HEIGHT_LONG)) == 0 ||
            strncmp(*arg, ARG_HEIGHT_SHORT, sizeof(ARG_HEIGHT_SHORT)) == 0)
        {
            arg++;
            if (*arg == NULL)
                goto usage;

            errno = 0;
            long value = strtol(*arg, /*str_end*/ NULL, /*base*/ 10);
            if (errno || value < 1 || value > UINT_MAX)
                goto usage;
            height = (unsigned int)value;
            arg++;
        }
        else if (strncmp(*arg, ARG_WIDTH_LONG, sizeof(ARG_WIDTH_LONG)) == 0 ||
                 strncmp(*arg, ARG_WIDTH_SHORT, sizeof(ARG_WIDTH_SHORT)) == 0)
        {
            arg++;
            if (*arg == NULL)
                goto usage;

            errno = 0;
            long value = strtol(*arg, /*str_end*/ NULL, /*base*/ 10);
            if (errno || value < 1 || value > UINT_MAX)
                goto usage;
            width = (unsigned int)value;
            arg++;
        }
        else
        {
            goto usage;
        }
    }

    // The maze graph is always a regular lattice, so hold it directly in a (2D)
    // array (edges) which fully expresses the structure of the graph by
    // tracking the "south" and "east" edges for each node. Also tracks whether
    // a node has been visited.

    uint8_t *edges = NULL;
    stack track;

    if (SIZE_MAX / height < width)
    {
        puts("error: maze area too big");
        goto error;
    }

    size_t area = height * width;

    if ((edges = calloc((size_t)(area), sizeof(uint8_t))) == NULL)
    {
        puts("error: failed to allocate memory for maze");
        goto error;
    }

    if (stack_init(&track, area) == -1)
    {
        puts("error: failed to initialize stack");
        goto error;
    }

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

    // Render to screen.

    for (unsigned long y = 0; y < height * 2; y++)
    {
        for (unsigned long x = 0; x < width * 4; x++)
        {
            size_t cell = (y / 2) * width + (x / 4);
            if (x % 4 < 2 && y % 2 == 0) printf("#");
            else if (x % 4 > 1 && y % 2 == 1) printf(" ");
            else if (x % 4 < 2 && y % 2 == 1) printf(edges[cell] & SOUTH ? "#" : " ");
            else if (x % 4 > 1 && y % 2 == 0) printf(edges[cell] & EAST ? "#" : " ");
            else printf(" ");
        }
        printf("\n");
    }

    free(edges);
    stack_destroy(&track);
    return 0;

error:
    free(edges);
    stack_destroy(&track);
    return 1;

usage:
    printf("usage: maze [-h|--height 1..UINT_MAX] [-w|--width 1..UINT_MAX]\n");
    return 1;
}

