import QtQuick 2.15
import QtQuick.Controls 2.15
import "qrc:/assets/ui" as UiStyle

Pane {
  property QtObject gameController
  background: UiStyle.TerminalPane {}

  Flickable {
    id: terminalFlickable
    anchors.fill: parent
    contentHeight: terminalContent.height
    clip: true

    ScrollBar.vertical: UiStyle.TerminalScrollbar { orientation: Qt.Vertical }

    Column {
      id: terminalContent
      width: parent.width - 10
      onHeightChanged: terminalFlickable.contentY = Math.max(0, terminalContent.height - terminalFlickable.height);

      Repeater {
        model: gameController.consoleMessages
        delegate: Text {
          horizontalAlignment: Text.AlignJustify
          wrapMode: Text.WordWrap
          width: terminalContent.width - 5
          text: "> " + gameController.consoleMessages[index]
          color: "green"
          font.family: application.consoleFont.name
          font.pointSize: application.consoleFont.tinySize
        }
      }
    }
  }
}
