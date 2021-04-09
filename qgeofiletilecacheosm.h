/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QGEOFILETILECACHEOSM_H
#define QGEOFILETILECACHEOSM_H
#define Q_DECL_EXPORT
#include "qgeotileproviderosm.h"
#include <QtLocation/private/qgeofiletilecache_p.h>
#include <QHash>
#include <QtConcurrent>
#include <QSharedMemory>
#include <QtDebug>
#include <QPainter>
#include <math.h>
#include <qatomic.h>
#include <qtilefinder.h>
#include "coordinateStruct.h"
#include "LineDrawingStruct.h"

QT_BEGIN_NAMESPACE

class Q_DECL_EXPORT QGeoFileTileCacheOsm : public QGeoFileTileCache
{
    Q_OBJECT
public:
    QGeoFileTileCacheOsm(const QVector<QGeoTileProviderOsm *> &providers,
                         const QString &offlineDirectory = QString(),
                         const QString &directory = QString(),
                         QObject *parent = 0);
    QGeoFileTileCacheOsm();
    ~QGeoFileTileCacheOsm();
    static QGeoFileTileCacheOsm * instance;
    QVector<coordinateStruct> coordinates;
    QVector<CountDotsForLine>  drawingVector;
    QImage drawOnTile(QImage image);
    QGeoFileTileCacheOsm *getInstance();
    coordinateStruct xyToLatLon(const QGeoTileSpec &spec);
    double lattitude, longitude, longitudeOfTheTopRightCorner, lattitudeOfTheTopRightCorner, longitudeOfTheBottomLeftCorner,lattitudeOfTheBottomLeftCorner;
    double stepLattitude;
    double stepLongitude;
    bool isNeededTile(const QGeoTileSpec &spec, coordinateStruct coordinates);
    QSharedPointer<QGeoTileTexture> get(const QGeoTileSpec &spec) override;
    QSharedMemory rofl;

Q_SIGNALS:
    void mapDataUpdated(int mapId);

protected:
    void init() override;
    inline QString tileSpecToFilename(const QGeoTileSpec &spec, const QString &format) const;
    QGeoTileSpec filenameToTileSpec(const QString &filename) const override;
    QSharedPointer<QGeoTileTexture> getFromOfflineStorage(const QGeoTileSpec &spec);
    void dropTiles(int mapId);
    void loadTiles(int mapId);
    void prepareLineDrawing(int zoom, QVector<coordinateStruct>* coordinateVector);
    void clearObsoleteTiles(const QGeoTileProviderOsm *p);
    void stepLatLon(const QGeoTileSpec &spec);
    QDir m_offlineDirectory;
    bool m_offlineData;
    QVector<QGeoTileProviderOsm *> m_providers;
//    QVector<bool> m_highDpi;
    QVector<QDateTime> m_maxMapIdTimestamps;
};

QT_END_NAMESPACE

#endif // QGEOFILETILECACHEOSM_H
