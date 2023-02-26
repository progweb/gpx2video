import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtPositioning 5.12
import QtLocation 5.12

Window {
    id: root
    visible: true
//    width: 640
//    height: 480
    title: qsTr("GPX2Video")

      property int margin: 11
//      width: mainLayout.implicitWidth + 2 * margin
//      height: mainLayout.implicitHeight + 2 * margin
//      minimumWidth: mainLayout.Layout.minimumWidth + 2 * margin
//      minimumHeight: mainLayout.Layout.minimumHeight + 2 * margin

	ColumnLayout {
        anchors.fill: parent
		anchors.margins: margin
        spacing: 10

		ComboBox {
			id: cb
			//currentIndex: 2
			Layout.minimumHeight: 30
//			Layout.preferredHeight: 30
//			Layout.fillHeight: true
			Layout.fillWidth: true

			valueRole: "value"

			model: ListModel {
//				id: cbItems
				ListElement { text: qsTr("OpenStreetMap I") }
				ListElement { text: qsTr("OpenStreetMap II") }
				ListElement { text: qsTr("OpenAerialMap") }
				ListElement { text: qsTr("Maps-For-Free") }
				ListElement { text: qsTr("OpenCycleMap") }
				ListElement { text: qsTr("OpenTopoMap") }
				ListElement { text: qsTr("Public Transport") }
				ListElement { text: qsTr("Google Maps") }
				ListElement { text: qsTr("Google Satellite") }
				ListElement { text: qsTr("Google Hybrid") }
				ListElement { text: qsTr("Virtual Earth") }
				ListElement { text: qsTr("Virtual Earth Satellite") }
				ListElement { text: qsTr("Virtual Earth Hybrid") }
				ListElement { text: qsTr("OSMC Trails") }
				ListElement { text: qsTr("IGN Essentiel Map") }
				ListElement { text: qsTr("IGN Essentiel Photo") }
			}
//			model: the_map.supportedMapTypes
			//width: 200
			//onCurrentIndexChanged: console.debug(cbItems.get(currentIndex).text + ", " + cbItems.get(currentIndex).value)
			//onCurrentIndexChanged: the_map.activeMapType = the_map.supportedMapTypes[ cbItems.get(currentIndex).value ]
			onCurrentIndexChanged: the_map.activeMapType = the_map.supportedMapTypes[currentIndex]
		}

    Map {
        id: the_map
//        anchors.fill: parent
//              implicitWidth: 200
//              implicitHeight: 60
              Layout.fillWidth: true
              Layout.fillHeight: true
        minimumZoomLevel: 1
        maximumZoomLevel: 20
        zoomLevel: 10
        center: QtPositioning.coordinate(48.86, 2.33) // Paris
		activeMapType: the_map.supportedMapTypes[0]

        plugin: Plugin {
			id: the_plugin
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
}

