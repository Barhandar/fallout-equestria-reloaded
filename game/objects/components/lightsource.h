#ifndef  LIGHTSOURCECOMPONENT_H
# define LIGHTSOURCECOMPONENT_H

# include "detectable.h"

class TileLayer;

class LightSourceComponent : public DetectableComponent
{
  Q_OBJECT
  typedef DetectableComponent ParentType;

  Q_PROPERTY(int lightRadius MEMBER lightRadius NOTIFY lightRadiusChanged)
  Q_PROPERTY(int lightType MEMBER lightType NOTIFY lightTypeChanged)
public:
  explicit LightSourceComponent(QObject *parent = nullptr);

  void load(const QJsonObject&);
  void save(QJsonObject&) const;

  TileLayer* getLightZone() const { return lightZone; }

signals:
  void lightRadiusChanged();
  void lightTypeChanged();
  void lightZoneRemoved(TileLayer*);
  void lightZoneAdded(TileLayer*);

public slots:
  void reloadLightzone();
  void refreshLightzone();

private:
  void drawLightzone();

  int lightRadius = 0;
  int lightType = 0;
  TileLayer* lightZone = nullptr;
};

#endif // LIGHTSOURCECOMPONENT_H
