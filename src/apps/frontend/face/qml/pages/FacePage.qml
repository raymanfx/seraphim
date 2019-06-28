import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0

Page {
    property bool backendSessionOpened: false

    GridLayout {
        id: mainLayout
        columns: 2

        Item {
            width: mainFrame.width
            height: mainFrame.height

            Image {
                id: mainFrame
                width: 1280
                height: 720
                Layout.alignment: Qt.AlignLeft | Qt.AlignTop
                cache: false
                visible: !overlayBlend.visible
                property int frameCount: 0

                Connections {
                    target: mainViewerObj

                    onRepaint: {
                        mainFrame.frameCount++;
                        mainFrame.source = "image://mainViewer/main/" + mainFrame.frameCount;
                        mainViewerObj.framePainted();
                    }
                }
            }

            Image {
                id: overlayFrame
                anchors.fill: mainFrame
                cache: false
                visible: false
                property int frameCount: 0

                Connections {
                    target: overlayViewerObj

                    onRepaint: {
                        overlayFrame.frameCount++;
                        overlayFrame.source = "image://overlayViewer/overlay/" + overlayFrame.frameCount;
                        overlayViewerObj.framePainted();
                    }
                }
            }

            Blend {
                id: overlayBlend
                anchors.fill: mainFrame
                source: mainFrame
                foregroundSource: overlayFrame
                mode: "normal"
                cached: false
                visible: false
            }
        }

        ColumnLayout {
            id: backendControl
            enabled: backendSessionOpened
            Layout.alignment: Qt.AlignLeft | Qt.AlignTop
            height: mainFrame.height

            ListView {
                id: faceButtonList
                model: ListModel { id: faceListModel }

                width: 200
                height: 500

                delegate: FaceButton {
                    id: faceButton
                    faceLabel: label
                    faceImage.source: image
                    onClicked: mainWindow.faceButtonClicked(label)
                }

                function getElement(label) {
                    for (var i = 0; i < model.count; i++) {
                        if (model.get(i).label === label) {
                            return model.get(i);
                        }
                    }
                    return null;
                }

                function getDelegate(label) {
                    for (var i = 0; i < contentItem.children.length; i++) {
                        var item = contentItem.children[i];
                        if (item.faceLabel === label) {
                            return item;
                        }
                    }
                    return null;
                }
            }

            Button {
                id: addFaceButton
                implicitWidth: parent.width
                implicitHeight: 100
                text: "Add"

                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    opacity: parent.enabled ? 1.0 : 0.3
                    color: parent.pressed ? "#F84018" : "#FFFFFF";
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    color: "#000000";
                    border.color: "#FFFFFF";
                    border.width: 1
                    radius: 2
                    opacity: enabled ? 1 : 0.3
                }

                property int faceLabel: 0

                onClicked: {
                    faceButtonList.model.append({
                        label: faceLabel++,
                        image: ""
                    })
                }
            }

            Button {
                id: faceDetectionButton
                implicitWidth: parent.width
                implicitHeight: 100
                text: "Detection"
                property bool active: false
                onClicked: {
                    active = !active;
                    mainWindow.faceDetectionButtonClicked();
                }

                contentItem: Text {
                    text: faceDetectionButton.text
                    font: faceDetectionButton.font
                    opacity: enabled ? 1.0 : 0.3
                    color: faceDetectionButton.pressed ? "#F84018" : "#FFFFFF";
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    color: faceDetectionButton.active ? "#F84018" : "#000000";
                    border.color: "#FFFFFF";
                    border.width: 1
                    radius: 2
                    opacity: enabled ? 1 : 0.3
                }
            }

            Button {
                id: faceRecognitionButton
                implicitWidth: parent.width
                implicitHeight: 100
                text: "Recognition"
                onClicked: {
                    addFaceButton.enabled = !addFaceButton.enabled;
                    faceButtonList.enabled = !faceButtonList.enabled;
                    mainWindow.faceRecognitionButtonClicked();
                }

                contentItem: Text {
                    text: faceRecognitionButton.text
                    font: faceRecognitionButton.font
                    opacity: enabled ? 1.0 : 0.3
                    color: faceRecognitionButton.pressed ? "#F84018" : "#FFFFFF";
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    color: "#000000";
                    border.color: "#FFFFFF";
                    border.width: 1
                    radius: 2
                    opacity: enabled ? 1 : 0.3
                }
            }

            Connections {
                target: mainWindow

                onTraining: {
                    trainingBar.visible = progress < 1;
                    trainingBar.value = progress

                    if (progress < 1) {
                        return;
                    }

                    var listElem = faceButtonList.getElement(label);
                    if (!listElem) {
                        console.warn("Cannot handle face label: " + label);
                        return;
                    }

                    listElem.image = "";
                    listElem.image = "image://faceStorage/" + label;
                    faceRecognitionButton.enabled = true;
                }

                onRecognized: {
                    var listElem = faceButtonList.getElement(label);
                    if (!listElem) {
                        console.warn("Cannot handle face label: " + label);
                        return;
                    }

                    faceButtonList.currentIndex = label;
                    faceButtonList.getDelegate(label).glow(1000);
                }
            }
        }

        Connections {
            target: mainWindow

            onPrintDiag: {
                debugOutput.text = text;
            }

            onToggleOverlay: {
                overlayBlend.visible = visible;
            }
        }
    }

    footer: ColumnLayout {
        ProgressBar {
            id: trainingBar
            visible: false
        }

        TextArea {
            id: debugOutput
            color: "#FFFFFF"
        }
    }
}
