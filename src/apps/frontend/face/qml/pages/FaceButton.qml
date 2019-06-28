import QtQuick 2.0
import QtQuick.Controls 2.0
import QtGraphicalEffects 1.0

Button {
    id: faceButton
    implicitWidth: 200
    implicitHeight: 200
    text: faceButtonImage.source == "" ? "Enroll" : ""

    property int faceLabel

    // Specify the name of the children to access them
    property alias faceImage: faceButtonImage

    Rectangle {
        anchors.fill: parent
        color: "#000000";
        border.color: faceButtonImage.source == "" ? "#FFFFFF" : "#000000";
        border.width: 4
        radius: 1
    }

    RectangularGlow {
        id: faceButtonGlow
        anchors.fill: parent
        visible: faceButtonGlowEffect.running
        glowRadius: 20
        color: "#F84018"

        PropertyAnimation {
            id: faceButtonGlowEffect
            target: faceButtonGlow
            property: "opacity"
            from: 1.0
            to: 0.5
            duration: 1000
            loops: 1
            easing.type: Easing.OutQuad
            running: false
        }
    }

    Image {
        id: faceButtonImage
        anchors.fill: parent
        anchors.centerIn: parent
        anchors.margins: 10
        cache: false
        fillMode: Image.PreserveAspectFit
    }

    contentItem: Text {
        text: faceButton.text
        font: faceButton.font
        opacity: enabled ? 1.0 : 0.3
        color: faceButton.pressed ? "#F84018" : "#FFFFFF";
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    function glow(duration) {
        faceButtonGlowEffect.duration = duration;
        faceButtonGlowEffect.running = true;
    }
}
