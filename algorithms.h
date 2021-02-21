#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "maze.h"
#include <stdint.h>

error_code df_flood(unsigned int width, unsigned int height, uint8_t* graph);
error_code kruskal(unsigned int width, unsigned int height, uint8_t* graph);

#endif
