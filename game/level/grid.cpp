#include "grid.h"
#include "game/characters/actionqueue.h"

GridComponent::GridComponent(QObject *parent) : QObject(parent)
{
  grid = new LevelGrid(this);
}

void GridComponent::registerDynamicObject(DynamicObject* object)
{
  if (object->isCharacter())
  {
    Character* character = reinterpret_cast<Character*>(object);

    characterObservers.insert(character, {
      connect(character, &Sprite::movementFinished, this, [this, character]() { onMovementFinished(character); }),
      connect(character, &Character::died, this, [this, character]() {
                                  qDebug() << "vbvbvbzbitoiu";
                                  onCharacterDied(character); })
    });
  }
}

void GridComponent::unregisterDynamicObject(DynamicObject* object)
{
  if (object->isCharacter())
  {
    Character* character = reinterpret_cast<Character*>(object);

    character->setAnimation("idle-down");
    for (auto observer : characterObservers.value(character))
      disconnect(observer);
    characterObservers.remove(character);
  }
}

void GridComponent::addCharacterObserver(Character* character, QMetaObject::Connection observer)
{
  characterObservers[character].push_back(observer);
}

DynamicObject* GridComponent::getOccupantAt(QPoint position)
{
  return grid->getOccupant(position.x(), position.y());
}

void GridComponent::onCharacterDied(Character* character)
{
  qDebug() << "character died " << character;
  grid->removeObject(character);
}

void GridComponent::onMovementFinished(Character* object)
{
  auto position = object->getPosition();

  grid->triggerZone(object, position.x(), position.y());
  if (object->rcurrentPath().size() > 0)
  {
    object->rcurrentPath().pop_front();
    if (object->getCurrentPath().size() > 0)
    {
      QPoint nextCase = object->getCurrentPath().first();

      if (!startCharacterMoveToTile(object, nextCase.x(), nextCase.y()))
      {
        object->getActionQueue()->reset();
        emit object->getActionQueue()->queueCompleted();
        emit object->pathBlocked(); // Remove ?
      }
    }
    else
    {
      object->setAnimation("idle-down");
      emit object->reachedDestination();
    }
  }
  else
    qDebug() << "!!";
}

bool GridComponent::startCharacterMoveToTile(Character* character, int x, int y)
{
  if (grid->moveObject(character, x, y))
  {
    QPoint renderPosition = getRenderPositionForTile(x, y);

    character->moveTo(x, y, renderPosition);
    return true;
  }
  return false;
}

void GridComponent::setCharacterPosition(Character* character, int x, int y)
{
  setObjectPosition(character, x, y);
}

void GridComponent::setObjectPosition(DynamicObject* object, int x, int y)
{
  grid->moveObject(object, x, y);
  object->setPosition(QPoint(x, y));
  if (!object->isFloating())
  {
    QPoint renderPosition = getRenderPositionForTile(x, y);

    object->setRenderPosition(renderPosition);
  }
}

bool GridComponent::moveTo(Character* character, QPoint targetPosition)
{
  QPoint position = character->getPosition();

  if (grid->findPath(position, targetPosition, character->rcurrentPath()))
  {
    if (character->getCurrentPath().size() > 0)
    {
      QPoint nextCase = character->getCurrentPath().first();

      startCharacterMoveToTile(character, nextCase.x(), nextCase.y());
    }
    else
      emit character->reachedDestination();
    return true;
  }
  return false;
}
