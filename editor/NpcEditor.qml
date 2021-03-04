import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qrc:/assets/ui" as UiStyle
import "../ui"
import Game 1.0

Item {
  property var list: scriptController.getCharacterSheets()
  property alias currentCharacter: characterSheetSelect.currentName

  onCurrentCharacterChanged: {
    scriptController.loadCharacterSheet(currentCharacter, statController);
    statController.specialPoints = Math.max(0, 40 - (statController.strength + statController.perception + statController.endurance + statController.charisma + statController.intelligence + statController.agility + statController.luck));
  }

  StatModel {
    id: statController
  }

  Dialog {
    id: newCharacterDialog
    title: "New character"
    modal: true
    anchors.centerIn: parent
    standardButtons: Dialog.Ok | Dialog.Cancel
    Row {
      Label { text: "name" }
      TextField { id: newCharacterName }
    }
    onAccepted: {
      list.push(newCharacterName.text + ".json");
      listChanged();
      currentCharacter = newCharacterName.text + ".json";
    }
  }

  RowLayout {
    anchors.fill: parent

    EditorSelectPanel {
      id: characterSheetSelect
      model: list
      onNewClicked: newCharacterDialog.open()
    }

    CharacterSheet {
      mode: "gameEditor"
      characterSheet: statController
      Layout.fillWidth: true
      Layout.fillHeight: true
      onAccepted: scriptController.saveCharacterSheet(currentCharacter, statController)
    }
  }
}
