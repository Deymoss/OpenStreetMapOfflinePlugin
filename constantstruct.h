#ifndef CONSTANTSTRUCT_H
#define CONSTANTSTRUCT_H

#include <QString>
#include <QDataStream>
#pragma pack(push,1)
class ConstantStruct
{

public:
    ConstantStruct() : countOfTiles(0), xTileStart(0), yTileStart(0), xTileCount(0), yTileCount(0) {}
    uint32_t countOfTiles;
    uint32_t xTileStart;
    uint32_t yTileStart;
    uint32_t xTileCount;
    uint32_t yTileCount;
    ConstantStruct(uint32_t count, uint32_t xStart, uint32_t yStart, uint32_t xCount, uint32_t yCount) : countOfTiles(count),
        xTileStart(xStart), yTileStart(yStart), xTileCount(xCount), yTileCount(yCount){
    }
    friend QDataStream& operator>>(QDataStream &stream, ConstantStruct &data);
    friend QDataStream& operator<<(QDataStream &stream, ConstantStruct data);
};
#pragma pack(pop)
#endif // CONSTANTSTRUCT_H
