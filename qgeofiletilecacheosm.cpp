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
    //qDebug()<<instance;
////    if(!instance)
////    {
////        qDebug()<<this;
////        instance = this;
////    }
//     rofl.setKey("/test");
////    qDebug()<<"SOZDANIE: "<<rofl.create(8);
////   // rofl.lock();

//    rofl.attach();
////    qDebug()<<rofl.errorString();
//    auto cst = qobject_cast<QObject*>(this);
//    char *ptr = (char*)rofl.data();
//    const char *from = (char*)cst;
//    memcpy(ptr,from,sizeof(this));
    //qDebug()<<"1"<<(char*) this;
}
QGeoFileTileCacheOsm::~QGeoFileTileCacheOsm()
{
    std::cerr<<"Я упал"<<std::flush;

}
QGeoFileTileCacheOsm::QGeoFileTileCacheOsm()
{
    qDebug()<<"Конструктор умолчания";
}
QGeoFileTileCacheOsm *QGeoFileTileCacheOsm::getInstance()
{
    return instance;
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
//    if (!m_offlineData)
//        return QSharedPointer<QGeoTileTexture>();

//    int providerId = spec.mapId() - 1;
//    if (providerId < 0 || providerId >= m_providers.size())
//        return QSharedPointer<QGeoTileTexture>();

//    const QString fileName = tileSpecToFilename(spec, QStringLiteral("*"));
//    QStringList validTiles = m_offlineDirectory.entryList({fileName});
//    if (!validTiles.size())
//        return QSharedPointer<QGeoTileTexture>();

//    QFile file(m_offlineDirectory.absoluteFilePath(validTiles.first()));
//    qDebug()<<spec;
    QTileFinder *finder = new QTileFinder();

//    if (!file.open(QIODevice::ReadOnly))
//        return QSharedPointer<QGeoTileTexture>();
//    QByteArray bytes = file.readAll();
//    file.close();
    QImage image = finder->getTile(spec.x(),spec.y(),spec.zoom());;

//    if (!image.loadFromData(bytes)) {
//        handleError(spec, QLatin1String("Problem with tile image"));
//        return QSharedPointer<QGeoTileTexture>(0);
//    }

    //addToMemoryCache(spec, bytes, QString());
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

