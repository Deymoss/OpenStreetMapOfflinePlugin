# OpenStreetMapOfflinePlugin
Plugin for drawing a map based on [libosmscout](https://github.com/Framstag/libosmscout), this plugin works in conjunction with [QOsmTilerGui](https://github.com/Deymoss/Tiler)
### Foreword
First of all, I must say that I did this plugin for search teams that search for people in difficult conditions, that is, I did not have a goal to create a plugin that would build routes or help find the nearest cafe / store, etc., I needed a plugin that draws a map of the area with maximum speed and minimum battery use, and can draw a route that has already been passed so as not to look for people in the same place.
### Manual
You can read about installing libosmscout [here](https://github.com/Deymoss/Tiler).

After installing clone this repository in `/home/username/Qt/5.15.2(or your version of qt)/Src/qtlocation/src/plugins/geoservices/`

Build it and find `libqtgeoservices_osm_custom.so` and `libqtgeoservices_osm_custom.so.debug`, it can be in /plugins/geoservices/ in root dir, move them into `/home/username/Qt/5.15.2(or your version of qt)/gcc_64/plugins/geoservices`

Find `tiles.bin` in build folder of QOsmTilerGui, and move it to folder with plugin.

Now, in your qml app in map item you must set plugin parameter like this:

    plugin: Plugin {

            name: "osm_custom";
            
            PluginParameter {
            
                name: "osm_custom.mapping.offline.directory"
                
                value: " "
            }
        }
        
Value consist of whitespace cause tiles are taken from a binary file and not from a folder, I'll tell you more about it later.

Now you can see your map.
