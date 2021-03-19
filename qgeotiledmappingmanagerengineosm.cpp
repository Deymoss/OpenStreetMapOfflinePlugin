#include "qgeotiledmappingmanagerengineosm.h"
#include "qgeotilefetcherosm.h"
#include "qgeotiledmaposm.h"
#include "qgeofiletilecacheosm.h"

#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeotiledmap_p.h>
#include <QtLocation/private/qgeofiletilecache_p.h>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkDiskCache>
#include <QDebug>

QGeoTiledMappingManagerEngineOsm::QGeoTiledMappingManagerEngineOsm(const QVariantMap &parameters, QGeoServiceProvider::Error *error, QString *errorString)
:   QGeoTiledMappingManagerEngine()
{
    qDebug() << "Работает кастомный плагин";
    QGeoCameraCapabilities cameraCaps;
    cameraCaps.setMinimumZoomLevel(0.0);
    cameraCaps.setMaximumZoomLevel(19.0);
    cameraCaps.setSupportsBearing(true);
    cameraCaps.setSupportsTilting(true);
    cameraCaps.setMinimumTilt(0);
    cameraCaps.setMaximumTilt(80);
    cameraCaps.setMinimumFieldOfView(20.0);
    cameraCaps.setMaximumFieldOfView(120.0);
    cameraCaps.setOverzoomEnabled(true);
    setCameraCapabilities(cameraCaps);

    setTileSize(QSize(256, 256));

    const QByteArray pluginName = "osm";
    m_cacheDirectory = QAbstractGeoTileCache::baseLocationCacheDirectory() + QLatin1String(pluginName);

    QNetworkAccessManager *nmCached = new QNetworkAccessManager(this);
    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(m_cacheDirectory + QLatin1String("/providers"));
    diskCache->setMaximumCacheSize(100000000000); // enough to prevent diskCache to fiddle with tile cache. it's anyway used only for providers.
    nmCached->setCache(diskCache);

    QNetworkAccessManager *nm = new QNetworkAccessManager(); // Gets owned by QGeoTileFetcherOsm

    QVector<TileProvider *> providers_street;

    const QDateTime defaultTs = QDateTime::fromString(QStringLiteral("2016-06-01T00:00:00"), Qt::ISODate);
    providers_street.push_back(
        new TileProvider(QStringLiteral("http://c.tile.openstreetmap.org/%z/%x/%y.png"),
            QStringLiteral("png"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap.org</a>"),
            QStringLiteral("<a href='http://www.openstreetmap.org/copyright'>OpenStreetMap</a> contributors")));

    /* QGeoTileProviderOsms setup */
    m_providers.push_back( new QGeoTileProviderOsm( nmCached,
            QGeoMapType(QGeoMapType::StreetMap, tr("Street Map"), tr("Street map view in daylight mode"), false, false, 1, pluginName, cameraCaps),
            providers_street, cameraCaps ));

    bool disableRedirection = false;
    if (parameters.contains(QStringLiteral("osm_custom.mapping.providersrepository.disabled")))
        disableRedirection = parameters.value(QStringLiteral("osm_custom.mapping.providersrepository.disabled")).toBool();

    updateMapTypes();


    /* TILE CACHE */
    if (parameters.contains(QStringLiteral("osm_custom.mapping.offline.directory")))
        m_offlineDirectory = parameters.value(QStringLiteral("osm_custom.mapping.offline.directory")).toString();
    QGeoFileTileCacheOsm *tileCache = new QGeoFileTileCacheOsm(m_providers, m_offlineDirectory, m_cacheDirectory);

    tileCache->setCostStrategyDisk(QGeoFileTileCache::ByteSize);
    tileCache->setCostStrategyMemory(QGeoFileTileCache::ByteSize);
    tileCache->setCostStrategyTexture(QGeoFileTileCache::ByteSize);

    setTileCache(tileCache);

    /* TILE FETCHER */
    QGeoTileFetcherOsm *tileFetcher = new QGeoTileFetcherOsm( nm, this);
    setTileFetcher(tileFetcher);

    *error = QGeoServiceProvider::NoError;
    errorString->clear();
}

QGeoTiledMappingManagerEngineOsm::~QGeoTiledMappingManagerEngineOsm()
{
}

QGeoMap *QGeoTiledMappingManagerEngineOsm::createMap()
{
    QGeoTiledMap *map = new QGeoTiledMapOsm(this);
    connect(qobject_cast<QGeoFileTileCacheOsm *>(tileCache()), &QGeoFileTileCacheOsm::mapDataUpdated
            , map, &QGeoTiledMap::clearScene);
    map->setPrefetchStyle(m_prefetchStyle);
    return map;
}

//const QVector<QGeoTileProviderOsm *> &QGeoTiledMappingManagerEngineOsm::providers()
//{
//    return m_providers;
//}

QString QGeoTiledMappingManagerEngineOsm::customCopyright() const
{
    return m_customCopyright;
}

void QGeoTiledMappingManagerEngineOsm::updateMapTypes()
{
    QList<QGeoMapType> mapTypes;
    for (auto provider : m_providers) {
        // assume provider are ok until they have been resolved invalid
        if (!provider->isResolved() || provider->isValid())
            mapTypes << provider->mapType();
    }
    const QList<QGeoMapType> currentlySupportedMapTypes = supportedMapTypes();
    if (currentlySupportedMapTypes != mapTypes)
        // See map type implementations in QGeoTiledMapOsm and QGeoTileFetcherOsm.
        setSupportedMapTypes(mapTypes);
}
