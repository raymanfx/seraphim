import QtQuick 2.0
import QtQuick.Window 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.0

import "pages"

ApplicationWindow {
    visible: true
    minimumWidth: mainLayout.implicitWidth
    minimumHeight: mainLayout.implicitHeight
    title: "Seraphim"

    menuBar: MenuBar {
        id: topMenuBar
        Menu {
            title: qsTr("&Session")
            Action {
                text: qsTr("Open SHM..")
                onTriggered: shmDialog.open()
            }
            Action {
                text: qsTr("Open TCP..")
                onTriggered: tcpDialog.open()
            }
        }

        Menu {
            title: qsTr("&Settings")
            Action {
                text: qsTr("Sync to backend")
                checkable: true
                onCheckedChanged: mainWindow.toggleBackendSync(checked)
            }
        }

        Dialog {
            id: shmDialog
            parent: mainLayout
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            title: "Shared Memory Session"
            standardButtons: Dialog.Ok | Dialog.Cancel

            contentItem: TextField {
                id: shmPath
                text: "/seraphim"
            }

            onAccepted: objectPage.backendSessionOpened = mainWindow.openShmSession(shmPath.text)
        }

        Dialog {
            id: tcpDialog
            parent: mainLayout
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            title: "TCP Session"
            standardButtons: Dialog.Ok | Dialog.Cancel

            contentItem: RowLayout {
                TextField {
                    id: tcpIp
                    text: "127.0.0.1"
                }

                Label {
                    text: ":"
                }

                SpinBox {
                    id: tcpPort
                    from: 0
                    to: 65535
                    value: 8001
                }
            }

            onAccepted: objectPage.backendSessionOpened = mainWindow.openTcpSession(tcpIp.text, tcpPort.value)
        }
    }

    ColumnLayout {
        id: mainLayout
        anchors.fill: parent

        TabBar {
            id: navbar
            Layout.fillWidth: parent

            TabButton {
                text: qsTr("Object")
            }
        }

        StackLayout {
            currentIndex: navbar.currentIndex

            ObjectPage {
                id: objectPage
            }
        }
    }
}
