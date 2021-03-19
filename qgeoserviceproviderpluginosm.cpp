#include "qgeoserviceproviderpluginosm.h"
#include "qgeotiledmappingmanagerengineosm.h"

QGeoMappingManagerEngine *QGeoServiceProviderFactoryOsm::createMappingManagerEngine(
    const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString) const
{
    return new QGeoTiledMappingManagerEngineOsm(parameters, error, errorString);
}
