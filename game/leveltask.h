#ifndef  LEVELTASK_H
# define LEVELTASK_H

# include <QObject>
# include <QTimer>
# include "tilemap/tilemap.h"
# include "levelgrid.h"

# include "dynamicobject.h"

class LevelTask : public QObject
{
  Q_OBJECT

  Q_PROPERTY(bool     paused MEMBER paused NOTIFY pausedChanged)
  Q_PROPERTY(TileMap* tilemap READ getTileMap NOTIFY tilemapReady)
public:
  explicit LevelTask(QObject *parent = nullptr);

  void load(const QString& levelName);

  TileMap* getTileMap() const { return tilemap; }

  Q_INVOKABLE void moveTo(int x, int y);
  Q_INVOKABLE void moveCharacterTo(DynamicObject*, int x, int y);
  Q_INVOKABLE DynamicObject* getOccupantAt(int x, int y);

  void triggerCharacterMoveTo(DynamicObject* character, int x, int y);
  void forceCharacterPosition(DynamicObject* chracter, int x, int y);

signals:
  void pausedChanged();
  void tilemapReady();

private slots:
  void onPauseChanged();
  void onClockTick();
  void onTaskTick();

private:
  DynamicObject* player;

  QTimer     taskTick, clockTick;
  TileMap*   tilemap = nullptr;
  LevelGrid* grid = nullptr;
  bool       paused = true;
};

#endif // LEVELTASK_H
