import QtQuick

Window {
    width: 450
    height: 450
    visible: true
    title: qsTr("Hello World")
    Rectangle {
        anchors.fill: parent
        Text {
            x: 0
            y: 0
            width: 312
            height: 262
            text: "fcfsfdcs";
            font.pixelSize: 50
        }
    }
}
