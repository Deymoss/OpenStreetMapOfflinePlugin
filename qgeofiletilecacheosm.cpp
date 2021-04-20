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
    coordinateStruct str;
    str.lattitude = 52.140;
    str.longitude = 24.093;
    coordinates.push_back(str);
    coordinateStruct str1;
    str1.lattitude = 53.762;
    str1.longitude = 26.202;
    coordinates.push_back(str1);
    coordinateStruct str2;
    str2.lattitude = 54.079;
    str2.longitude = 25.433;
    coordinates.push_back(str2);
    coordinateStruct str3;
    str3.lattitude = 54.470;
    str3.longitude = 27.883;
    coordinates.push_back(str3);
    coordinateStruct str4;
    str4.lattitude = 55.147;
    str4 .longitude = 29.729;
    coordinates.push_back(str4);
    coordinateStruct str5;
    str5.lattitude = 54.272;
    str5.longitude = 30.168;
    coordinates.push_back(str5);
    coordinateStruct str6;
    str6.lattitude = 52.362;
    str6.longitude = 24.478;
    coordinates.push_back(str6);
    qDebug()<<"нормальный конструктор";

}
QGeoFileTileCacheOsm::~QGeoFileTileCacheOsm()
{
    std::cerr<<"Я упал"<<std::flush;
    
}

