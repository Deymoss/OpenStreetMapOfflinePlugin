#include "qgeofiletilecacheosm.h"
#include <QtLocation/private/qgeotilespec_p.h>
#include <QDir>
#include <QDirIterator>
#include <QPair>
#include <iostream>
#include <QDateTime>
#include <QtConcurrent>
#include <QThread>
QGeoFileTileCacheOsm *QGeoFileTileCacheOsm::instance = nullptr;
QGeoFileTileCacheOsm::QGeoFileTileCacheOsm(const QVector<QGeoTileProviderOsm *> &providers,
                                           const QString &offlineDirectory,
                                           const QString &directory,
                                           QObject *parent)
:   QGeoFileTileCache(directory, parent), m_offlineDirectory(offlineDirectory), m_offlineData(false), m_providers(providers)
{
    if (!offlineDirectory.isEmpty()) {
        m_offlineDirectory = QDir(offlineDirectory);
        if (m_offlineDirectory.exists())
            m_offlineData = true;
    }
    qDebug()<<"нормальный конструктор";

}
QGeoFileTileCacheOsm::~QGeoFileTileCacheOsm()
{
    std::cerr<<"Я упал"<<std::flush;
    
}

QImage QGeoFileTileCacheOsm::drawOnTile(QImage image, coordinateStruct coord, coordinateStruct startCoord, double stepLattitude, double stepLongitude)
{
    qDebug()<<"fuck";
    double lattitude = 53.9108;
    double longitude = 27.4850;
    int checkLat = (startCoord.lattitude - lattitude)/stepLattitude;
    int checkLon = (longitude - startCoord.longitude)/stepLongitude;
    //qDebug()<<checkLat<<" "<<checkLon;
    QPainter painter(&image);
    QPen pen;
    pen.setWidth(10);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    painter.drawPoint(checkLon,checkLat);
    return image;
}
QGeoFileTileCacheOsm::QGeoFileTileCacheOsm()
{
    qDebug()<<"Конструктор умолчания";
}
QGeoFileTileCacheOsm *QGeoFileTileCacheOsm::getInstance()
{
    return instance;
}

coordinateStruct QGeoFileTileCacheOsm::xyToLatLon(const QGeoTileSpec &spec)
{
    coordinateStruct coord;
    coord.longitude = spec.x() / (double)(1 << spec.zoom()) * 360.0 - 180;
    double n = M_PI - 2.0 * M_PI * spec.y() / (double)(1 << spec.zoom());
    coord.lattitude = 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
    return coord;
}

bool QGeoFileTileCacheOsm::isNeededTile(const QGeoTileSpec &spec, coordinateStruct coordinates)
{
    int x = (coordinates.longitude + 180)/360 * pow(2,spec.zoom());
    //qDebug()<<x;
    double latrad = coordinates.lattitude * M_PI/180.0;
    int y = (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << spec.zoom())));
    if(spec.x() == x && spec.y() == y)
    {
        return true;
    }
    else
    {
        return false;
    }
}

QSharedPointer<QGeoTileTexture> QGeoFileTileCacheOsm::get(const QGeoTileSpec &spec)
{
    QSharedPointer<QGeoTileTexture> tt = getFromOfflineStorage(spec);
    if (tt)
        return tt;
    if ((tt = getFromOfflineStorage(spec)))
        return tt;
    return getFromDisk(spec);
}

// init() is always called before the provider resolution starts
void QGeoFileTileCacheOsm::init()
{
    if (directory_.isEmpty())
        directory_ = baseLocationCacheDirectory();
    QDir::root().mkpath(directory_);

    // find max mapId
    int max = 0;
    for (auto p: m_providers)
        if (p->mapType().mapId() > max)
            max = p->mapType().mapId();
    // Create a mapId to maxTimestamp LUT..
    m_maxMapIdTimestamps.resize(max+1); // initializes to invalid QDateTime

    // .. by finding the newest file in each tileset (tileset = mapId).
    QDir dir(directory_);
    QStringList formats;
    formats << QLatin1String("*.*");
    QStringList files = dir.entryList(formats, QDir::Files);

    for (const QString &tileFileName : files) {
        QGeoTileSpec spec = filenameToTileSpec(tileFileName);
        if (spec.zoom() == -1)
            continue;
        QFileInfo fi(dir.filePath(tileFileName));
        if (fi.lastModified() > m_maxMapIdTimestamps[spec.mapId()])
            m_maxMapIdTimestamps[spec.mapId()] = fi.lastModified();
    }

    // Base class ::init()
    QGeoFileTileCache::init();

//    for (QGeoTileProviderOsm * p: m_providers)
//        clearObsoleteTiles(p);
}

