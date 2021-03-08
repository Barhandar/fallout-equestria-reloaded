#ifndef CHARACTERMOVEMENT_H
#define CHARACTERMOVEMENT_H

#include "../storageobject.h"
#define CHARACTER_BASE_OBJECT StorageObject

class CharacterMovement : public CHARACTER_BASE_OBJECT
{
  Q_OBJECT

  Q_PROPERTY(QString orientation READ getOrientation)
  Q_PROPERTY(QString movementMode READ getMovementMode WRITE setMovementMode)
public:
  explicit CharacterMovement(QObject *parent = nullptr);

  virtual void load(const QJsonObject&);
  virtual void save(QJsonObject&) const;

  void             setAnimation(const QString& animationName) override;
  void             moveTo(int x, int y, QPoint renderPosition);
  Q_INVOKABLE void lookTo(int x, int y);

  const QString&       getMovementMode() const { return movementMode; }
  void                 setMovementMode(const QString&);
  const QString&       getOrientation() const { return orientation; }
  const QList<QPoint>& getCurrentPath() const { return currentPath; }
  QList<QPoint>&       rcurrentPath() { return currentPath; }

public slots:
  void onIdle();

signals:
  void reachedDestination();
  void pathBlocked();

private slots:
  void onMovementEnded();
  void onDestinationReached();

protected:
  QList<QPoint> currentPath;
  QString       orientation;
  QString       movementMode;
};

#endif // CHARACTERMOVEMENT_H