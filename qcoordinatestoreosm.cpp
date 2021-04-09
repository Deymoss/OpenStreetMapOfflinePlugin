#include "qcoordinatestoreosm.h"

QCoordinateStoreOsm::QCoordinateStoreOsm(QVector<coordinateStruct> coordinates)
{
    this->coordinates = coordinates;
}

void QCoordinateStoreOsm::SetCoordinates(QVector<double> coordinates, int zoom)
{
    double longitude = coordinates.first();
    coordinates.pop_front();
    double lattitude = coordinates.first();
    coordinates.pop_front();
    int x = (longitude + 180)/360 * pow(2,zoom);
    double latrad = lattitude * M_PI/180.0;
    int y = (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << zoom)));


}
