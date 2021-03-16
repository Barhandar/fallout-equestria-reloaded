#include "levelgrid.h"
#include "tilemap/tilemap.h"
#include "dynamicobject.h"
#include "character.h"
#include "astar.hpp"
#include <cmath>
#include <QLineF>
#include <QRectF>

static bool lineIntersectsRect(QLineF line, QRectF rect)
{
  QLineF leftSide(rect.topLeft(), rect.bottomLeft());
  QLineF topSide(rect.topLeft(), rect.topRight());
  QLineF rightSide(rect.topRight(), rect.bottomRight());
  QLineF bottomSide(rect.bottomLeft(), rect.bottomRight());

  return (leftSide.intersects(line, nullptr)   == QLineF::BoundedIntersection) ||
         (topSide.intersects(line, nullptr)    == QLineF::BoundedIntersection) ||
         (rightSide.intersects(line, nullptr)  == QLineF::BoundedIntersection) ||
         (bottomSide.intersects(line, nullptr) == QLineF::BoundedIntersection);
}

bool LevelGrid::CaseContent::isBlocked() const
{
  if (!occupied)
  {
    for (const auto* zone : zones)
    {
      if (zone->getAccessBlocked())
        return true;
    }
    return false;
  }
  return true;
}

LevelGrid::LevelGrid(QObject *parent) : QObject(parent)
{
}

void LevelGrid::initializeGrid(TileMap* tilemap)
{
  auto* wallLayer = tilemap->getLayer("walls");

  size = tilemap->getSize();
  grid.resize(size.width() * size.height());
  eachCase([wallLayer](int x, int y, CaseContent& gridCase)
  {
    gridCase.occupied = wallLayer->getTile(x, y) != nullptr;
    gridCase.position = QPoint(x, y);
  });
  initializePathfinding();
}

void LevelGrid::eachCase(std::function<void (int x, int y, CaseContent&)> callback, QPoint from, QPoint to)
{
  if (to.x() == 0 && to.y() == 0)
    to = QPoint(size.width(), size.height());
  for (int x = from.x() ; x < to.x() ; ++x)
  {
    for (int y = from.y() ; y < to.y() ; ++y)
    {
      int          position = y * size.width() + x;
      CaseContent& gridCase = grid[position];

      callback(x, y, gridCase);
    }
  }
}

void LevelGrid::registerZone(TileZone* zone)
{
  for (const QPoint position : zone->getPositions())
  {
    CaseContent* gridCase = getGridCase(position.x(), position.y());

    if (gridCase)
      gridCase->zones.append(zone);
  }
}

void LevelGrid::unregisterZone(TileZone* zone)
{
  for (const QPoint position : zone->getPositions())
  {
    CaseContent* gridCase = getGridCase(position.x(), position.y());

    if (gridCase)
      gridCase->zones.removeOne(zone);
  }
}

void LevelGrid::initializePathfinding()
{
  for (auto& gridCase : grid)
  {
    bool hasLeft  = gridCase.position.x() - 1 >= 0;
    bool hasRight = gridCase.position.x() + 1 < size.width();
    bool hasUp    = gridCase.position.y() - 1 >= 0;
    bool hasDown  = gridCase.position.y() + 1 < size.height();

    gridCase.successors.clear();
    gridCase.successors.push_back(hasLeft && hasUp    ? getGridCase(gridCase.position.x() - 1, gridCase.position.y() - 1) : nullptr);
    gridCase.successors.push_back(hasUp               ? getGridCase(gridCase.position.x(),     gridCase.position.y() - 1) : nullptr);
    gridCase.successors.push_back(hasRight && hasUp   ? getGridCase(gridCase.position.x() + 1, gridCase.position.y() - 1) : nullptr);
    gridCase.successors.push_back(hasLeft             ? getGridCase(gridCase.position.x() - 1, gridCase.position.y())     : nullptr);
    gridCase.successors.push_back(hasRight            ? getGridCase(gridCase.position.x() + 1, gridCase.position.y())     : nullptr);
    gridCase.successors.push_back(hasDown && hasLeft  ? getGridCase(gridCase.position.x() - 1, gridCase.position.y() + 1) : nullptr);
    gridCase.successors.push_back(hasDown             ? getGridCase(gridCase.position.x(),     gridCase.position.y() + 1) : nullptr);
    gridCase.successors.push_back(hasDown && hasRight ? getGridCase(gridCase.position.x() + 1, gridCase.position.y() + 1) : nullptr);
    for (auto it = gridCase.successors.begin() ; it != gridCase.successors.end() ;)
    {
      if ((*it) == nullptr || (*it)->occupied)
        it = gridCase.successors.erase(it);
      else
        it++;
    }
  }
}

bool LevelGrid::findPath(QPoint from, QPoint to, QList<QPoint>& path)
{
  typedef AstarPathfinding<LevelGrid::CaseContent> Pathfinder;
  Pathfinder        astar;
  unsigned short    iterationCount = 0;
  Pathfinder::State state;
  CaseContent*      fromCase = getGridCase(from.x(), from.y());
  CaseContent*      toCase   = getGridCase(to.x(), to.y());

  if (fromCase && toCase)
  {
    bool fromOccupiedBackup = fromCase->occupied;
    bool toOccupiedBackup = toCase->occupied;

    fromCase->occupied = false;
    toCase->occupied = false;
    path.clear();
    astar.SetStartAndGoalStates(*fromCase, *toCase);
    while ((state = astar.SearchStep()) == Pathfinder::Searching && ++iterationCount < 250);
    if (state == Pathfinder::Succeeded)
    {
      for (auto& gridCase : astar.GetSolution())
        path << gridCase.position;
      path.pop_front(); // first case is the starting point
      fromCase->occupied = fromOccupiedBackup;
      toCase->occupied = toOccupiedBackup;
      return true;
    }
    fromCase->occupied = fromOccupiedBackup;
    toCase->occupied = toOccupiedBackup;
  }
  return false;
}

