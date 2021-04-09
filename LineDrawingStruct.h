#ifndef LINEDRAWINGSTRUCT_H
#define LINEDRAWINGSTRUCT_H
#include <iostream>
struct LineDrawingStruct
{
    uint32_t pointLat;
    uint32_t pointLon;
    uint8_t x;
    uint8_t y;
};

struct CountDotsForLine
{
    uint32_t tileX;
    uint32_t tileY;
    uint32_t xFrom;
    uint32_t xTo;
    uint32_t yFrom;
    uint32_t yTo;
};

#endif // LINEDRAWINGSTRUCT_H
