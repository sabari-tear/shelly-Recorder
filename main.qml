import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Effects
import Qt.labs.platform

Window {
    id: root
    width: 400
    height: 550
    color: "transparent"
    minimumHeight: 500
    maximumHeight: 500
    minimumWidth: 400
    maximumWidth: 400
    flags: Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint
    visible: true
    Component.onCompleted: {
        select_full_or_area.full_state = true
        backend.set_fullscreen()
        backend.set_customlocation(path.text)
        fps_24_30_60.offx = 0
        backend.get_fps(24)
        backend.get_quality(50)
        backend.set_audio(0)
    }

    Rectangle {
        id:win
        anchors.fill: parent
        color: "#0e0f12"
        radius: 20
        border.color: "#1c1e20"
        Loader {
            id: areaSelector
            anchors.fill: parent
            active: false
            source: "selectarea.qml"
            onLoaded: {
                item.visible = true
            }
        }

        Rectangle {
            id: header
            anchors.top: parent.top
            anchors.topMargin: 15
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50
            color: "transparent"
            Text {
                id: app_name
                text: "Shelly Recorder"
                color:"#FFFFFF"
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.left: parent.left
                anchors.leftMargin: 25
                font.pointSize: 15
            }
            Rectangle {
                id: close_button
                width: 50
                height: 50
                color: "transparent"
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 15

                property color xColor: mouseArea.containsMouse ? "#f84d54" : "#252729"

                // Diagonal line 1
                Rectangle {
                    width: parent.width / 2
                    height: 3
                    color: close_button.xColor
                    rotation: 45
                    anchors.centerIn: parent
                    radius: 30
                    
                    Behavior on color {
                        ColorAnimation {
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                // Diagonal line 2
                Rectangle {
                    width: parent.width / 2
                    height: 3
                    color: close_button.xColor
                    rotation: -45
                    anchors.centerIn: parent
                    radius: 30
                    
                    Behavior on color {
                        ColorAnimation {
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        console.log("X clicked")
                        root.close()
                    }
                }
            }
            MouseArea {
                anchors.fill: parent
                onPressed: {
                    root.startSystemMove()
                }
            }
        }

        Rectangle {
            id: quality
            anchors.left: parent.left
            anchors.leftMargin: 17
            anchors.top: fps_selector.bottom
            anchors.topMargin: 15
            anchors.right: parent.right
            anchors.rightMargin: 17
            height: 50
            color:"transparent"
            border.color: "#1c1e20"
            radius: 10
            Text {
                id: quality_text
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: parent.left
                anchors.leftMargin: 15
                text: "Quality"
                color:"#FFFFFF"
                font.pointSize: 12
            }
            Rectangle {
                id: slider
                anchors.left: quality_text.right
                anchors.right: parent.right
                anchors.leftMargin: 30
                anchors.rightMargin: 30
                anchors.verticalCenter: parent.verticalCenter
                color: "#1c1e20"
                height: 5
                radius: 30

                property int indi_x: 0

                Rectangle {
                    id: indicator
                    width: 15
                    height: 15
                    anchors.verticalCenter: parent.verticalCenter
                    radius: 100
                    color: "#4f46d8"
                    x: slider.indi_x

                    MouseArea {
                        id: mover_indi
                        anchors.fill: parent
                        drag.target: indicator
                        drag.axis: Drag.XAxis
                        drag.minimumX: 0
                        drag.maximumX: slider.width - indicator.width
                        onPositionChanged: {
                            slider.indi_x = indicator.x
                        }
                        onReleased: {
                            var quality = Math.round((indicator.x / (slider.width - indicator.width)) * 100)
                            backend.get_quality(quality)
                        }
                    }

                    Behavior on x {
                        NumberAnimation {
                            duration: 100
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
            }

        }

        Rectangle {
            id: fps_selector
            anchors.top: audio_setting.bottom
            anchors.topMargin: 15
            color: "#0e0f12"
            anchors.left: parent.left
            anchors.leftMargin: 17
            anchors.right: parent.right
            anchors.rightMargin: 17
            width: parent.width -17-17
            height: 50
            //border.color: "#1c1e20"
            Text {
                id: fps_text
                text: "FPS"
                anchors.left:parent.left
                anchors.top: parent.top
                anchors.leftMargin: 15
                anchors.topMargin: 15
                font.pointSize: 12
                color: "#FFFFFF"
            }
            Rectangle {
                id: fps_24_30_60
                anchors.left: fps_text.right
                width: 60+60+60
                anchors.leftMargin: 15
                color: "#0e0f12"
                anchors.top: parent.top
                height: 50
                border.color: "#1c1e20"
                radius: 10
                property int offx : 0
                Rectangle {
                    id: mover
                    width: 60
                    height: 50
                    color: "#4f46d8"
                    radius: 10
                    x: fps_24_30_60.offx
                    Behavior on x {
                        NumberAnimation {
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                }
                Rectangle {
                    id: fps_24
                    anchors.left:parent.left
                    height: 50
                    width: 60
                    color: "transparent"
                    radius: 10
                    Text {
                        id: fps_24_text
                        text: "24"
                        anchors.centerIn: parent
                        color:"#FFFFFF"
                        font.pointSize: 12
                    }
                    MouseArea {
                        id: fps_24_click
                        anchors.fill: parent
                        onClicked: {
                            fps_24_30_60.offx = 0
                            backend.get_fps(24)
                        }
                    }
                }
                Rectangle {
                    id: fps_30
                    anchors.left: fps_24.right
                    height: 50
                    width: 60
                    color: "transparent"
                    radius: 10
                    Text {
                        id: fps_30_text
                        text: "30"
                        anchors.centerIn: parent
                        color:"#FFFFFF"
                        font.pointSize: 12
                    }
                    MouseArea {
                        id: fps_30_click
                        anchors.fill: parent
                        onClicked: {
                            fps_24_30_60.offx = 60
                            backend.get_fps(30)
                        }
                    }
                }
                Rectangle {
                    id: fps_60
                    anchors.left:fps_30.right
                    height: 50
                    width: 60
                    color: "transparent"
                    radius: 10
                    Text {
                        id: fps_60_text
                        text: "60"
                        anchors.centerIn: parent
                        color:"#FFFFFF"
                        font.pointSize: 12
                    }
                    MouseArea {
                        id: fps_60_click
                        anchors.fill: parent
                        onClicked: {
                            fps_24_30_60.offx = 120
                            backend.get_fps(60)
                        }
                    }
                }
            }
        }

        Rectangle {
            id: select_full_or_area
            property bool full_state: true
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 80
            color:"transparent"
            height: 50
            Rectangle {
                id: fullscreen
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: 17
                color: select_full_or_area.full_state ? "#4f46d8" : (fullscreen_mouse.containsMouse ? "#252729" : "#121516")
                width: 170
                height: 50
                radius: 30
                Text {
                    id: fullscreen_text
                    anchors.centerIn: parent
                    text: qsTr("Fullscreen")
                    color: "#FFFFFF"
                    font.pointSize: 12
                }
                MouseArea {
                    id: fullscreen_mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        select_full_or_area.full_state = true
                        backend.set_fullscreen()
                        if (areaSelector.active) {
                            areaSelector.active = false
                        }
                    }
                }

                border.color: "#1c1e20"
                Behavior on color {
                    ColorAnimation {
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }
            }

            Rectangle {
                id: select_area
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.rightMargin: 17
                color: !select_full_or_area.full_state ? "#4f46d8" : (select_area_mouse.containsMouse ? "#252729" : "#121516")
                width: 170
                height: 50
                radius: 30
                Text {
                    id: selcet_area_text
                    anchors.centerIn: parent
                    text: qsTr("Select Area")
                    color: "#FFFFFF"
                    font.pointSize: 12
                }
                MouseArea {
                    id: select_area_mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        select_full_or_area.full_state = false
                        areaSelector.active = true
                    }
                }

                border.color: "#1c1e20"
                Behavior on color {
                    ColorAnimation {
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }

        Rectangle {
            id: output_path
            anchors.top: select_full_or_area.bottom
            anchors.left: parent.left
            anchors.topMargin: 15
            anchors.leftMargin: 17
            color: "#0e0f12"
            width: 300
            height: 50
            radius: 10
            border.color: "#1c1e20"
            clip: true

            Text {
                id: path
                text: "C:/Users/LENOVO/Videos/out.mp4"
                font.pointSize: 12
                color: "#FFFFFF"
                anchors.left: parent.left
                anchors.leftMargin: 15
                anchors.right: parent.right
                anchors.rightMargin: 15
                anchors.verticalCenter: parent.verticalCenter
                elide: Text.ElideMiddle
                horizontalAlignment: Text.AlignLeft
            }
        }

        Rectangle {
            id: browse
            anchors.top:select_full_or_area.bottom
            anchors.right: parent.right
            anchors.rightMargin: 17
            anchors.left: output_path.right
            anchors.leftMargin: 15
            anchors.topMargin: 15
            color: saveDialog.visible ? "#4f46d8" : (browse_mouse.containsMouse ? "#252729" : "#121516")
            height: 50
            width: 50
            border.color: "#1c1e20"
            radius: 10

            Behavior on color {
                ColorAnimation {
                    duration: 300
                    easing.type: Easing.InOutQuad
                }
            }

            Text {
                id: browse_text
                text: "..."
                color: "#FFFFFF"
                anchors.centerIn: parent
                font.pointSize: 12
            }
            MouseArea {
                id: browse_mouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    saveDialog.open()
                }
            }
            FileDialog {
                id: saveDialog
                title: "Choose file to save"
                fileMode: FileDialog.SaveFile
                nameFilters: ["MP4 Files (*.mp4)"]
                defaultSuffix: "mp4"
                onAccepted: {
                    path.text = saveDialog.file.toString().replace("file:///", "")
                    backend.set_customlocation(path.text)
                }
            }
        }

        Rectangle {
            id: audio_setting
            anchors.top: output_path.bottom
            color: "#0e0f12"
            anchors.left: parent.left
            anchors.leftMargin: 17
            anchors.right: parent.right
            anchors.rightMargin: 17
            height: 50
            radius: 10
            anchors.topMargin: 15

            Text {
                id: audio_text
                text: "Audio"
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.leftMargin: 15
                anchors.topMargin: 15
                font.pointSize: 12
                color: "#FFFFFF"
            }

            Rectangle {
                id: audio_bool
                width: 80
                height: 50
                anchors.left: audio_text.right
                anchors.leftMargin: 15
                anchors.top: parent.top
                radius: 30
                color: "#0e0f12"
                border.color: "#1c1e20"
                property bool audioEnabled: false

                Text {
                    id: audio_off
                    visible: !audio_bool.audioEnabled
                    anchors.right: parent.right
                    anchors.rightMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    text: "off"
                    font.pointSize: 12
                    color: "#FFFFFF"
                }

                Text {
                    id: audio_on
                    visible: audio_bool.audioEnabled
                    anchors.left: parent.left
                    anchors.leftMargin: 15
                    anchors.verticalCenter: parent.verticalCenter
                    text: "on"
                    font.pointSize: 12
                    color: "#FFFFFF"
                }

                Rectangle {
                    id: audio_trigger
                    width: 30
                    height: 30
                    radius: 80
                    anchors.verticalCenter: parent.verticalCenter
                    property real onX: parent.width - width - 10
                    property real offX: 10
                    x: audio_bool.audioEnabled ? onX : offX
                    color: audio_bool.audioEnabled ? "#4f46d8" : "#1c1e20"

                    Behavior on x {
                        NumberAnimation {
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }

                    Behavior on color {
                        ColorAnimation {
                            duration: 300
                            easing.type: Easing.InOutQuad
                        }
                    }
                }


                MouseArea {
                    id: audio_xor
                    anchors.fill: parent
                    onClicked: {
                        backend.set_audio(!audio_bool.audioEnabled);
                        audio_bool.audioEnabled = !audio_bool.audioEnabled;
                        // Reset dropdown visibility when toggling audio
                        if (audio_bool.audioEnabled) {
                            audio_devices.listVisible = false;
                        }
                    }
                }
            }
            Rectangle {
                id: audio_devices
                anchors.left: audio_bool.right
                anchors.leftMargin: 15
                anchors.right: parent.right
                anchors.top: parent.top
                height: 50
                color: "#0e0f12"
                border.color: "#1c1e20"
                radius: 10
                property bool listVisible: false
                property var audioDevices: backend ? backend.getAudioDevices() : []
                property string lastSelectedDevice: "Select..."
                visible : audio_bool.audioEnabled

                Behavior on visible {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }

                Behavior on opacity {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }

                opacity: audio_bool.audioEnabled ? 1 : 0

                Rectangle {
                    id: dropdown
                    width: parent.width
                    anchors.verticalCenter: parent.verticalCenter
                    height: 30
                    color: "transparent"

                    Text {
                        id: selectedText
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 15
                        anchors.right: parent.right
                        anchors.rightMargin: 15
                        text: audio_devices.lastSelectedDevice
                        color: "white"
                        font.pointSize: 12
                        elide: Text.ElideRight
                        horizontalAlignment: Text.AlignLeft
                    }
                    MouseArea {
                        anchors.fill: parent
                        onClicked:{

                            audio_devices.listVisible = !audio_devices.listVisible
                        }
                    }
                }

                Rectangle {
                    id: list_view_container
                    width: audio_devices.width
                    height: 80
                    y: audio_devices.height
                    visible: audio_devices.listVisible
                    clip: true
                    radius:10
                    color:"transparent"
                    ListView {
                        id: listView
                        width: parent.width
                        height: 80
                        anchors.fill: parent
                        clip: true
                        model: backend ? backend.getAudioDevices() : []
                        delegate: Rectangle {
                            width: listView.width
                            height: 40
                            color: "#121516"
                            Text {
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 15
                                text: modelData
                                color: "#FFFFFF"
                                font.pixelSize: 12
                            }
                            Rectangle {
                                anchors.top: parent.top
                                height: 1
                                anchors.left: parent.left
                                anchors.right: parent.right
                                color: "#1c1e20"
                            }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    selectedText.text = modelData
                                    audio_devices.lastSelectedDevice = modelData
                                    backend.set_audioDeviceName(modelData)
                                    audio_devices.listVisible = false
                                }
                            }
                        }
                    }
                }
            }
        }

        Rectangle {
            id: bottons
            anchors.left: parent.left
            anchors.leftMargin: 17
            anchors.right: parent.right
            anchors.rightMargin: 17
            anchors.top: quality.bottom
            anchors.topMargin: 15
            height: 50
            color: "transparent"
            property bool started: false
            Rectangle {
                id: recording_id
                height: 50
                width: 200
                radius: 30
                color: !bottons.started ? "#44b653":"#f84d54"
                //border.color: "#f76b71"
                Text {
                    id: recording_status
                    anchors.centerIn: parent
                    text: !bottons.started ? qsTr("Start recording") : "Stop recording"
                    color: "#FFFFFF"
                    font.pointSize: 12
                }
                MouseArea {
                    id: record_space
                    anchors.fill: parent
                    onClicked: {
                        bottons.started = !bottons.started
                        if (bottons.started) {
                            another_option.pause = true
                            backend.start_record()
                        } else {
                            backend.stop_record()
                        }
                    }
                }
                Behavior on color {
                    ColorAnimation {
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            Rectangle {
                id: another_option
                height: 50
                anchors.right: parent.right
                width: 150
                radius: 30
                property bool pause: true
                color: bottons.started ? (another_option.pause ?
                    (pause_resume.containsMouse ? "#252729" : "#121516") :
                    "#4f46d8") : "#121516"
                border.color: "#1c1e20"
                Text {
                    id: another_option_text
                    anchors.centerIn: parent
                    text: bottons.started ? (another_option.pause ? "Pause" : "Resume") : "Pause"
                    color: "#FFFFFF"
                    font.pointSize: 12
                }
                MouseArea {
                    id: pause_resume
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked:{
                        if (another_option.pause) {
                            backend.pause_record()
                        } else {
                            backend.resume_record()
                        }
                        another_option.pause = !another_option.pause
                    }
                }
                opacity: bottons.started ? 1:0
                visible: opacity>0
                Behavior on opacity {
                    NumberAnimation {
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }
                Behavior on color {
                    ColorAnimation {
                        duration: 300
                        easing.type: Easing.InOutQuad
                    }
                }
            }
        }
        // Rectangle {
        //     id: log_text
        //     anchors.top:bottons.bottom
        //     anchors.topMargin: 15
        //     anchors.left: parent.left
        //     anchors.right: parent.right
        //     anchors.leftMargin: 17
        //     anchors.rightMargin: 17
        //     anchors.bottom: parent.bottom
        //     anchors.bottomMargin: 17
        //     color:  "transparent"
        //     border.color: "#1c1e20"
        //     radius: 10
        //     clip: true

        //     // Maintain a property for logs
        //     property string logText: ""

        //     // Connect to backend log signals
        //     Connections {
        //         target: backend
        //         function onLogMessage(message) {
        //             // Add new message to log
        //             log_text.logText += message + "\n";
        //             // Auto-scroll to bottom
        //             logArea.cursorPosition = logArea.length
        //             logArea.ensureVisible(logArea.cursorPosition)
        //         }
        //     }

        //     ScrollView {
        //         id: logScrollView
        //         anchors.fill: parent
        //         anchors.margins: 10
        //         clip: true

        //         ScrollBar.vertical.policy: ScrollBar.AlwaysOn
        //         ScrollBar.horizontal.policy: ScrollBar.AsNeeded

        //         TextArea {
        //             id: logArea
        //             width: parent.width
        //             height: parent.height
        //             text: log_text.logText
        //             color: "#CCCCCC"
        //             font.family: "Consolas"
        //             font.pixelSize: 12
        //             readOnly: true
        //             wrapMode: TextEdit.Wrap
        //             selectByMouse: true

        //             background: Rectangle {
        //                 color: "#121516"
        //                 radius: 5
        //             }
        //         }
        //     }

        //     // Clear button
        //     Rectangle {
        //         id: clearButton
        //         width: 60
        //         height: 24
        //         radius: 4
        //         color: clearBtnMouseArea.containsMouse ? "#4f46d8" : "#252729"
        //         anchors.top: parent.top
        //         anchors.right: parent.right
        //         anchors.margins: 8
        //         z: 1

        //         Text {
        //             anchors.centerIn: parent
        //             text: "Clear"
        //             color: "white"
        //             font.pixelSize: 10
        //         }

        //         MouseArea {
        //             id: clearBtnMouseArea
        //             anchors.fill: parent
        //             hoverEnabled: true
        //             onClicked: {
        //                 log_text.logText = ""
        //             }
        //         }

        //         Behavior on color {
        //             ColorAnimation {
        //                 duration: 150
        //                 easing.type: Easing.InOutQuad
        //             }
        //         }
        //     }
        //}
    }

}
