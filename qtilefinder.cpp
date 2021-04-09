#include "qtilefinder.h"
QTileFinder::QTileFinder()
{

    QFile file("/home/deymos/Qt/5.15.2/Src/qtlocation/src/plugins/geoservices/osm_custom/file.bin");
    if(file.open(QIODevice::ReadOnly))
    {
        QDataStream stream(&file);
        ConstantStruct constan;
        for(int i =0; i<20; i++)//запись констант в файл
        {
            stream>>constan;
            constants.push_back(constan);
           // qDebug()<<constan.countOfTiles;
        }

    }
    file.close();
}

QImage QTileFinder::getTile(uint32_t x, uint32_t y, uint32_t zoom)
{

    int countOfXNum = x-constants.at(zoom-1).xTileStart;
    int countOfYnum = y - constants.at(zoom-1).yTileStart;
    int countTls = constants.at(zoom-1).xTileCount*countOfYnum + countOfXNum;
    int result = 0;
    for(int k = 0; k<zoom-1; k++)
    {
        result += constants.at(k).countOfTiles;
    }
    countTls += result;
    QFile file("/home/deymos/Qt/5.15.2/Src/qtlocation/src/plugins/geoservices/osm_custom/file.bin");
    QTileDataClass *tile = new QTileDataClass(0,0,0,0,0,0,0);
    if(file.open(QIODevice::ReadOnly))
    {
        file.seek(400 + sizeof(QTileDataClass)*(countTls));
        QDataStream dataStream(&file);
        dataStream>>*tile;
        //qDebug()<<tile->x<<" "<<tile->y<<" "<<tile->zoom<<" "<<tile->startPoint<<" "<<tile->size;
    }
    QPixmap pixmap;
    QByteArray arr;
    QDataStream stream(&file);
    file.seek(tile->startPoint);
    arr = file.read(tile->size);
    QPixmap img;
    img.loadFromData(arr);
    QImage image(img.toImage());

    return image;
}

QTileDataClass QTileFinder::getTileInfo(uint32_t x, uint32_t y, uint32_t zoom)
{
    int countOfXNum = x-constants.at(zoom-1).xTileStart;
    int countOfYnum = y - constants.at(zoom-1).yTileStart;
    int countTls = constants.at(zoom-1).xTileCount*countOfYnum + countOfXNum;
    int result = 0;
    for(int k = 0; k<zoom - 1; k++)
    {
        result += constants.at(k).countOfTiles;
    }
    countTls += result;
    QFile file("/home/deymos/Qt/5.15.2/Src/qtlocation/src/plugins/geoservices/osm_custom/file.bin");
    QTileDataClass *tile = new QTileDataClass(0,0,0,0,0,0,0);
    if(file.open(QIODevice::ReadOnly))
    {
        file.seek(400 + sizeof(QTileDataClass)*(countTls));
        QDataStream dataStream(&file);
        dataStream>>*tile;
    }
    return *tile;
}