std::list<LevelGrid::CaseContent*> LevelGrid::CaseContent::GetSuccessors(const CaseContent* parent) const
{
  std::list<LevelGrid::CaseContent*> results;

  for (auto* node : successors)
  {
    if (node != parent && !node->isBlocked())
      results.push_back(node);
  }
  return results;
}

float LevelGrid::CaseContent::GoalDistanceEstimate(const CaseContent& other) const
{
  int distX = position.x() - other.position.x();
  int distY = position.y() - other.position.y();

  return std::sqrt(static_cast<float>(distX * distX + distY * distY));
}

LevelGrid::CaseContent* LevelGrid::getGridCase(int x, int y)
{
  int position = y * size.width() + x;

  if (position >= grid.count() || position < 0)
    return nullptr;
  return &(grid[position]);
}

bool LevelGrid::isOccupied(int x, int y) const
{
  int index = y * size.width() + x;

  if (index >= grid.count() || index < 0)
    return true;
  return grid.at(index).isBlocked();
}

void LevelGrid::setCaseOccupant(CaseContent& _case, DynamicObject* occupant)
{
  if (occupant && occupant->isBlockingPath())
  {
    _case.occupied = true;
    _case.occupant = occupant;
  }
  else
  {
    _case.occupied = false;
    _case.occupant = nullptr;
  }
}

DynamicObject* LevelGrid::getOccupant(int x, int y)
{
  auto* gridCase = getGridCase(x, y);

  if (gridCase)
    return gridCase->occupant;
  return nullptr;
}

int LevelGrid::getVisionQuality(int fromX, int fromY, int toX, int toY)
{
  const qreal   caseSize = 10;
  const int     minX = std::min(fromX, toX), minY = std::min(fromY, toY);
  const int     maxX = std::max(fromX, toX), maxY = std::max(fromY, toY);
  const QPointF sightFrom(static_cast<qreal>(fromX - minX) * caseSize, static_cast<qreal>(fromY - minY) * caseSize);
  const QPointF sightTo  (static_cast<qreal>(toX - minX) * caseSize,   static_cast<qreal>(toY - minY) * caseSize);
  const QLineF  sightLine(sightFrom, sightTo);
  int visionScore = 100;

  for (int x = minX ; x <= maxX ; ++x)
  {
    for (int y = minY ; y <= maxY ; ++y)
    {
      LevelGrid::CaseContent* gridCase;
      qreal posX, posY;

      if ((x == fromX && y == fromY) || (x == toX && y == toY))
        continue ;
      gridCase = getGridCase(x, y);
      if (!gridCase || !gridCase->occupied)
        continue ;
      posX = static_cast<qreal>(x - minX) * 10;
      posY = static_cast<qreal>(y - minY) * 10;
      if (lineIntersectsRect(sightLine, QRectF(posX, posY, caseSize, caseSize)))
      {
        if (gridCase->occupant)
          visionScore -= gridCase->occupant->getCoverValue();
        else
          visionScore = 0;
        if (visionScore <= 0)
          return std::max(0, visionScore);
      }
    }
  }
  return std::max(0, visionScore);
}

void LevelGrid::removeObject(DynamicObject* object)
{
  auto  position = object->getPosition();
  auto* gridCase = getGridCase(position.x(), position.y());

  if (gridCase && gridCase->occupant == object)
  {
    setCaseOccupant(*gridCase, nullptr);
    for (auto* zone : gridCase->zones)
    {
      if (object->isCharacter())
        reinterpret_cast<Character*>(object)->onZoneExited(zone);
      emit zone->exitedZone(object, zone);
    }
  }
}

bool LevelGrid::moveObject(DynamicObject* object, int x, int y)
{
  auto* gridCase = getGridCase(x, y);

  if (object->isBlockingPath() && gridCase)
  {
    QPoint currentPosition = object->getPosition();
    auto*  oldCase = getGridCase(currentPosition.x(), currentPosition.y());

    if (oldCase)
      setCaseOccupant(*oldCase, nullptr);
    setCaseOccupant(*gridCase, object);
    object->setPosition(QPoint(x, y));
    return true;
  }
  else if (gridCase)
    object->setPosition(QPoint(x, y));
  return gridCase != nullptr;
}

void LevelGrid::triggerZone(CharacterMovement* character, int x, int y)
{
  static const QVector<TileZone*> emptyZoneList;
  auto* gridCase = getGridCase(x, y);
  const QVector<TileZone*>  lastZones = character->getCurrentZones();
  const QVector<TileZone*>& newZones  = gridCase ? gridCase->zones : emptyZoneList;

  for (auto* zone : newZones)
  {
    if (!(character->isInZone(zone)))
      emit zone->enteredZone(character, zone);
  }
  for (auto* zone : lastZones)
  {
    if (!(zone->isInside(x, y)))
      emit zone->exitedZone(character, zone);
  }
}
