import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Qt.labs.platform

Window {
    width: 450
    height: 450
    visible: true
    title: qsTr("Hello World")

    Rectangle {
        id : main
        visible: true
        anchors.fill: parent
        color: "#000000"

        Rectangle {
            id :  fullscreen
            x: 31
            y: 41
            width: 100
            height: 30
            color: "#2a2a2a"
            Text {
                anchors.centerIn: parent
                text: "Fullscreen"
                color: "#FFFFFF"

            }

            MouseArea {
                id: mouseArea_fullscreen
                anchors.fill: parent
                onClicked: {
                    backend.set_fullscreen()
                    selectAreaLoader.active=false
                }
            }
            radius: 20
            Component.onCompleted: {
                console.log("QML Loaded")
                backend.set_fullscreen()
            }
        }

        Rectangle {
            id : select_area
            x: 183
            y: 41
            width: 100
            height: 30
            color: "#2a2a2a"
            Text {
                anchors.centerIn: parent
                text: "Select Area"
                color: "#FFFFFF"

            }

            MouseArea {
                id: mouseArea_select_area
                anchors.fill: parent
                onClicked: {
                    console.log("Select Area button clicked")
                    selectAreaLoader.active = true
                    console.log("Loader active state:", selectAreaLoader.active)
                    console.log("Loader status:", selectAreaLoader.status)
                    console.log("Loader source:", selectAreaLoader.source)
                }
            }
            radius: 20
        }

        Loader {
            id: selectAreaLoader
            active: false
            source: "qrc:/selectarea.qml"
            onStatusChanged: {
                console.log("Loader status changed to:", status)
                if (status === Loader.Error) {
                    console.log("Loader error:", selectAreaLoader.sourceComponent.errorString())
                }
            }
        }

        property string savePath: ""

        Rectangle {
            id: path
            x: 31
            y: 92
            width: 200
            height: 30
            color: "#2a2a2a"
            radius: 20

            TextInput {
                id: textInput
                anchors.centerIn: parent
                text: qsTr("C:/Users/LENOVO/Videos/out.mp4")
                color: "#ffffff"
                width: parent.width - 20
            }
            Component.onCompleted: {
                backend.set_customlocation(textInput.text)
            }
        }

        Rectangle {
            id: browse_path
            x: 253
            y: 92
            width: 30
            height: 30
            color: "#2a2a2a"

            Text {
                anchors.centerIn: parent
                text: "..."
                color: "#ffffff"
            }

            MouseArea {
                anchors.fill: parent
                onClicked: saveDialog.open()
            }

            FileDialog {
                id: saveDialog
                title: "Choose file to save"
                fileMode: FileDialog.SaveFile
                nameFilters: ["MP4 Files (*.mp4)"]
                defaultSuffix: "mp4"
                onAccepted: {
                    textInput.text = saveDialog.file.toString().replace("file:///", "")
                    console.log("Save file path: " + textInput.text)
                    backend.set_customlocation(textInput.text)
                }
            }
        }

        Rectangle {
            id: system_tray
            x: 31
            y: 149
            width:30
            height: 30
            color: "#2a2a2a"
            Text {
                id: no_system
                anchors.centerIn: parent
                color: "#FFFFFF"
                text: "no"
                visible:true
            }
            Text {
                id: yes_system
                anchors.centerIn: parent
                color: "#FFFFFF"
                text: "yes"
                visible:false
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (no_system.visible) {
                        yes_system.visible=true;
                        no_system.visible=false;
                    }
                    else {
                        no_system.visible=true;
                        yes_system.visible=false;
                    }
                }
            }
        }

        Rectangle {
            id: need_audio
            x: 31
            y: 204
            width: no_audio.width+yes_audio.width
            height: 30
            color: "#2a2a2a"
            Text {
                id: no_audio
                anchors.centerIn: parent
                color: "#FFFFFF"
                text: "audio_ no"
                visible:true
            }
            Text {
                id: yes_audio
                anchors.centerIn: parent
                color: "#FFFFFF"
                text: "audio_yes"
                visible:false
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (no_audio.visible) {
                        yes_audio.visible=true;
                        backend.set_audio(1);
                        no_audio.visible=false;
                    }
                    else {
                        no_audio.visible=true;
                        backend.set_audio(0);
                        yes_audio.visible=false;
                    }
                }
            }
        }

        Rectangle {
            id: select_fps
            width:250
            height: 30
            color:"#2a2a2a"
            property string curr: "24"
            x: 193
            y: 204
            Row {
                leftPadding: 20
                spacing:10
                Rectangle {
                    id: curr_fps
                    width: 90
                    height: 30
                    color: "#1a1a1a"
                    Text {
                        anchors.centerIn: parent
                        text: "selected_fps : " + select_fps.curr
                        color: "#FFFFFF"
                        visible: true
                    }
                    Component.onCompleted: backend.get_fps(24)
                }

                Rectangle {
                    id : select_24
                    width: 30
                    height: 30
                    color: "#3a3a3a"
                    Text {
                        anchors.centerIn: parent
                        text: "24"
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: mouseArea_select_24
                        anchors.fill: parent
                        onClicked: {
                            backend.get_fps(24);
                            select_fps.curr="24";
                        }
                    }
                    radius: 2
                }
                Rectangle {
                    id : select_30
                    width: 30
                    height: 30
                    color: "#3a3a3a"
                    Text {
                        anchors.centerIn: parent
                        text: "30"
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: mouseArea_select_30
                        anchors.fill: parent
                        onClicked: {
                            backend.get_fps(30);
                            select_fps.curr="30";
                        }
                    }
                    radius: 2
                }
                Rectangle {
                    id : select_60
                    width: 30
                    height: 30
                    color: "#3a3a3a"
                    Text {
                        anchors.centerIn: parent
                        text: "60"
                        color: "#FFFFFF"
                    }

                    MouseArea {
                        id: mouseArea_select_60
                        anchors.fill: parent
                        onClicked: {
                            backend.get_fps(60);
                            select_fps.curr="60";
                        }
                    }
                    radius: 6
                }
            }
        }

        Rectangle {
            id: slider
            x: 31
            y: 259
            width: 200
            height: 60
            color: "#2a2a2a"

            property int minValue: 0
            property int maxValue: 100
            property int currentValue: 0

            Rectangle {
                id: slider_text
                width: parent.width
                height: 20
                color:"#2a2a2a"
                Text {
                    id: slider_text_comp
                    anchors.left: parent.left
                    anchors.leftMargin: 8
                    anchors.verticalCenter: parent.verticalCenter
                    text: "quality: " + slider.currentValue
                    color: "#FFFFFF"
                }
            }

            Rectangle {
                id: slider_cmp
                x: 13
                y: 39
                width: 175
                height: 5
                color: "#2a2F2F"
                radius: 2

                Rectangle {
                    id: slide_clip
                    width: 10
                    height: 10
                    radius: 5
                    color: "#FFFFFF"
                    anchors.verticalCenter: parent.verticalCenter
                    x: (slider.currentValue - slider.minValue) / (slider.maxValue - slider.minValue) * (slider_cmp.width - width)

                    MouseArea {
                        id: drag_area
                        anchors.fill: parent
                        drag.target: parent
                        drag.axis: Drag.XAxis
                        drag.minimumX: 0
                        drag.maximumX: slider_cmp.width - slide_clip.width

                        onPositionChanged: {
                            slider.currentValue = Math.round((slide_clip.x / (slider_cmp.width - slide_clip.width)) * (slider.maxValue - slider.minValue) + slider.minValue);
                        }
                        onReleased: backend.get_quality(slider.currentValue);
                    }
                }
            }
            Component.onCompleted: backend.get_quality(slider.currentValue);
        }

        Rectangle {
            id : start_recording
            x: 31
            y: 356
            width: 100
            height: 30
            color: "#2a2a2a"
            Text {
                anchors.centerIn: parent
                text: "Start recording"
                color: "#FFFFFF"

            }

            MouseArea {
                id: mouseArea_start_recording
                anchors.fill: parent
                onClicked: backend.start_record()
            }
            radius: 20
        }

        Rectangle {
            id : pause
            x: 143
            y: 356
            width: 100
            height: 30
            color: "#2a2a2a"
            Text {
                anchors.centerIn: parent
                text: "pause"
                color: "#FFFFFF"

            }

            MouseArea {
                id: mouseArea_pause
                anchors.fill: parent
                onClicked: backend.pause_record()
            }
            radius: 20
        }

        Rectangle {
            id : resume
            x: 253
            y: 259
            width: 100
            height: 30
            color: "#2a2a2a"
            Text {
                anchors.centerIn: parent
                text: "resume"
                color: "#FFFFFF"

            }

            MouseArea {
                id: mouseArea_resume
                anchors.fill: parent
                onClicked: backend.resume_record()
            }
            radius: 20
        }

        Rectangle {
            id : stop
            x: 268
            y: 356
            width: 100
            height: 30
            color: "#2a2a2a"
            Text {
                anchors.centerIn: parent
                text: "stop"
                color: "#FFFFFF"

            }

            MouseArea {
                id: mouseArea_stop
                anchors.fill: parent
                onClicked: backend.stop_record()
            }
            radius: 20
        }

        Rectangle {
            id: audio_devices
            x: 123
            y: 149
            width: 160
            height: 30
            color: "#2a2a2a"

            property bool listVisible: false

            Rectangle {
                id: dropdown
                width: parent.width
                height: 30
                color: "#3a3a3a"

                Text {
                    id: selectedText
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 5
                    text: "Select..."
                    color: "white"
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: listView.visible = !listView.visible
                }
            }
            property var audioDevices: backend.getAudioDevices()
            ListView {
                id: listView
                width: 160
                height: 100
                y: dropdown.height
                visible: false
                clip: true

                model:audioDevices

                delegate: Rectangle {
                    width: listView.width
                    height: 30
                    color: "#000000"

                    Row {
                        spacing: 5
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 5

                        Rectangle {
                            width: 20
                            height: 20
                            color: "#2a2a2a"
                        }

                        Text {
                            text: modelData
                            color: "#FFFFFF"
                            font.pixelSize: 10
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            selectedText.text = modelData;
                            backend.set_audioDeviceName(modelData);
                            listView.visible = false;
                        }
                    }
                }
            }
            Component.onCompleted: {
                listView.model = backend.getAudioDevices()
            }
        }

    }
}
