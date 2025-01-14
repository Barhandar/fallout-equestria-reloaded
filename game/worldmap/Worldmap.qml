import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import QtQuick.Shapes 1.15
import "../../assets/ui" as UiStyle
import "../../ui"
import "../hud" as LevelHud
import Game 1.0 as MyGame

Item {
  property QtObject controller // WorldMap
  property point movementStart: controller.currentPosition
  property bool hasOverlay: inventoryViewContainer.visible || mainMenu.visible
  id: root
  state: "default"
  //onHasOverlayChanged: controller.paused = hasOverlay || root.state != "default";

  Connections {
    target: controller

    function onEncounterTriggered(title) { application.popView(); }
    function onEncounterNotify(name, params) {
      root.state = "randomEncounter";
      if (params.optional) {
        encounterConfirmDialog.text = i18n.t("messages.encounter-title", params) + ' ' + i18n.t("messages.encounter-prompt");
        encounterConfirmDialog.open();
      }
      root.controller.paused = true;
    }
  }

  ConfirmDialog {
    id: encounterConfirmDialog
    anchors.centerIn: parent
    onAccepted: gameManager.currentGame.randomEncounters.triggerScheduledEncounter()
    onRejected: {
      root.state = "default";
      root.controller.paused = false;
    }
  }

  LevelHud.Actions {
    id: actions
    enabled: application.currentView === root && root.state == "default"
    onMenuTriggered:           mainMenu.visible = true
    onInventoryTriggered:      inventoryViewContainer.visible = true
    onBackTriggered: {
      if  (mainMenu.visible)
        mainMenu.visible = false;
      else if (inventoryViewContainer.visible)
        inventoryViewContainer.visible = false;
      else
        mainMenu.visible = true
    }
  }

  function clickedOnPlayer() {
    console.log("Clicked on player at", controller.currentPosition, controller.paused);
    if (controller.paused) { return ; }
    for (var i = 0 ; i < controller.cities.length ; ++i) {
      if (controller.cities[i].isInside(controller.currentPosition)) {
        application.popView();
        return controller.getIntoCity(controller.cities[i]);
      }
    }
    controller.getIntoWasteland(controller.currentPosition);
  }

  function clickedOnMap() {
    console.log("Clicked on map", worldmapView.mouseX, worldmapView.mouseY, controller.paused);
    if (controller.paused) { return ; }
    movementStart = controller.currentPosition;
    controller.targetPosition = Qt.point(worldmapView.mouseX, worldmapView.mouseY);
  }

  function clickedOnCity(city) {
    if (controller.paused) { return ; }
    movementStart = controller.currentPosition;
    controller.targetPosition = city.position;
  }

  WorldmapView {
    id: worldmapView
    activated: actions.enabled
    anchors { top: parent.top; left: parent.left; bottom: parent.bottom; right: sidebar.left }
    controller: root.controller
    onMapClicked: clickedOnMap()

    content: [
      WorldmapCities {
        model: controller.cities
      }
    ]

    Shape {
      anchors.fill: parent
      ShapePath {
        startX: movementStart.x
        startY: movementStart.y
        strokeWidth: 2
        strokeColor: "red"
        strokeStyle: ShapePath.DashLine
        PathLine { x: controller.currentPosition.x; y: controller.currentPosition.y }
      }
    }

    Shape {
      id: positionShape
      x: controller.currentPosition.x - width / 2; y: controller.currentPosition.y - height / 2
      width: 24
      height: 12
      ShapePath {
        strokeWidth: positionMouseArea.containsMouse ? 6 : 4
        strokeColor: "yellow"
        fillColor: Qt.rgba(255, 255, 0, positionMouseArea.containsMouse ? 1 : 0.5)
        startX: 0
        startY: 0
        PathLine { x: positionShape.width; y: 0 }
        PathLine { x: positionShape.width / 2; y: positionShape.height }
        PathLine { x: 0; y: 0}
      }
      MouseArea {
        id: positionMouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: clickedOnPlayer()
      }
    }

    Rectangle {
      id: encounterShape
      visible: false
      x: controller.currentPosition.x - width / 2; y: controller.currentPosition.y - height / 2
      width: 50
      height: 50
      radius: width * 0.5
      color: Qt.rgba(255, 0, 0, 0.5);
      border.color: "red"
      border.width: 3
      RotationAnimation on opacity {
        loops: Animation.Infinite
        from: 1
        to:   0.5
      }
    }
  }

  states: [
    State {
      name: "randomEncounter"
      PropertyChanges { target: encounterShape; visible: true }
      PropertyChanges { target: positionShape;  visible: false }
    },
    State {
      name: "default"
      PropertyChanges { target: encounterShape; visible: false }
      PropertyChanges { target: positionShape;  visible: true }
    }
  ]

  WorldmapSidebar {
    id: sidebar
    anchors { right: parent.right; top: parent.top; bottom: parent.bottom }
    controller: root.controller
  }

  LevelHud.PlayerInventory {
    id: inventoryViewContainer
    visible: false
    anchors.fill: parent
  }

  LevelHud.Menu {
    id: mainMenu
    anchors.centerIn: parent
    visible: false
    onVisibleChanged: controller.paused = visible
  }
}
