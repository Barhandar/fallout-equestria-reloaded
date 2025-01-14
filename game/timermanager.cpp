#include "timermanager.h"
#include <QJsonObject>

TimeManager::TimeManager(QObject *parent) : QObject(parent)
{
  dateTime = DateTime(2011, 1, 26, 8, 30, 0);
}

void TimeManager::save(QJsonObject& data) const
{
  data["y"] = dateTime.GetYear();
  data["M"] = dateTime.GetMonth();
  data["d"] = dateTime.GetDay();
  data["h"] = dateTime.GetHour();
  data["m"] = dateTime.GetMinute();
  data["s"] = dateTime.GetSecond();
}

void TimeManager::load(const QJsonObject& data)
{
  dateTime = DateTime(
    data["y"].toInt(), data["M"].toInt(), data["d"].toInt(),
    data["h"].toInt(), data["m"].toInt(), data["s"].toInt()
  );
}

void TimeManager::addElapsedMilliseconds(qint64 value)
{
  milliseconds += value;
  if (milliseconds > 1000)
  {
    qint64 seconds = milliseconds / 1000;

    if (seconds > 0)
      addElapsedTime(DateTime::Seconds(static_cast<unsigned int>(seconds)));
    milliseconds %= 1000;
  }
}

void TimeManager::addElapsedTime(DateTime::TimeUnit time)
{
  dateTime = dateTime + time;
  emit dateChanged();
}

void TimeManager::addElapsedMinutes(int minutes)
{
  dateTime = dateTime + DateTime::Minutes(minutes);
  emit dateChanged();
}

long TimeManager::secondsUntilTime(const QVariantMap &timeData) const
{
  DateTime nextTime(
    dateTime.GetYear(), dateTime.GetMonth(), dateTime.GetDay(),
    timeData["hour"].toUInt(),
    timeData.contains("minute") ? timeData["minute"].toUInt() : 0,
    timeData.contains("second") ? timeData["second"].toUInt() : 0
  );
  std::time_t seconds;

  if (nextTime <= dateTime)
    nextTime = nextTime + DateTime::Days(1);
  seconds = (nextTime - dateTime).GetTimestamp();
  return static_cast<long>(seconds);
}
