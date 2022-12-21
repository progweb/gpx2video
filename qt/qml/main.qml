import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtPositioning 5.12
import QtLocation 5.12

Window {
    id: root
    visible: true
    width: 640
    height: 480
    title: qsTr("GPX2Video")

    Map {
        id: the_map
        anchors.fill: parent
        minimumZoomLevel: 1
        maximumZoomLevel: 20
        zoomLevel: 10
        center: QtPositioning.coordinate(48.86, 2.33) // Paris

        plugin: Plugin {
            name: "GPX2Video" //"esri" "mapbox" "osm" "here"

            PluginParameter {
                name: "mapPath"
                value: applicationDirPath+"/dianzi_gaode_ArcgisServerTiles/_alllayers"
            }

            PluginParameter {
                name: "format"
                value: "png"
            }
        }

        //MapParameter { }
    }
}

