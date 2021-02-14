#include "dynamicobject.h"
#include "game.h"
#include <QJsonArray>

DynamicObject::DynamicObject(QObject *parent) : Sprite(parent)
{
  taskManager = new TaskRunner(this);
  connect(&tick, &QTimer::timeout, this, &DynamicObject::onTicked);
  connect(this, &Sprite::movementFinished, this, &DynamicObject::onMovementEnded);
  connect(this, &DynamicObject::reachedDestination, this, &DynamicObject::onDestinationReached);
}

void DynamicObject::setScript(const QString& name)
{
  script = Game::get()->loadScript(getScriptPath() + '/' + name);
  QJSValue callback = script.property("initialize");

  scriptName = name;
  taskManager->setLocalModule(script);
  if (callback.isCallable())
  {
    QJSValueList args;

    args << Game::get()->getScriptEngine().newQObject(this);
    Game::get()->scriptCall(callback, args, "initialize");
  }
}

void DynamicObject::setTickBehavior(int interval, bool repeat)
{
  tick.setSingleShot(!repeat);
  tick.setInterval(interval);
  tick.start();
}

void DynamicObject::moveTo(int x, int y, QPoint renderPosition)
{
  QString direction, animationName;

  if (position.x() > x && position.y() > y)
    direction = "up";
  else if (position.x() < x && position.y() < y)
    direction = "down";
  else if (position.x() == x && position.y() > y)
    direction = "right";
  else if (position.x() == x && position.y() < y)
    direction = "left";
  else if (position.x() >= x)
    direction = "left";
  else
    direction = "right";
  animationName = "walking-" + direction;
  position = QPoint(x, y);
  moveToCoordinates(renderPosition);
  if (getCurrentAnimation() != animationName)
    setAnimation(animationName);
}

void DynamicObject::onMovementEnded()
{
  QJSValue callback = script.property("onMovementEnded");

  if (callback.isCallable())
  {
    QJSValueList args;

    args << Game::get()->getScriptEngine().newQObject(this);
    Game::get()->scriptCall(callback, args, "onMovementEnded");
  }
  //position = nextPosition;
  //qDebug() << "new position" << position;
}

void DynamicObject::onDestinationReached()
{
  QJSValue callback = script.property("onDestinationReached");

  if (callback.isCallable())
  {
    QJSValueList args;

    args << Game::get()->getScriptEngine().newQObject(this);
    Game::get()->scriptCall(callback, args, "onDestinationReached");
  }
}

void DynamicObject::onTicked()
{
  QJSValue callback = script.property("onTicked");

  if (callback.isCallable())
  {
    QJSValueList args;

    args << Game::get()->getScriptEngine().newQObject(this);
    Game::get()->scriptCall(callback, args, "onTicked");
  }
}

QStringList DynamicObject::getAvailableInteractions()
{
  QJSValue callback = script.property("getAvailableInteractions");

  if (callback.isCallable())
  {
    QJSValueList args;
    QJSValue retval;

    args << Game::get()->getScriptEngine().newQObject(this);
    retval = Game::get()->scriptCall(callback, args, "getAvailableInteractions");
    return retval.toVariant().toStringList();
  }
  return QStringList();
}

void DynamicObject::update(qint64 delta)
{
  Sprite::update(delta);
  taskManager->update(delta);
}

void DynamicObject::load(const QJsonObject& data)
{
  objectName = data["objectName"].toString();
  position.setX(data["x"].toInt()); position.setY(data["y"].toInt());
  nextPosition.setX(data["nextX"].toInt()); nextPosition.setY(data["nextY"].toInt());
  interactionPosition.setX(data["intX"].toInt()); interactionPosition.setY(data["intY"].toInt());
  for (QJsonValue pathPointData : data["currentPath"].toArray())
  {
    QPoint pathPoint;

    pathPoint.setX(pathPointData["x"].toInt());
    pathPoint.setY(pathPointData["y"].toInt());
    currentPath << pathPoint;
  }
  currentZone = data["currentZone"].toString();
  scriptName  = data["script"].toString();
  dataStore   = data["dataStore"].toObject();
  Sprite::load(data);
  setScript(scriptName);
  taskManager->setLocalModule(script);
  taskManager->load(data);
}

void DynamicObject::save(QJsonObject& data) const
{
  QJsonArray currentPathData;

  data["objectName"] = objectName;
  data["x"] = position.x(); data["y"] = position.y();
  data["nextX"] = nextPosition.x(); data["nextY"] = nextPosition.y();
  data["intX"] = interactionPosition.x(); data["intY"] = interactionPosition.y();
  for (QPoint pathPoint : currentPath)
  {
    QJsonObject pathPointData;

    pathPointData["x"] = pathPoint.x();
    pathPointData["y"] = pathPoint.y();
    currentPathData << pathPointData;
  }
  data["currentPath"] = currentPathData;
  data["currentZone"] = currentZone;
  data["script"]      = scriptName;
  data["dataStore"]   = dataStore;
  Sprite::save(data);
  taskManager->save(data);
}
