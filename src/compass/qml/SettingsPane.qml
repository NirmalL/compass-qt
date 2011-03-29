/*
 * Copyright (c) 2011 Nokia Corporation.
 */

import QtQuick 1.0
import "settings.js" as DB

BorderDialog {
    id: pane

    property bool autoNorthInMap
    property bool bearingTurnInTrackMode
    property bool satelliteMap
    property bool screenSaverInhibited
    property bool vibraEnabled

    function readSettings() {
        view.model = DB.readSettings()
    }

    function saveRouteCoordinate(time, latitude, longitude, accuracyInMeters) {
        // Commented out for now, when we can draw the route on top of the map
        // this code will be used

        //DB.insertRouteCoordinate(time, latitude, longitude, accuracyInMeters)
    }

    Text {
        x: 35; y: 45
        color: "#eea604"
        text: "Settings"
        font.bold: true
        font.pixelSize: 20
    }

    ListView {
        id: view

        property real itemHeight: pane.portrait ? view.height / 3
                                                  - view.spacing + 1
                                                : view.height
                                                  - view.spacing + 1
        property real itemWidth: view.width

        anchors {
            fill: parent
            leftMargin: pane.border.left
            rightMargin: pane.border.right
            topMargin: pane.border.top + 2
            bottomMargin: pane.border.bottom + 35
        }
        clip: true

        delegate: delegate
        snapMode: ListView.SnapOneItem
        spacing: 6

        Component {
            id: delegate

            Rectangle {
                width: view.itemWidth - scrollBar.width - 2; height: view.itemHeight

                color: "transparent"
                border.color: "#80EEA604"
                border.width: 2
                radius: 4

                Text {
                    id: nameText

                    anchors {
                        top: parent.top; topMargin: 10
                        left: parent.left; leftMargin: 19
                        right: parent.right; rightMargin: 19
                    }

                    text: name
                    color: "white"
                    wrapMode: Text.WordWrap
                    font.bold: true
                    font.pixelSize: 16
                }

                Rectangle {
                    property bool enabled: value

                    anchors {
                        top: nameText.bottom; topMargin: 6
                        left: nameText.left
                        right: nameText.right
                        bottom: parent.bottom; bottomMargin: 20
                    }

                    radius: 4
                    color: "#999999"

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        x: parent.width / 5

                        color: "white"
                        text: "On"

                        font.bold: true
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        x: parent.width / 6 * 4

                        color: "white"
                        text: "Off"

                        font.bold: true
                    }

                    Rectangle {
                        x: parent.enabled ? parent.width / 2 : 3
                        y: 3

                        Behavior on x {
                            PropertyAnimation { duration: 100 }
                        }

                        gradient: Gradient {
                            GradientStop { position: 0.0; color: "#909090" }
                            GradientStop { position: 1.0; color: "#707070" }
                        }

                        width: parent.width / 2 - 6
                        height: parent.height - 6
                        radius: 4
                        color: "#707070"
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: DB.toggleSetting(index)
                }
            }
        }

        Rectangle {
            id: scrollBar

            anchors.right: view.right
            y: view.visibleArea.yPosition * view.height
            width: 4; height: view.visibleArea.heightRatio * view.height
            opacity: 0.5
            radius: 2

            color: "#80EEA604"
        }
    }

    Rectangle {

        anchors {
            right: parent.right; rightMargin: 18
            bottom: parent.bottom; bottomMargin: 18
        }

        width: 79; height: 49
        radius: 10
        color: "#434343"

        Text {
            anchors.centerIn: parent
            text: "Close"
            color: "#eea604"
            font.bold: true
            font.pixelSize: 15
        }

        MouseArea {
            anchors.fill: parent
            onClicked: pane.shown = false
        }
    }

    Rectangle {

        anchors {
            left: parent.left; leftMargin: 18
            bottom: parent.bottom; bottomMargin: 18
        }

        width: 79; height: 49
        radius: 10
        color: "#434343"

        Text {
            anchors.centerIn: parent
            text: "Defaults"
            color: "#eea604"
            font.bold: true
            font.pixelSize: 15
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                DB.dropTables()
                view.model = DB.readSettings()
            }
        }
    }
}
