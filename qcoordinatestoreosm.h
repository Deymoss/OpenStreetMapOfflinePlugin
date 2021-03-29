#ifndef QCOORDINATESTOREOSM_H
#define QCOORDINATESTOREOSM_H
#include <QVector>
#include <math.h>
#include "coordinateStruct.h"
class QCoordinateStoreOsm
{
public:
    QCoordinateStoreOsm(QVector<coordinateStruct> coordinates);
    QVector<coordinateStruct> coordinates;
    void SetCoordinates(QVector<double> coordinates, int zoom);
};

#endif // QCOORDINATESTOREOSM_H
