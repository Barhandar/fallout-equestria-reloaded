import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qrc:/assets/ui" as UiStyle
import "../ui"

Pane {
  id: root
  property alias model: repeater.model
  property string currentName
  property bool readOnly: false
  background: UiStyle.TerminalPane {}
  Layout.fillHeight: true
  Layout.preferredWidth: 200

  signal newClicked()

  Flickable {
    anchors.fill: parent
    contentHeight: column.height
    clip: true
    Column {
      id: column
      spacing: 5

      TerminalButton {
        text: "+ Add"
        onClicked: newClicked()
        visible: !readOnly
      }

      Repeater {
        id: repeater
        delegate: TerminalButton {
          text: repeater.model[index]
          onClicked: { root.currentName = repeater.model[index]; }
          backgroundColor: root.currentName === repeater.model[index] ? "green" : "transparent"
          width: column.parent.width
          height: 25
        }
      }
    }
  }
}
// END ItemSelect