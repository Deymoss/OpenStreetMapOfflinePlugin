TARGET = qtgeoservices_osm_custom

QT += location-private positioning-private network concurrent
CONFIG += c++11

PLUGIN_TYPE = geoservices
PLUGIN_CLASS_NAME = QGeoServiceProviderFactoryOsmCustom
load(qt_plugin)

DISTFILES += \
    osm_plugin.json \
    providers/5.8/cycle \
    providers/5.8/hiking \
    providers/5.8/night-transit \
    providers/5.8/satellite \
    providers/5.8/street \
    providers/5.8/street-hires \
    providers/5.8/terrain \
    providers/5.8/transit

HEADERS += \
    constantstruct.h \
    coordinateStruct.h \
    qcoordinatestoreosm.h \
    qgeofiletilecacheosm.h \
    qgeomapreplyosm.h \
    qgeoserviceproviderpluginosm.h \
    qgeotiledmaposm.h \
    qgeotiledmappingmanagerengineosm.h \
    qgeotilefetcherosm.h \
    qgeotileproviderosm.h \
    qtiledataclass.h \
    qtilefinder.h

SOURCES += \
    constantstruct.cpp \
    qcoordinatestoreosm.cpp \
    qgeofiletilecacheosm.cpp \
    qgeomapreplyosm.cpp \
    qgeoserviceproviderpluginosm.cpp \
    qgeotiledmaposm.cpp \
    qgeotiledmappingmanagerengineosm.cpp \
    qgeotilefetcherosm.cpp \
    qgeotileproviderosm.cpp \
    qtiledataclass.cpp \
    qtilefinder.cpp
