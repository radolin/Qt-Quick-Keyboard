import QtQuick 2.0

Rectangle {
	width: 800
	height: 480
	color: "#3a6ea5"

            Rectangle {
                anchors {
                    left: parent.left
                    top: parent.top
                    right: parent.right
                    bottom: keyboardArea.top
                    margins: 10
                }
                color: "white"
                TextEdit {
                    anchors { fill: parent; margins: 2 }
                }
            }



	Item {
		id: keyboardArea
		anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
		height: Qt.inputMethod === undefined ? 1 : Qt.inputMethod.keyboardRectangle.height
	}
}

