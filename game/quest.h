#ifndef QUEST_H
#define QUEST_H

#include <QObject>
#include "scriptcontroller.h"
#include "utils/storableobject.h"

class Character;
class InventoryItem;

class Quest : public StorableObject
{
  Q_OBJECT

  Q_PROPERTY(QString     name        MEMBER name         NOTIFY nameChanged)
  Q_PROPERTY(bool        completed   MEMBER completed    NOTIFY completedChanged)
  Q_PROPERTY(bool        failed      MEMBER failed       NOTIFY completedChanged)
  Q_PROPERTY(bool        inProgress  READ inProgress     NOTIFY completedChanged)
  Q_PROPERTY(QString     description READ getDescription NOTIFY descriptionChanged)
public:
  enum ObjectiveState { InProgress = 0, Done, Failed };

  explicit Quest(QObject *parent = nullptr);
  ~Quest();

  void initialize(const QString& name);
  void load(const QJsonObject&);
  QJsonObject save() const;

  Q_INVOKABLE void completeObjective(const QString&);
  Q_INVOKABLE bool isObjectiveCompleted(const QString&) const;
  inline bool inProgress() const { return !completed && !failed; }

  Q_INVOKABLE QVariantList getObjectives() const;
  Q_INVOKABLE QJSValue getScriptObject() const;
  Q_INVOKABLE QString tr(const QString& key, const QVariantMap& options = {});
  QString getDescription() const;

public slots:
  void onCharacterKilled(Character* victim, Character* killed);
  void onItemPicked(InventoryItem*);

signals:
  void nameChanged();
  void completedChanged();
  void descriptionChanged();

private slots:
  void onCompletedChanged();

private:
  QString           name;
  bool              completed, failed;
  ScriptController* script = nullptr;
};

#endif // QUEST_H
