#include "constantstruct.h"

QDataStream& operator>>(QDataStream &stream, ConstantStruct &data)
{
    stream>>data.countOfTiles>>data.xTileStart>>data.yTileStart>>data.xTileCount>>data.yTileCount;
    return stream;
}
QDataStream& operator<<(QDataStream &stream, ConstantStruct data)
{
    stream<<data.countOfTiles<<data.xTileStart<<data.yTileStart<<data.xTileCount<<data.yTileCount;
    return stream;
}
