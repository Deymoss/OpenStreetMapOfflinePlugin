#ifndef QTILEDATACLASS_H
#define QTILEDATACLASS_H


#include <QString>
#include <QDataStream>
#include <QDebug>
#pragma pack(push,1)
class QTileDataClass
{

public:
    QTileDataClass() : x( 0 ), y( 0 ), zoom(0), size(0), startPoint(0) { }
    uint32_t x;
    uint32_t y;
    uint8_t zoom;
    uint32_t size;
    uint32_t startPoint;
        QTileDataClass( uint32_t xx, uint32_t yy, uint8_t zzoom, uint32_t ssize, uint32_t sstartPoint) : x( xx ), y( yy ), zoom(zzoom), size(ssize), startPoint(sstartPoint) {
        }
    friend QDataStream& operator>>(QDataStream &stream, QTileDataClass &data);
    friend QDataStream& operator<<(QDataStream &stream, QTileDataClass data);
};
#pragma pack(pop)

#endif // QTILEDATACLASS_H
