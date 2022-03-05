#ifndef GEOMETRY_H_2pVPBUqZ39aBWKfa
#define GEOMETRY_H_2pVPBUqZ39aBWKfa

#include "headers.h"

typedef struct Point2D
{
    int x, y;
} Point2D;

typedef struct Size2D
{
    size_t width, height;
} Size2D;

typedef struct Rect2D
{
    Point2D position;
    Size2D size;
} Rect2D;

#endif