import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "qrc:/assets/ui" as UiStyle
import ".."
import "../../ui"

Item {
  id: root
  property QtObject worldMap
  property var list
  property var cityNames: []
  property string selectedCity
  property QtObject currentModel

  function refreshNames() {
    const newArray = [];

    for (var i = 0 ; i < list.length ; ++i)
      newArray.push(list[i].name);
    cityNames = newArray;
  }

  onListChanged: refreshNames()

  onSelectedCityChanged: {
    currentModel = null;
    for (var i = 0 ; i < list.length ; ++i) {
      if (list[i].name === selectedCity) {
        currentModel = list[i];
        break ;
      }
    }
  }

  Loader {
    anchors.fill: parent
    sourceComponent: currentModel ? cityComponent : citySelectComponent
  }

  Component {
    id: citySelectComponent
    EditorSelectPanel {
      id: citySelect
      model: cityNames
      onCurrentNameChanged: root.selectedCity = currentName
      onNewClicked: addCityDialog.open()
    }
  }

  Component {
    id: cityComponent
    CityForm {
      cityModel: root.currentModel
      onCityNameChanged: root.refreshNames()
      onPreviousClicked: root.selectedCity = ""
      onDestroyClicked: {
        worldMap.removeCity(root.currentModel);
        root.refreshNames();
      }
    }
  }

  Dialog {
    id: addCityDialog
    title: "Add city"
    modal: true
    anchors.centerIn: parent.parent.parent.parent
    standardButtons: Dialog.Ok | Dialog.Cancel
    GridLayout {
      columns: 2
      Label { text: "Name" }
      TextField { id: newCityNameInput; text: "" }
    }
    onAccepted: {
      worldMap.createCity(newCityNameInput.text);
      selectedCity = newCityNameInput.text;
    }
  }
}
