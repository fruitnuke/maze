#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define HEIGHT 10
#define WIDTH 10
#define AREA HEIGHT * WIDTH
#define MAX_NEIGHBORS 4

#define VISITED 0x10
#define EAST 0x4
#define SOUTH 0x2

typedef struct
{
    size_t buff[AREA];
    size_t p;
} stack;

void stack_push(stack* s, size_t value)
{
    s->buff[s->p++] = value;
    assert(s->p < AREA);
}

size_t stack_pop(stack* s)
{
    assert(s->p > 0);
    return s->buff[s->p--];
}

size_t stack_size(stack* s)
{
    return s->p;
}

int main(int argc, char** argv)
{
    srand(time(NULL));

    uint8_t edges[AREA] = {0};
    uint8_t bitmap[WIDTH * 4][HEIGHT * 2] = {0};

    stack track = {0};
    size_t cell = 0;

    do
    {
        edges[cell] |= VISITED;

        // find the set of unvisited neighbors
        size_t n = 0;
        char neighbors[MAX_NEIGHBORS] = {0};
        if ((cell - WIDTH < cell) && !(edges[cell - WIDTH] & VISITED))
            neighbors[n++] = 'n';
        if ((cell + 1 > cell) && ((cell + 1) % WIDTH > (cell % WIDTH)) && !(edges[cell + 1] & VISITED))
            neighbors[n++] = 'e';
        if ((cell + WIDTH < AREA) && (cell + WIDTH > cell) && !(edges[cell + WIDTH] & VISITED))
            neighbors[n++] = 's';
        if ((((cell - 1) % WIDTH) < (cell % WIDTH)) && (cell - 1 < cell) && !(edges[cell - 1] & VISITED))
            neighbors[n++] = 'w';

        // If there are unvisited neighbors, choose one at random and update the
        // state, otherwise backtrack one cell.

        if (n > 0)
        {
            size_t next = SIZE_MAX;
            switch (neighbors[rand() % n])
            {
            case 'n':
                next = cell - WIDTH;
                edges[next] |= SOUTH;
                break;
            case 'e':
                next = cell + 1;
                edges[cell] |= EAST;
                break;
            case 's':
                next = cell + WIDTH;
                edges[cell] |= SOUTH;
                break;
            case 'w':
                next = cell - 1;
                edges[next] |= EAST;
                break;
            default:
                assert(0);
            }
            assert(next < AREA);

            stack_push(&track, cell);
            cell = next;
        }
        else
        {
            cell = stack_pop(&track);
        }
    } while (stack_size(&track) > 0);

    // render to a scaled bitmap first
    for (int y = 0; y < HEIGHT; y++)
    {
        for (int x = 0; x < WIDTH; x++)
        {
            size_t cell = y * WIDTH + x;
            size_t i = x * 4;
            size_t j = y * 2;
            if (edges[cell] & VISITED)
                bitmap[i][j] = bitmap[i+1][j] = 1;
            if (edges[cell] & EAST)
                bitmap[i+2][j] = bitmap[i+3][j] = 1;
            if (edges[cell] & SOUTH)
                bitmap[i][j+1] = bitmap[i+1][j+1] = 1;
        }
    }
    
    // then print the bitmap
    for (int y = 0; y < HEIGHT * 2; y++)
    {
        for (int x = 0; x < WIDTH * 4; x++)
        {
            printf(bitmap[x][y] == 1 ? "#" : " ");
        }
        printf("\n");
    }
}
