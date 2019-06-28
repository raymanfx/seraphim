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

            Button {
                id: laneDetectionButton
                implicitWidth: 200
                implicitHeight: 100
                text: "Lane Detection"
                property bool active: false
                onClicked: {
                    active = !active;
                    mainWindow.laneDetectionButtonClicked();
                }

                contentItem: Text {
                    text: laneDetectionButton.text
                    font: laneDetectionButton.font
                    opacity: enabled ? 1.0 : 0.3
                    color: laneDetectionButton.pressed ? "#F84018" : "#FFFFFF";
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    color: laneDetectionButton.active ? "#F84018" : "#000000";
                    border.color: "#FFFFFF";
                    border.width: 1
                    radius: 2
                    opacity: enabled ? 1 : 0.3
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
        TextArea {
            id: debugOutput
            color: "#FFFFFF"
        }
    }
}
