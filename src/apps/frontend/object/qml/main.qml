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
                text: qsTr("Transport Session..")
                onTriggered: transportDialog.open()
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
            id: transportDialog
            parent: mainLayout
            x: (parent.width - width) / 2
            y: (parent.height - height) / 2
            title: "Transport Session"
            standardButtons: Dialog.Ok | Dialog.Cancel

            contentItem: TextField {
                id: transportURI
                text: "shm:///seraphim"
            }

            onAccepted: objectPage.backendSessionOpened = mainWindow.openTransportSession(transportURI.text)
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
