#include "sight.h"
#include "game.h"
#include <cmath>

CharacterSight::CharacterSight(QObject *parent) : ParentType(parent)
{
  fieldOfView = new FieldOfView(reinterpret_cast<Character&>(*this));
  connect(this, &CharacterDiplomacy::diplomacyUpdated, this, &CharacterSight::refreshFieldOfView);
  connect(fieldOfView, &FieldOfView::characterDetected, this, &CharacterSight::onCharacterDetected, Qt::QueuedConnection);
}

CharacterSight::~CharacterSight()
{
  delete fieldOfView;
}

bool CharacterSight::hasLineOfSight(const DynamicObject* other) const
{
  return hasSightFrom(other, position);
}

bool CharacterSight::hasLineOfSight(DynamicObject *other) const
{
  return hasSightFrom(other, position);
}

bool CharacterSight::hasSightFrom(const DynamicObject* other, QPoint pos)
{
  if (other)
    return hasSightFrom(other->getPosition(), pos);
  return false;
}

bool CharacterSight::hasLineOfSight(QPoint target) const
{
  return hasSightFrom(target, position);
}

bool CharacterSight::hasSightFrom(QPoint target, QPoint pos)
{
  auto* level = Game::get()->getLevel();

  if (level)
  {
    auto*  grid   = level->getGrid();
    int    score  = grid->getVisionQuality(pos.x(), pos.y(), target.x(), target.y());

    return score > 0;
  }
  return false;
}

float CharacterSight::getDistance(const DynamicObject* target) const
{
  return getDistance(target->getPosition());
}

float CharacterSight::getDistance(DynamicObject* target) const
{
  return getDistance(target->getPosition());
}

float CharacterSight::getDistance(QPoint other) const
{
  auto self  = getPosition();
  auto a = self.x() - other.x();
  auto b = self.y() - other.y();

  return std::sqrt(static_cast<float>(a * a + b * b));
}

void CharacterSight::refreshFieldOfView()
{
  if (fieldOfView && Game::get()->getLevel())
  {
    fieldOfView->reset();
    fieldOfView->runTask();
  }
}

void CharacterSight::onCharacterDetected(Character* character)
{
  if (script && script->hasMethod("onCharacterDetected"))
  {
    QJSValueList args;

    args << character->asJSValue();
    script->call("onCharacterDetected", args);
  }
}
