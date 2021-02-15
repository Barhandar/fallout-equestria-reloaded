#ifndef  DYNAMICOBJECT_H
# define DYNAMICOBJECT_H

# include "globals.h"
# include <QObject>
# include <QPoint>
# include <QJSValue>
# include <QTimer>
# include <QJsonObject>
# include "sprite.h"
# include "taskrunner.h"

class TileZone;

class DynamicObject : public Sprite
{
  Q_OBJECT

  Q_PROPERTY(QString objectName MEMBER objectName NOTIFY objectNameChanged)
  Q_PROPERTY(QPoint  position    READ getPosition)
  Q_PROPERTY(QString currentZone READ getCurrentZone)
  Q_PROPERTY(TaskRunner* tasks MEMBER taskManager)
  Q_PROPERTY(bool floating MEMBER floating NOTIFY floatingChanged)
  Q_PROPERTY(TileZone* controlZone MEMBER controlZone NOTIFY controlZoneChanged)

public:
  explicit DynamicObject(QObject *parent = nullptr);

  void update(qint64);

  virtual void load(const QJsonObject&);
  virtual void save(QJsonObject&) const;

  inline bool isCharacter() const { return getObjectType() == "Character"; }
  inline bool isFloating() const { return floating; }
  virtual bool isBlockingPath() const { return true; }

  void setObjectName(const QString& value) { objectName = value; emit objectNameChanged(); }
  const QString& getObjectName() const { return objectName; }
  void setScript(const QString& name);
  TaskRunner* getTaskManager() { return taskManager; }

  Q_INVOKABLE bool     hasVariable(const QString& name) const { return dataStore.contains(name); }
  Q_INVOKABLE QVariant getVariable(const QString& name) const { return dataStore[name].toVariant(); }
  Q_INVOKABLE void     setVariable(const QString& name, const QVariant& value) { dataStore.insert(name, QJsonValue::fromVariant(value)); }
  Q_INVOKABLE void     unsetVariable(const QString& name) { dataStore.remove(name); }

  Q_INVOKABLE QString getObjectType() const { return metaObject()->className(); }
  Q_INVOKABLE QPoint getPosition() const { return position; }
  Q_INVOKABLE virtual QPoint getInteractionPosition() const { return interactionPosition; }
  virtual QStringList getAvailableInteractions();
  void setPosition(QPoint value) { position = value; }
  void setInteractionPosition(QPoint value) { interactionPosition = value; }
  void moveTo(int x, int y, QPoint renderPosition);

  Q_INVOKABLE TileZone* addControlZone();
  Q_INVOKABLE void      removeControlZone();
  TileZone*             getControlZone() { return controlZone; }

  Q_INVOKABLE void      scriptCall(const QString& method, const QString& message = "");

  const QList<QPoint>& getCurrentPath() const { return currentPath; }
  QList<QPoint>& rcurrentPath() { return currentPath; }

  const QString& getCurrentZone() const { return currentZone; }
  void setCurrentZone(const QString& value) { currentZone = value; }

signals:
  void objectNameChanged();
  void reachedDestination();
  void pathBlocked();
  void floatingChanged();
  void controlZoneChanged();
  void controlZoneAdded(TileZone*);
  void controlZoneRemoved(TileZone*);

private slots:
  void onMovementEnded();
  void onDestinationReached();

protected:
  virtual QString getScriptPath() const { return SCRIPTS_PATH + "behaviours"; }
  QJSValue script;
  TaskRunner* taskManager;
  TileZone* controlZone = nullptr;
private:
  QString objectName, scriptName;
  QPoint position, nextPosition;
  bool floating;
  QList<QPoint> currentPath;
  QString currentZone;
  QPoint interactionPosition;
  QJsonObject dataStore;
};

#endif // DYNAMICOBJECT_H
