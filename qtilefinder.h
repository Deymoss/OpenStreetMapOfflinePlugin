#ifndef QTILEFINDER_H
#define QTILEFINDER_H
#include <QImage>
#include <QVector>
#include <QFile>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include <QPixmap>
#include <constantstruct.h>
#include <qtiledataclass.h>
class QTileFinder
{
public:
    QTileFinder();
    QImage getTile(uint32_t x, uint32_t y, uint32_t zoom);
    QTileDataClass getTileInfo(uint32_t x, uint32_t y, uint32_t zoom);
    QVector<ConstantStruct> constants;
};

#endif // QTILEFINDER_H
