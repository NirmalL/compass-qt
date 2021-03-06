/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

import QtQuick 1.1
import com.nokia.meego 1.0
import "../common"

Page {
    id: mapView

    orientationLock: PageOrientation.LockPortrait

    tools: ToolBarLayout {
        ToolIcon {
            id: infoViewButton

            iconSource: "../images/info.png"
            onClicked: {
                if (mapView.pageStack.currentPage === mapView) {
                    mapView.pageStack.push(Qt.resolvedUrl("InfoView.qml"));
                }
            }
        }

        ToolIcon {
            id: gpsIndicator

            iconSource: "../images/center.png"
            onClicked: map.panToCoordinate(map.hereCenter)
        }

        ToolIcon {
            id: compassMode

            property bool checked: false

            iconSource: checked ? "../images/compass_selected.png"
                                : "../images/compass.png"
            onClicked: { checked = !checked; }
        }

        ToolIcon {
            id: settingsToolButton

            property bool checked: false

            iconSource: checked ? "../images/settings_selected.png"
                                : "../images/settings.png"
            onClicked: { checked = !checked; }
        }
    }

    Component.onCompleted: {
        mobility.startSensors();

        var initialCoordinate = settingsPane.readSettings();

        map.mapCenter = initialCoordinate;
        map.hereCenter.longitude = initialCoordinate.longitude;
        map.hereCenter.latitude = initialCoordinate.latitude;

        settingsPane.readRoute(map);
    }

    Mobility {
        id: mobility

        onCompass: {
            // Find if there is already calibration view open in page stack
            var calibrationView = mapView.pageStack.find(function(page) {
                return page.objectName === "calibrationView";
            });

            // If it does not exist and it should be shown, create and push it
            // to the stack.
            if (calibrationLevel < 1.0 && calibrationView === null) {
                calibrationView = mapView.pageStack.push(
                            Qt.resolvedUrl("CalibrationView.qml"));
            }

            // If the calibration view exists, set the calibration level to it.
            if (calibrationView !== null) {
                calibrationView.setCalibrationLevel(calibrationLevel);
            }

            compass.azimuth = azimuth;
        }

        onPosition: {
            console.log("Position: " + coordinate.latitude +
                        ", " + coordinate.longitude +
                        " alt " + coordinate.altitude + " m" +
                        " accuracy " + accuracyInMeters + " m");

            settingsPane.saveInitialCoordinate(coordinate);

            if (settingsPane.trackingOn && accuracyInMeters < 75 && altitudeValid) {
                // The recording is on and the position is accurate enough.
                var newRoute = settingsPane.saveRouteCoordinate(coordinate, time, accuracyInMeters);
                map.addRoutePoint(coordinate, newRoute);
            }

            if (mapView.status === PageStatus.Active) {
                map.moveHereToCoordinate(coordinate, accuracyInMeters, true);
            }
            else {
                map.moveHereToCoordinate(coordinate, accuracyInMeters, false);
            }
        }
    }

    Image {
        id: background

        anchors.fill: parent

        source: "../images/compass_back.png"
        fillMode: Image.Tile
    }

    PannableMap {
        id: map

        anchors.fill: parent
        satelliteMap: settingsPane.satelliteMap
    }

    CompassPlate {
        id: compass

        x: 34

        // Turns automatically the bearing to the map north
        onUserRotatedCompass: compass.bearing = -compass.rotation
        bearingRotable: true
    }

    SettingsPane {
        id: settingsPane

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom; bottomMargin: mapView.tools.height
        }

        opacity: settingsToolButton.checked ? 1.0 : 0.0
        onRouteCleared: map.clearRoute();
    }

    states: [
        State {
            when: !compassMode.checked
            name: "MapMode"
            PropertyChanges { target: map; opacity: 1.0 }
            PropertyChanges {
                target: compass
                opacity: 1.0
                width: 0.483607 * height; height: 0.40625 * mapView.height
                movable: true
                compassRotable: true
            }
        },
        State {
            when: compassMode.checked
            name: "TrackMode"
            PropertyChanges { target: map; opacity: 0 }
            PropertyChanges {
                target: compass
                opacity: 1.0; rotation: 0
                width: 0.483607 * height
                height: mapView.height - mapView.tools.height
                x: (mapView.width - width) / 2; y: 0;
                movable: false
                compassRotable: false
            }
        }
    ]

    transitions: Transition {
        SequentialAnimation {
            ScriptAction {
                script: compass.needleBehavior.enabled = false;
            }
            ParallelAnimation {
                PropertyAnimation {
                    properties: "x,y,width,height,opacity"
                    duration: 500
                    easing.type: Easing.InOutCirc
                }

                RotationAnimation {
                    property: "rotation"
                    duration: 500
                    easing.type: Easing.InOutCirc
                    direction:  RotationAnimation.Shortest
                }
            }
            ScriptAction {
                script: compass.needleBehavior.enabled = true;
            }
        }
    }
}
