import QtQuick 2.15
import QtQuick.Window 2.15
import QtQuick.Controls 2.15

Window {
    id: selectArea
    width: 320
    height: 200
    color: "transparent"
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Tool
    visible: true
    minimumWidth: 100
    minimumHeight: 100

    Component.onCompleted: {
        console.log("SelectArea window created")
        console.log("Window visible:", visible)
        console.log("Window flags:", flags)
    }

    Item {
        anchors.fill: parent
        focus: true

        Rectangle {
            id: border
            anchors.fill: parent
            color: "#44000000"
            border.color: "yellow"
            border.width: 2
            radius: 8

            MouseArea {
                id: dragArea
                anchors.fill: parent
                drag.target: parent
                onPressed: {
                    dragArea.drag.target = selectArea
                }
            }
        }

        // Resize handles
        Rectangle {
            id: topLeftHandle
            width: 10
            height: 10
            color: "yellow"
            radius: 5
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: -5
            anchors.topMargin: -5

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                onPressed: {
                    resizeHandle.pressed = true
                    resizeHandle.startX = mouseX
                    resizeHandle.startY = mouseY
                    resizeHandle.startWidth = selectArea.width
                    resizeHandle.startHeight = selectArea.height
                }
                onPositionChanged: {
                    if (resizeHandle.pressed) {
                        var deltaX = mouseX - resizeHandle.startX
                        var deltaY = mouseY - resizeHandle.startY
                        selectArea.width = Math.max(selectArea.minimumWidth, resizeHandle.startWidth - deltaX)
                        selectArea.height = Math.max(selectArea.minimumHeight, resizeHandle.startHeight - deltaY)
                        selectArea.x += deltaX
                        selectArea.y += deltaY
                    }
                }
                onReleased: {
                    resizeHandle.pressed = false
                }
            }
        }

        Rectangle {
            id: topRightHandle
            width: 10
            height: 10
            color: "yellow"
            radius: 5
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.rightMargin: -5
            anchors.topMargin: -5

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                onPressed: {
                    resizeHandle.pressed = true
                    resizeHandle.startX = mouseX
                    resizeHandle.startY = mouseY
                    resizeHandle.startWidth = selectArea.width
                    resizeHandle.startHeight = selectArea.height
                }
                onPositionChanged: {
                    if (resizeHandle.pressed) {
                        var deltaX = mouseX - resizeHandle.startX
                        var deltaY = mouseY - resizeHandle.startY
                        selectArea.width = Math.max(selectArea.minimumWidth, resizeHandle.startWidth + deltaX)
                        selectArea.height = Math.max(selectArea.minimumHeight, resizeHandle.startHeight - deltaY)
                        selectArea.y += deltaY
                    }
                }
                onReleased: {
                    resizeHandle.pressed = false
                }
            }
        }

        Rectangle {
            id: bottomLeftHandle
            width: 10
            height: 10
            color: "yellow"
            radius: 5
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.leftMargin: -5
            anchors.bottomMargin: -5

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                onPressed: {
                    resizeHandle.pressed = true
                    resizeHandle.startX = mouseX
                    resizeHandle.startY = mouseY
                    resizeHandle.startWidth = selectArea.width
                    resizeHandle.startHeight = selectArea.height
                }
                onPositionChanged: {
                    if (resizeHandle.pressed) {
                        var deltaX = mouseX - resizeHandle.startX
                        var deltaY = mouseY - resizeHandle.startY
                        selectArea.width = Math.max(selectArea.minimumWidth, resizeHandle.startWidth - deltaX)
                        selectArea.height = Math.max(selectArea.minimumHeight, resizeHandle.startHeight + deltaY)
                        selectArea.x += deltaX
                    }
                }
                onReleased: {
                    resizeHandle.pressed = false
                }
            }
        }

        Rectangle {
            id: bottomRightHandle
            width: 10
            height: 10
            color: "yellow"
            radius: 5
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.rightMargin: -5
            anchors.bottomMargin: -5

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                onPressed: {
                    resizeHandle.pressed = true
                    resizeHandle.startX = mouseX
                    resizeHandle.startY = mouseY
                    resizeHandle.startWidth = selectArea.width
                    resizeHandle.startHeight = selectArea.height
                }
                onPositionChanged: {
                    if (resizeHandle.pressed) {
                        var deltaX = mouseX - resizeHandle.startX
                        var deltaY = mouseY - resizeHandle.startY
                        selectArea.width = Math.max(selectArea.minimumWidth, resizeHandle.startWidth + deltaX)
                        selectArea.height = Math.max(selectArea.minimumHeight, resizeHandle.startHeight + deltaY)
                    }
                }
                onReleased: {
                    resizeHandle.pressed = false
                }
            }
        }

        // Resize handle state
        QtObject {
            id: resizeHandle
            property bool pressed: false
            property int startX: 0
            property int startY: 0
            property int startWidth: 0
            property int startHeight: 0
        }

        // Close on Escape
        Keys.onReleased: if (event.key === Qt.Key_Escape) selectArea.close()
    }
}