QSharedPointer<QGeoTileTexture> QGeoFileTileCacheOsm::getFromOfflineStorage(const QGeoTileSpec &spec)
{
    QTileFinder *finder = new QTileFinder();
    coordinateStruct str;

    QImage image = finder->getTile(spec.x(),spec.y(),spec.zoom());
//    for(int i=0; i<coordinates.length(); i++)
//    {

    str.lattitude = 53.9108;
    str.longitude = 27.4850;
    coordinates.push_back(str);
    //qDebug()<<coordinates.at(0).lattitude<<" "<<coordinates.at(0).longitude;
        if(isNeededTile(spec, str))
        {
            QTileDataClass tile = finder->getTileInfo(spec.x(),spec.y(),spec.zoom());
            //qDebug()<<spec.x()<<spec.y();
            coordinateStruct startCoord = xyToLatLon(spec);
            stepLatLon(spec);
            image = drawOnTile(image, str, startCoord, stepLattitude, stepLongitude);
        }
//    }
    return addToTextureCache(spec, image);
}

void QGeoFileTileCacheOsm::dropTiles(int mapId)
{
    QList<QGeoTileSpec> keys;
    keys = textureCache_.keys();
    for (const QGeoTileSpec &k : keys)
        if (k.mapId() == mapId)
            textureCache_.remove(k);

    keys = memoryCache_.keys();
    for (const QGeoTileSpec &k : keys)
        if (k.mapId() == mapId)
            memoryCache_.remove(k);

    keys = diskCache_.keys();
    for (const QGeoTileSpec &k : keys)
        if (k.mapId() == mapId)
            diskCache_.remove(k);
}

void QGeoFileTileCacheOsm::loadTiles(int mapId)
{
    QStringList formats;
    formats << QLatin1String("*.*");

    QDir dir(directory_);
    QStringList files = dir.entryList(formats, QDir::Files);

    for (int i = 0; i < files.size(); ++i) {
        QGeoTileSpec spec = filenameToTileSpec(files.at(i));
        if (spec.zoom() == -1 || spec.mapId() != mapId)
            continue;
        QString filename = dir.filePath(files.at(i));
        addToDiskCache(spec, filename);
    }
}

void QGeoFileTileCacheOsm::stepLatLon(const QGeoTileSpec &spec)
{
    longitude = (spec.x()/pow(2,spec.zoom()))*360-180;
    lattitude = atan(sinh(M_PI-(spec.y()/pow(2,spec.zoom()))*(2*M_PI)))*(180/M_PI);
    longitudeOfTheTopRightCorner = (spec.x()/pow(2,spec.zoom()))*360-180;
    lattitudeOfTheTopRightCorner = atan(sinh(M_PI-((spec.y()+1)/pow(2,spec.zoom()))*(2*M_PI)))*(180/M_PI);
    longitudeOfTheBottomLeftCorner = ((spec.x()+1)/pow(2,spec.zoom()))*360-180;
    lattitudeOfTheBottomLeftCorner = atan(sinh(M_PI-(spec.x()/pow(2,spec.zoom()))*(2*M_PI)))*(180/M_PI);
    stepLattitude = (lattitude - lattitudeOfTheTopRightCorner)/256;
    stepLongitude = (longitudeOfTheBottomLeftCorner - longitude)/256;
}

QString QGeoFileTileCacheOsm::tileSpecToFilename(const QGeoTileSpec &spec, const QString &format) const
{

    QString filename = spec.plugin();
    filename += QLatin1String("-");
    filename += QLatin1Char('l');
    filename += QLatin1String("-");
    filename += QString::number(spec.mapId());
    filename += QLatin1String("-");
    filename += QString::number(spec.zoom());
    filename += QLatin1String("-");
    filename += QString::number(spec.x());
    filename += QLatin1String("-");
    filename += QString::number(spec.y());

    //Append version if real version number to ensure backwards compatibility and eviction of old tiles
    if (spec.version() != -1) {
        filename += QLatin1String("-");
        filename += QString::number(spec.version());
    }

    filename += QLatin1String(".");
    filename += format;
    return filename;
}

QGeoTileSpec QGeoFileTileCacheOsm::filenameToTileSpec(const QString &filename) const
{
    QGeoTileSpec emptySpec;

    QStringList parts = filename.split('.');

    if (parts.length() != 2)
        return emptySpec;

    QString name = parts.at(0);
    QStringList fields = name.split('-');

    int length = fields.length();
    if (length != 6 && length != 7)
        return emptySpec;

    QList<int> numbers;

    bool ok = false;
    for (int i = 2; i < length; ++i) {
        ok = false;
        int value = fields.at(i).toInt(&ok);
        if (!ok)
            return emptySpec;
        numbers.append(value);
    }

    if (numbers.at(0) > m_providers.size())
        return emptySpec;

    //File name without version, append default
    if (numbers.length() < 5)
        numbers.append(-1);

    return QGeoTileSpec(fields.at(0),
                    numbers.at(0),
                    numbers.at(1),
                    numbers.at(2),
                    numbers.at(3),
                    numbers.at(4));
}

