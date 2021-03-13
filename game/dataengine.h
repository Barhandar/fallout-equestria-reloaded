#ifndef DATAENGINE_H
#define DATAENGINE_H

#include <QObject>
#include "cmap/statmodel.h"

class QJsonDocument;

class DataEngine : public QObject
{
  Q_OBJECT
public:
  explicit DataEngine(QObject *parent = nullptr);

  Q_INVOKABLE void loadFromFile(const QString& path);
  Q_INVOKABLE void saveToFile(const QString &path);

  void         setCurrentLevel(const QString&);
  QString      getCurrentLevel() const;
  void         exitLevel();
  bool         isLevelActive(const QString&) const;
  QJsonObject  getLevelData(const QString&);
  void         setLevelData(const QString&, const QJsonObject&);

  QJsonObject  getTimeData() const;
  void         setTimeData(const QJsonObject&);

  QJsonObject  getPlayerParty() const;
  void         setPlayerParty(const QJsonObject&);

  QJsonObject  getWorldDiplomacy() const;
  void         setWorldDiplomacy(const QJsonObject&);
  Q_INVOKABLE void registerFaction(const QString&);

  Q_INVOKABLE QStringList getFactionList() const;
  Q_INVOKABLE QStringList getEnemyListForFaction(const QString& faction) const;
  Q_INVOKABLE void setFactionAsEnemy(const QString& faction, const QString& enemyFaction, bool set = true);
  Q_INVOKABLE void setFactionReputationEnabled(const QString& faction, bool set);
  Q_INVOKABLE bool hasFactionReputationEnabled(const QString& faction) const;

  QJsonObject  getQuests() const;
  void         setQuests(const QJsonObject&);

  QJsonObject  getWorldmap() const;
  void         setWorldmap(const QJsonObject&);

  StatModel* makeStatModel(const QString& characterId, const QString& source = "");
  void       saveStatModel(const QString& characterId, StatModel*);

  void fromJson(const QString&);
  QString toJson() const;

  QJsonDocument asDocument() { return QJsonDocument(data); }

signals:
  void diplomacyUpdated();

private:
  QJsonObject data;
  QJsonObject levels, characters, time, diplomacy, quests, worldmap;
};

#endif // DATAENGINE_H
