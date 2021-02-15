#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
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

#define BLOCK u8"\u2588"

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

typedef struct
{
    unsigned int width;
    unsigned int height;
} arguments;

int parse_args(char** argv, arguments* args)
{
    for (char** arg = argv; *arg != NULL;)
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
            args->height = (unsigned int)value;
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
            args->width = (unsigned int)value;
            arg++;
        }
        else
        {
            goto usage;
        }
    }
    return 0;

usage:
    printf("usage: maze [-h|--height 1..UINT_MAX] [-w|--width 1..UINT_MAX]\n");
    return -1;
}

enum errors
{
    ERROR_UNKNOWN = -1,
    ERROR_OUT_OF_MEMORY = -2,
    ERROR_MAZE_TOO_LARGE = -3,
};

int depth_first_maze(arguments args, uint8_t* edges)
{
    // The maze graph is always a regular lattice, so hold it directly in a (2D)
    // array (edges) which fully expresses the structure of the graph by
    // tracking the "south" and "east" edges for each node. Also tracks whether
    // a node has been visited.

    stack track;
    size_t area = args.height * args.width;

    if (stack_init(&track, area) == -1)
        return ERROR_OUT_OF_MEMORY;

    size_t cell = 0;

    // Compute the maze using a depth-first floodfill algorithm.

    do
    {
        edges[cell] |= VISITED;

        // find the set of unvisited neighbors
        size_t n = 0;
        char neighbors[MAX_NEIGHBORS] = {0};
        if ((cell - args.width < cell) && !(edges[cell - args.width] & VISITED))
            neighbors[n++] = 'n';
        if ((cell + 1 > cell) && ((cell + 1) % args.width > (cell % args.width)) && !(edges[cell + 1] & VISITED))
            neighbors[n++] = 'e';
        if ((cell + args.width < area) && (cell + args.width > cell) && !(edges[cell + args.width] & VISITED))
            neighbors[n++] = 's';
        if ((((cell - 1) % args.width) < (cell % args.width)) && (cell - 1 < cell) && !(edges[cell - 1] & VISITED))
            neighbors[n++] = 'w';

        // If there are unvisited neighbors, choose one at random and update the
        // state, otherwise backtrack one cell.

        if (n > 0)
        {
            size_t next = SIZE_MAX;
            switch (neighbors[rand() % n])
            {
            case 'n':
                next = cell - args.width;
                edges[next] |= SOUTH;
                break;
            case 'e':
                next = cell + 1;
                edges[cell] |= EAST;
                break;
            case 's':
                next = cell + args.width;
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
    return 0;
}

int main(int argc, char* argv[])
{
    srand(time(NULL) + clock());

    if (setlocale(LC_ALL, "en_US.UTF-8") == NULL)
    {
        puts("error: failed to set locale");
        return EXIT_FAILURE;
    };

    arguments args = {10, 10};
    if (parse_args(&argv[1], &args) < 0)
        return EXIT_FAILURE;

    uint8_t *edges = NULL;

    if (SIZE_MAX / args.height < args.width)
    {
        puts("error: maze area too big");
        return EXIT_FAILURE;
    }

    size_t area = args.height * args.width;

    if ((edges = calloc((size_t)(area), sizeof(uint8_t))) == NULL)
    {
        puts("error: out of memory");
        return EXIT_FAILURE;
    }

    if (depth_first_maze(args, edges) < 0)
    {
        puts("error: out of memory");
        return EXIT_FAILURE;
    }

    // Render to screen.

    for (unsigned long y = 0; y < args.height * 2; y++)
    {
        for (unsigned long x = 0; x < args.width * 4; x++)
        {
            size_t cell = (y / 2) * args.width + (x / 4);
            if (x % 4 < 2 && y % 2 == 0) printf(BLOCK);
            else if (x % 4 > 1 && y % 2 == 1) printf(" ");
            else if (x % 4 < 2 && y % 2 == 1) printf(edges[cell] & SOUTH ? BLOCK : " ");
            else if (x % 4 > 1 && y % 2 == 0) printf(edges[cell] & EAST ? BLOCK : " ");
            else printf(" ");
        }
        printf("\n");
    }

    free(edges);
    return EXIT_SUCCESS;
}

