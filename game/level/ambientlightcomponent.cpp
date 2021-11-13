#include "ambientlightcomponent.h"
#include "game.h"

static QMap<int, QColor> daylightColors = {
  {0,  QColor("#00004F")},
  {6,  QColor("#6F202F")},
  {7,  QColor("#5F3F2F")},
  {8,  QColor("#3F3F3F")},
  {10, QColor("#4F4F4F")},
  {18, QColor("#3F3F4F")},
  {21, QColor("#2F1F4F")},
  {22, QColor("#00004F")}
};

AmbientLightComponent::AmbientLightComponent(QObject *parent) : ParentType(parent)
{
  ambientColor = QColor(255, 255, 255, 0);
}

void AmbientLightComponent::update(qint64 delta)
{
  if (usesDaylight())
    updateDaylight();
  ParentType::update(delta);
}

void AmbientLightComponent::updateDaylight()
{
  TimeManager* timeManager = Game::get()->getTimeManager();
  QColor       newColor;

  for (auto it = daylightColors.begin() ; it != daylightColors.end() ; ++it)
  {
    if (it.key() <= timeManager->getHour())
      newColor = it.value();
  }
  if (ambientColor != newColor)
  {
    ambientColor = newColor;
    emit ambientColorChanged();
  }
}
