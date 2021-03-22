#ifndef QCOORDINATESTOREOSM_H
#define QCOORDINATESTOREOSM_H
#include <QVector>

class QCoordinateStoreOsm
{
public:
    QCoordinateStoreOsm();
    void SetCoordinates(QVector<double> coordinates);
};

#endif // QCOORDINATESTOREOSM_H
