#include "algorithms.h"
#include "maze.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARG_HEIGHT_LONG "--height"
#define ARG_HEIGHT_SHORT "-h"
#define ARG_WIDTH_LONG "--width"
#define ARG_WIDTH_SHORT "-w"

#define BLOCK u8"\u2588"

enum algorithms
{
    ALGO_DF_FLOOD,
    ALGO_KRUSKAL
};

typedef struct
{
    unsigned int width;
    unsigned int height;
    enum algorithms algorithm;
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
        }
        else if (strncmp(*arg, "--floodfill", sizeof("--floodfill")) == 0)
        {
            args->algorithm = ALGO_DF_FLOOD;
        }
        else if (strncmp(*arg, "--kruskal", sizeof("--kruskal")) == 0)
        {
            args->algorithm = ALGO_KRUSKAL;
        }
        else
        {
            goto usage;
        }
        arg++;
    }
    return 0;

usage:
    printf("usage: maze [-h|--height value] [-w|--width value] [--floodfill] [--kruskal]\n");
    return -1;
}

static const char* error_msg[] = {
    "",
    "unknown",
    "not yet implemented",
    "failed to allocate memory",
    "maze bounds too large"
};

int main(int argc, char* argv[])
{
    srand(time(NULL) + clock());

    if (setlocale(LC_ALL, "en_US.UTF-8") == NULL)
    {
        puts("error: failed to set locale");
        return EXIT_FAILURE;
    };

    arguments args = {10, 10, ALGO_DF_FLOOD};
    if (parse_args(&argv[1], &args) < 0)
        return EXIT_FAILURE;

    uint8_t *edges = NULL;

    if (SIZE_MAX / args.height < args.width)
    {
        printf("error: %s\n", error_msg[ERR_BOUNDS]);
        return EXIT_FAILURE;
    }

    size_t area = args.height * args.width;

    if ((edges = calloc((size_t)(area), sizeof(uint8_t))) == NULL)
    {
        printf("error: %s\n", error_msg[ERR_MEMORY]);
        return EXIT_FAILURE;
    }

    error_code ec = NO_ERROR;
    switch (args.algorithm)
    {
    case ALGO_DF_FLOOD:
        ec = df_flood(args.width, args.height, edges);
        break;
    case ALGO_KRUSKAL:
        ec = kruskal(args.width, args.height, edges);
        break;
    }

    if (ec)
    {
        printf("error: %s\n", error_msg[ec]);
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