QImage QGeoFileTileCacheOsm::drawOnTile(QImage image, const QGeoTileSpec &spec)
{
    QPainter painter(&image);
    QPen pen;
    pen.setWidth(2);
    pen.setColor(Qt::black);
    painter.setPen(pen);
    //qDebug()<<drawingVector.at(currPosOfDrawing).tileX<<" "<<spec.x()<<" "<<drawingVector.at(currPosOfDrawing).tileY<<" "<<spec.y();
    for(int i=0; i<drawingVector.size(); i++)
    {
        if(drawingVector.at(i).tileX == spec.x()&&drawingVector.at(i).tileY==spec.y())
        {
            painter.drawLine(drawingVector.at(i).xFrom, drawingVector.at(i).yFrom,
                             drawingVector.at(i).xTo, drawingVector.at(i).yTo);
        }
    }


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

bool QGeoFileTileCacheOsm::isNeededTile(const QGeoTileSpec &spec)
{
    for(int i = 0; i<drawingVector.size(); i++)
    {
    if(drawingVector.at(i).tileX == (uint32_t)spec.x() && drawingVector.at(i).tileY == (uint32_t)spec.y())
    {
        return true;
    }
    }
        return false;
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
    coordinateStruct str;
    if(isNewZoom!=spec.zoom())
    {
        qDebug()<<isNewZoom<<" "<<spec.zoom();
        isNewZoom = spec.zoom();
        stepLatLon(spec);
        prepareLineDrawing(spec.zoom(), &coordinates);
    }
    QTileFinder *finder = new QTileFinder();

    QImage image = finder->getTile(spec.x(),spec.y(),spec.zoom());
        if(isNeededTile(spec))
        {
            image = drawOnTile(image,spec);
        }
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

void QGeoFileTileCacheOsm::prepareLineDrawing(int zoom, QVector<coordinateStruct> *coordinateVector)
{
    int xMerk1,yMerk1,xMerk2,yMerk2,count = 0;
    int offsetX1;
    int offsetY1;
    int offsetX2;
    int offsetY2;
    drawingVector.clear();
    while(coordinateVector->size()!=count+1)
    {
        xMerk1 = (coordinateVector->at(count).longitude + 180)/360 * pow(2,zoom);
        double latrad = coordinateVector->at(count).lattitude * M_PI/180.0;
        yMerk1 = (int)(floor((1.0 - asinh(tan(latrad)) / M_PI) / 2.0 * (1 << zoom)));
        xMerk2 = (coordinateVector->at(count+1).longitude + 180)/360 * pow(2,zoom);
        double latrad1 = coordinateVector->at(count+1).lattitude * M_PI/180.0;
        yMerk2 = (int)(floor((1.0 - asinh(tan(latrad1)) / M_PI) / 2.0 * (1 << zoom)));
        QGeoTileSpec startSpec;
        startSpec.setX(xMerk1);
        startSpec.setY(yMerk1);
        startSpec.setZoom(zoom);
        stepLatLon(startSpec);
        offsetY1 = abs((lattitude - coordinateVector->at(count).lattitude))/stepLattitude;
        offsetX1 = abs((longitude - coordinateVector->at(count).longitude))/stepLongitude;//посчитать вручную
        startSpec.setX(xMerk2);
        startSpec.setY(yMerk2);
        stepLatLon(startSpec);
        offsetY2 = abs((lattitude - coordinateVector->at(count+1).lattitude)/stepLattitude);
        offsetX2 = abs((longitude - coordinateVector->at(count+1).longitude)/stepLongitude);
        if(xMerk1 == xMerk2 && yMerk1 == yMerk2)
        {
            CountDotsForLine dots;
            dots.tileX = xMerk1;
            dots.tileY = yMerk1;
            dots.xFrom = offsetX1;
            dots.yFrom = offsetY1;
            dots.xTo = offsetX2;
            dots.yTo = offsetY2;
            drawingVector.push_back(dots);
        }
        else
        {
            double longitud = coordinateVector->at(count+1).longitude - coordinateVector->at(count).longitude;
            double lattitud = coordinateVector->at(count+1).lattitude - coordinateVector->at(count).lattitude;
            int pixelsLon = longitud/stepLongitude;
            int pixelsLat = lattitud/stepLattitude;
            //qDebug()<< pixelsLon<<" "<<pixelsLat;
            int pixelsLonCount = 0;
            int pixelsLatCount = 0;
            int lastXoffset = 0;
            int lastYoffset = 0;
            int offsetXForThisTile = offsetX1;
            int offsetYForThisTile = offsetY1;
            int startXOffset = offsetX1;
            int startYOffset = offsetY1;
            while(pixelsLonCount != pixelsLon && pixelsLatCount != pixelsLat)
            {
                if(pixelsLon<0)
                {
                   pixelsLonCount--;
                   offsetXForThisTile += pixelsLonCount - lastXoffset;
                }
                else
                {
                   pixelsLonCount++;
                   offsetXForThisTile += pixelsLonCount - lastXoffset;
                }
                pixelsLatCount = floor((double)pixelsLat/(double)pixelsLon*pixelsLonCount);
                if(pixelsLat<0)
                {
                offsetYForThisTile += -pixelsLatCount + lastYoffset;//неправильно рисует, подумать
                }
                else
                {
                    offsetYForThisTile -= pixelsLatCount - lastYoffset;
                }
                //qDebug()<<offsetXForThisTile<<" "<<offsetYForThisTile<<" "<<pixelsLatCount<<" "<<pixelsLon<<" "<<pixelsLat<<" "<<lastYoffset<<" "<<pixelsLonCount;
                lastXoffset = pixelsLonCount;
                lastYoffset = pixelsLatCount;
                if(offsetXForThisTile <= 0)
                {
                    CountDotsForLine dots;
                    dots.tileX = xMerk1;
                    dots.tileY = yMerk1;
                    dots.xFrom = startXOffset;
                    dots.yFrom = startYOffset;
                    dots.xTo = offsetXForThisTile;
                    dots.yTo = offsetYForThisTile;
                    drawingVector.push_back(dots);
                    xMerk1--;
                    startXOffset = 256;
                    startYOffset = offsetYForThisTile;
                    offsetXForThisTile = 255;
                }
                else if(offsetXForThisTile >= 255)
                {
                    CountDotsForLine dots;
                    dots.tileX = xMerk1;
                    dots.tileY = yMerk1;
                    dots.xFrom = startXOffset;
                    dots.yFrom = startYOffset;
                    dots.xTo = offsetXForThisTile;
                    dots.yTo = offsetYForThisTile;//мб тут
                    drawingVector.push_back(dots);
                    xMerk1++;
                    startXOffset = 0;
                    startYOffset = offsetYForThisTile;
                    offsetXForThisTile = 1;
                    //qDebug()<<"Fuck"<<offsetXForThisTile;
                }
                if(offsetYForThisTile >= 256)
                {
                    CountDotsForLine dots;
                    dots.tileX = xMerk1;
                    dots.tileY = yMerk1;
                    dots.xFrom = startXOffset;
                    dots.yFrom = startYOffset;
                    dots.xTo = offsetXForThisTile;
                    dots.yTo = offsetYForThisTile;
                    drawingVector.push_back(dots);
                    yMerk1++;
                    startXOffset = offsetXForThisTile;
                    startYOffset = 0;
                    offsetYForThisTile = 1;
                }
                else if(offsetYForThisTile <= 0)
                {
                    CountDotsForLine dots;
                    dots.tileX = xMerk1;
                    dots.tileY = yMerk1;
                    dots.xFrom = startXOffset;
                    dots.yFrom = startYOffset;
                    dots.xTo = offsetXForThisTile;
                    dots.yTo = offsetYForThisTile;
                    drawingVector.push_back(dots);
                    yMerk1--;
                    startXOffset = offsetXForThisTile;
                    startYOffset = 256;
                    offsetYForThisTile = 255;
                }
            }
            if(offsetXForThisTile>0&&offsetXForThisTile<256&&offsetYForThisTile>0&&offsetYForThisTile<256)
            {
            CountDotsForLine dots;
            dots.tileX = xMerk1;
            dots.tileY = yMerk1;
            dots.xFrom = startXOffset;
            dots.yFrom = startYOffset;
            dots.xTo = offsetXForThisTile;
            dots.yTo = offsetYForThisTile;
            drawingVector.push_back(dots);
        }
        }
        count++;
    }
}

void QGeoFileTileCacheOsm::stepLatLon(const QGeoTileSpec &spec)
{
    longitude = (spec.x()/pow(2,spec.zoom()))*360-180;
    lattitude = atan(sinh(M_PI-(spec.y()/pow(2,spec.zoom()))*(2*M_PI)))*(180/M_PI);
    longitudeOfTheTopRightCorner = (spec.x()/pow(2,spec.zoom()))*360-180;
    double lattitudeOfTheTopRightCorner = atan(sinh(M_PI-((spec.y()+1)/pow(2,spec.zoom()))*(2*M_PI)))*(180/M_PI);
    double longitudeOfTheBottomLeftCorner = ((spec.x()+1)/pow(2,spec.zoom()))*360-180;
    double lattitudeOfTheBottomLeftCorner = atan(sinh(M_PI-(spec.x()/pow(2,spec.zoom()))*(2*M_PI)))*(180/M_PI);
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

