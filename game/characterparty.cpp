#include "characterparty.h"
#include "leveltask.h"
#include <QJsonArray>

CharacterParty::CharacterParty(QObject *parent) : QObject(parent)
{

}

void CharacterParty::addCharacter(Character* character)
{
  character->setParent(this);
  list.push_back(character);
  emit partyChanged();
}

void CharacterParty::removeCharacter(Character* character)
{
  list.removeAll(character);
  emit partyChanged();
}

void CharacterParty::removeCharacter(const QString& name)
{
  for (auto it = list.begin() ; it != list.end() ;)
  {
    if ((*it)->getStatistics()->getName() == name)
      it = list.erase(it);
    else
      ++it;
  }
  emit partyChanged();
}

Character* CharacterParty::get(const QString& name)
{
  for (auto it = list.begin() ; it != list.end() ; ++it)
  {
    if ((*it)->getStatistics()->getName() == name)
      return *it;
  }
  return nullptr;
}

bool CharacterParty::insertIntoZone(LevelTask* level, TileZone* zone)
{
  auto* grid = level->getGrid();
  int characterIt = 0;

  for (auto position : zone->getPositions())
  {
    if (!grid->isOccupied(position.x(), position.y()))
    {
      Character* character = list.at(characterIt);

      level->registerDynamicObject(character);
      level->forceCharacterPosition(character, position.x(), position.y());
      if (++characterIt >= list.length())
        break ;
    }
  }
  return characterIt == list.length();
}

bool CharacterParty::insertIntoZone(LevelTask* level, const QString &zoneName)
{
  auto* tileMap = level->getTileMap();

  for (auto* zone : tileMap->getZones())
  {
    qDebug() << "Detected" << zone->getType() << "zone" << zone->getName() << zone->getIsDefault();
    if (zone->getName() == zoneName)
      return insertIntoZone(level, zone);
  }
  return false;
}

void CharacterParty::extractFromLevel(LevelTask* level)
{
  auto* grid = level->getGrid();

  for (Character* character : list)
  {
    grid->removeObject(character);
    level->unregisterDynamicObject(character);
  }
}

void CharacterParty::save(QJsonObject& data)
{
  QJsonArray charactersData;

  data["name"] = name;
  for (Character* character : list)
  {
    QJsonObject characterData;

    character->save(characterData);
    charactersData << characterData;
  }
  data.insert("list", charactersData);
}

void CharacterParty::load(const QJsonObject& data, LevelTask* level)
{
  auto* grid = level ? level->getGrid() : nullptr;

  name = data["name"].toString();
  for (QJsonValue characterDataV : data["list"].toArray())
  {
    QJsonObject characterData = characterDataV.toObject();
    Character*  character = new Character(this);

    character->load(characterData);
    if (level)
    {
      grid->moveObject(character, character->getPosition().x(), character->getPosition().y());
      level->registerDynamicObject(character);
    }
  }
}