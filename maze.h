#ifndef MAZE_H
#define MAZE_H

#define EAST 0x4
#define SOUTH 0x2

typedef enum
{
    NO_ERROR = 0,
    ERR_UNKNOWN,
    ERR_NOT_IMPLEMENTED,
    ERR_MEMORY,
    ERR_BOUNDS,
} error_code;

#endif
