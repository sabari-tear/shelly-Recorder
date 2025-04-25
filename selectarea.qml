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

    // Track if selection is confirmed
    property bool selectionConfirmed: false
    
    
    // Function to send coordinates to backend and close window
    function confirmSelection() {
        // Log values being sent to backend
        console.log("Sending to backend - Custom Area Selection:")
        console.log("Width: " + selectArea.width)
        console.log("Height: " + selectArea.height)
        console.log("X position: " + selectArea.x)
        console.log("Y position: " + selectArea.y)
        console.log("Screen number: 0")
        
        // Backend expects: width, height, x_offset, y_offset, screen_number
        backend.set_customarea(
            selectArea.width,
            selectArea.height,
            selectArea.x, 
            selectArea.y,
            0  // Using screen 0 by default
        )
        selectionConfirmed = true
        selectArea.close()
    }

    // Use a timer with longer interval for smoother performance
    Timer {
        id: resizeTimer
        interval: 16  // ~60fps
        repeat: false
        onTriggered: {
            if (resizeData.resizing) {
                selectArea.width = Math.max(selectArea.minimumWidth, resizeData.newWidth)
                selectArea.height = Math.max(selectArea.minimumHeight, resizeData.newHeight)
                
                if (resizeData.updateX)
                    selectArea.x += resizeData.deltaX
                
                if (resizeData.updateY)
                    selectArea.y += resizeData.deltaY
            }
        }
    }
    
    // Optimized resize data
    QtObject {
        id: resizeData
        property bool resizing: false
        property real startX: 0
        property real startY: 0
        property real startWidth: 0
        property real startHeight: 0
        property real newWidth: 0
        property real newHeight: 0
        property real deltaX: 0
        property real deltaY: 0
        property bool updateX: false
        property bool updateY: false
        property real previewX: 0
        property real previewY: 0
        property real previewWidth: 0
        property real previewHeight: 0
        
        function startResize(mouseX, mouseY, updateXPos, updateYPos) {
            resizing = true
            startX = mouseX
            startY = mouseY
            startWidth = selectArea.width
            startHeight = selectArea.height
            updateX = updateXPos
            updateY = updateYPos
            previewX = selectArea.x
            previewY = selectArea.y
            previewWidth = startWidth
            previewHeight = startHeight
            
            // Reset the preview container
            previewContainer.x = 0
            previewContainer.y = 0
            previewContainer.width = startWidth
            previewContainer.height = startHeight
        }
        
        function updateResize(mouseX, mouseY, fromLeft, fromTop, fromRight, fromBottom) {
            if (!resizing) return
            
            // Calculate deltas
            deltaX = mouseX - startX
            deltaY = mouseY - startY
            
            // Track relative changes to existing size
            var widthChange = 0
            var heightChange = 0
            var xChange = 0
            var yChange = 0
            
            // Calculate preview dimensions - store the deltas
            if (fromLeft) {
                widthChange = -deltaX
                xChange = deltaX
            } else if (fromRight) {
                widthChange = deltaX
            }
            
            if (fromTop) {
                heightChange = -deltaY
                yChange = deltaY
            } else if (fromBottom) {
                heightChange = deltaY
            }
            
            // Calculate new values ensuring minimums
            previewWidth = Math.max(selectArea.minimumWidth, startWidth + widthChange)
            previewHeight = Math.max(selectArea.minimumHeight, startHeight + heightChange)
            
            // Adjust x and y if width/height changed due to minimums
            if (fromLeft && widthChange < 0) {
                xChange = -(previewWidth - startWidth)
            }
            
            if (fromTop && heightChange < 0) {
                yChange = -(previewHeight - startHeight)
            }
            
            previewX = selectArea.x + xChange
            previewY = selectArea.y + yChange
            
            // Update the preview container position and size relative to window
            if (fromLeft) {
                previewContainer.x = xChange
                previewContainer.width = previewWidth
            } else if (fromRight) {
                previewContainer.width = previewWidth
            }
            
            if (fromTop) {
                previewContainer.y = yChange
                previewContainer.height = previewHeight
            } else if (fromBottom) {
                previewContainer.height = previewHeight
            }
            
            // Ensure preview is visible
            previewContainer.visible = true
        }
        
        function endResize() {
            if (!resizing) return
            resizing = false
            
            // Apply the final size/position
            selectArea.x = previewX
            selectArea.y = previewY
            selectArea.width = previewWidth
            selectArea.height = previewHeight
            
            // Reset the preview container
            previewContainer.x = 0
            previewContainer.y = 0
            previewContainer.width = selectArea.width
            previewContainer.height = selectArea.height
            
            // Hide preview
            previewContainer.visible = false
        }
    }

    Item {
        id: mainContainer
        anchors.fill: parent
        focus: true

        Rectangle {
            id: border_1
            anchors.fill: parent
            color: "#44000000"
            border.color: "white"
            border.width: 1
            radius: 0
            
            // Enable hardware acceleration
            layer.enabled: true
            layer.samples: 4

            MouseArea {
                id: dragArea
                anchors.fill: parent
                drag.target: parent
                drag.smoothed: true
                drag.filterChildren: true
                drag.threshold: 0  // Respond immediately to drag
                
                onPositionChanged: {
                    if (drag.active) {
                        selectArea.x += mouseX - startMouseX
                        selectArea.y += mouseY - startMouseY
                    }
                }
                
                property real startMouseX: 0
                property real startMouseY: 0
                
                onPressed: {
                    startMouseX = mouseX
                    startMouseY = mouseY
                }
            }
        }
        
        // Confirm button (positioned at bottom-right of selection)
        Rectangle {
            id: confirmButton
            width: 100
            height: 35
            color: confirmMouseArea.containsMouse ? "#5f56e8" : "#4f46d8"
            radius: 6
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            anchors.margins: 10
            z: 1001  // Above everything else
            
            Text {
                anchors.centerIn: parent
                text: "Confirm"
                color: "white"
                font.pixelSize: 14
                font.bold: true
            }
            
            MouseArea {
                id: confirmMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: confirmSelection()
            }
        }
        
        // Cancel button (positioned at bottom-left of selection)
        Rectangle {
            id: cancelButton
            width: 100
            height: 35
            color: cancelMouseArea.containsMouse ? "#f84d54" : "#e83e45"
            radius: 6
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.margins: 10
            z: 1001  // Above everything else
            
            Text {
                anchors.centerIn: parent
                text: "Cancel"
                color: "white"
                font.pixelSize: 14
                font.bold: true
            }
            
            MouseArea {
                id: cancelMouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked: selectArea.close()
            }
        }
        
        // Display current dimensions
        Rectangle {
            id: dimensions
            color: "#1c1e20"
            border.color: "#4f46d8"
            border.width: 1
            radius: 4
            height: 30
            width: dimensionsText.width + 20
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 15
            z: 1001
            
            Text {
                id: dimensionsText
                anchors.centerIn: parent
                text: selectArea.width + " × " + selectArea.height
                color: "white"
                font.pixelSize: 12
            }
        }
        
        // Preview container (visible only during resize)
        Rectangle {
            id: previewContainer
            visible: false
            border.color: "#4f46d8"  // Purple highlight
            border.width: 2
            color: "transparent"
            z: 999  // Keep on top
        }

        // Corner handles
        Rectangle {
            id: topLeftHandle
            width: 16
            height: 16
            color: "white"
            x: -8
            y: -8
            z: 1000  // Above preview

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, true, true)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, true, true, false, false)
                onReleased: resizeData.endResize()
            }
        }

        Rectangle {
            id: topRightHandle
            width: 16
            height: 16
            color: "white"
            x: parent.width - 8
            y: -8
            z: 1000  // Above preview

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, false, true)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, false, true, true, false)
                onReleased: resizeData.endResize()
            }
        }

        Rectangle {
            id: bottomLeftHandle
            width: 16
            height: 16
            color: "white"
            x: -8
            y: parent.height - 8
            z: 1000  // Above preview

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeBDiagCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, true, false)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, true, false, false, true)
                onReleased: resizeData.endResize()
            }
        }

        Rectangle {
            id: bottomRightHandle
            width: 16
            height: 16
            color: "white"
            x: parent.width - 8
            y: parent.height - 8
            z: 1000  // Above preview

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeFDiagCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, false, false)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, false, false, true, true)
                onReleased: resizeData.endResize()
            }
        }
        
        // Middle handles
        Rectangle {
            id: topMiddleHandle
            width: 16
            height: 16
            color: "white"
            x: parent.width/2 - 8
            y: -8
            z: 1000  // Above preview
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, false, true)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, false, true, false, false)
                onReleased: resizeData.endResize()
            }
        }
        
        Rectangle {
            id: bottomMiddleHandle
            width: 16
            height: 16
            color: "white"
            x: parent.width/2 - 8
            y: parent.height - 8
            z: 1000  // Above preview
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeVerCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, false, false)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, false, false, false, true)
                onReleased: resizeData.endResize()
            }
        }
        
        Rectangle {
            id: leftMiddleHandle
            width: 16
            height: 16
            color: "white"
            x: -8
            y: parent.height/2 - 8
            z: 1000  // Above preview
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, true, false)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, true, false, false, false)
                onReleased: resizeData.endResize()
            }
        }
        
        Rectangle {
            id: rightMiddleHandle
            width: 16
            height: 16
            color: "white"
            x: parent.width - 8
            y: parent.height/2 - 8
            z: 1000  // Above preview
            
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.SizeHorCursor
                preventStealing: true
                onPressed: resizeData.startResize(mouseX, mouseY, false, false)
                onPositionChanged: resizeData.updateResize(mouseX, mouseY, false, false, true, false)
                onReleased: resizeData.endResize()
            }
        }
        
        // Keep the handles positioned correctly when resizing
        Connections {
            target: mainContainer
            function onWidthChanged() {
                topRightHandle.x = mainContainer.width - 8
                bottomRightHandle.x = mainContainer.width - 8
                topMiddleHandle.x = mainContainer.width/2 - 8
                rightMiddleHandle.x = mainContainer.width - 8
            }
            function onHeightChanged() {
                bottomLeftHandle.y = mainContainer.height - 8
                bottomRightHandle.y = mainContainer.height - 8
                bottomMiddleHandle.y = mainContainer.height - 8
                leftMiddleHandle.y = mainContainer.height/2 - 8
                rightMiddleHandle.y = mainContainer.height/2 - 8
            }
        }

        // Close on Escape
        Keys.onReleased: {
            if (event.key === Qt.Key_Escape) {
                selectArea.close()
            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                confirmSelection()
            }
        }
    }
    
    // Handle window closing without confirmation

}
