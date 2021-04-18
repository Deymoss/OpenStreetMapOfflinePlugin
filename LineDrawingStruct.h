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
    int tileX;
    int tileY;
    int xFrom;
    int xTo;
    int yFrom;
    int yTo;
};

#endif // LINEDRAWINGSTRUCT_H
