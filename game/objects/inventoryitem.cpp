#include "globals.h"
#include "inventoryitem.h"
#include "../inventoryitemlibrary.h"
#include "game.h"

InventoryItem::InventoryItem(QObject* parent) : DynamicObject(parent), quantity(1)
{
  blocksPath = false;
  connect(this, &InventoryItem::quantityChanged, this, &InventoryItem::weightChanged);
  connect(this, &InventoryItem::quantityChanged, this, &InventoryItem::valueChanged);
  connect(this, &InventoryItem::itemTypeChanged, this, &InventoryItem::updateScript);
  connect(this, &InventoryItem::itemTypeChanged, this, &InventoryItem::updateSprite);
  setSpriteName("items");
}

QString InventoryItem::getCategory() const
{
  auto itemData = InventoryItemLibrary::get()->getObject(itemType);

  if (itemData.isObject())
    return itemData["type"].toString("misc");
  return "weapon";
}

QStringList InventoryItem::getAvailableInteractions()
{
  auto list = DynamicObject::getAvailableInteractions();

  if (list.empty())
    list << "use" << "look" << "use-skill";
  return list;
}

int InventoryItem::getWeight() const
{
  auto itemData = InventoryItemLibrary::get()->getObject(itemType);

  if (itemData.isObject())
    return itemData["weight"].toInt(1) * getQuantity();
  return getQuantity();
}

int InventoryItem::getValue() const
{
  auto itemData = InventoryItemLibrary::get()->getObject(itemType);

  if (itemData.isObject())
    return itemData["value"].toInt(1);
  return 1;
}

bool InventoryItem::isGroupable(InventoryItem* other)
{
  auto itemData = InventoryItemLibrary::get()->getObject(itemType);
  bool result   = true;

  if (itemData.isObject())
    result = itemData["isGroupable"].toBool(result);
  if (script && script->hasMethod("isGroupable"))
  {
    QJSValueList args;

    args << other->asJSValue()
         << result;
    result = script->call("isGroupable", args).toBool();
  }
  return result;
}

QStringList InventoryItem::getUseModes() const
{
  if (script)
  {
    QJSValue useModes = script->property("useModes");

    if (useModes.isArray())
      return script->property("useModes").toVariant().toStringList();
  }
  return QStringList() << "use";
}

bool InventoryItem::requiresTarget() const
{
  if (script)
    return script->property("requiresTarget").toBool();
  return true;
}

void InventoryItem::add(int amount)
{
  quantity += amount;
  emit quantityChanged();
}

bool InventoryItem::remove(int amount)
{
  if (quantity > amount)
  {
    quantity -= amount;
    emit quantityChanged();
    return true;
  }
  return false;
}

void InventoryItem::onEquippedBy(Character* user, bool on)
{
  resetUseMode();
  if (script && user)
  {
    QJSValueList args;

    args << user->asJSValue() << on;
    script->call("onEquipped", args);
  }
}

bool InventoryItem::canEquipInSlot(const QString& slotType)
{
  if (script && script->hasMethod("canEquipInSlotType"))
  {
    QJSValueList args;

    args << slotType;
    return script->call("canEquipInSlotType", args).toBool();
  }
  return  slotType == "any";
}

int InventoryItem::getActionPointCost()
{
  if (script && script->hasMethod("getActionPointCost"))
    return script->call("getActionPointCost").toInt();
  return 2;
}

bool InventoryItem::isCombatItem()
{
  QJSValue value;

  if (script)
    value = script->property("triggersCombat");
  return value.isBool() ? value.toBool() : false;
}

bool InventoryItem::isInRange(DynamicObject *target)
{
  if (target && script && script->hasMethod("isInRange"))
    return script->call("isInRange", QJSValueList() << target->asJSValue()).toBool();
  return true;
}

float InventoryItem::getRange() const
{
  if (script && script->hasMethod("getRange"))
    return static_cast<float>(script->call("getRange").toNumber());
  return 1.f;
}

bool InventoryItem::isValidTarget(DynamicObject* target)
{
  if (target && script && script->hasMethod("isValidTarget"))
    return script->call("isValidTarget", QJSValueList() << target->asJSValue()).toBool();
  return false;
}

QJSValue InventoryItem::useOn(DynamicObject* target)
{
  if (target && script && isValidTarget(target))
    return script->call("attemptToUseOn", QJSValueList() << target->asJSValue());
  return false;
}

void InventoryItem::useFromInventory()
{
  QJSValue result = useOn(getOwner());

  if (result.isObject())
  {
    QJSValue callback = result.property("callback");

    if (callback.isCallable())
      callback.call();
  }
}

void InventoryItem::setCountdown(int value)
{
  if (script && script->hasMethod("onCountdownReceived"))
    script->call("onCountdownReceived", QJSValueList() << value);
}

void InventoryItem::swapUseMode()
{
  QStringList useModes = getUseModes();
  int currentIndex = useModes.indexOf(useMode);

  switch (useModes.length())
  {
  case 0:
    useMode = "use";
    break ;
  case 1:
    useMode = useModes.first();
    break;
  default:
    if (currentIndex + 1 >= useModes.length() || currentIndex == -1)
      useMode = useModes.first();
    else
      useMode = useModes[currentIndex + 1];
    break ;
  }
  emit useModeChanged();
}

void InventoryItem::resetUseMode()
{
  QStringList useModes = getUseModes();

  useMode = useModes.length() > 0 ? useModes.first() : "use";
  emit useModeChanged();
}

int InventoryItem::getUseSuccessRate(DynamicObject* target)
{
  if (isValidTarget(target))
  {
    if (script && script->hasMethod("getUseSuccessRate"))
      return script->call("getUseSuccessRate", QJSValueList() << target->asJSValue()).toInt();
    else if (isInRange(target))
      return 95;
  }
  return 0;
}

DynamicObject* InventoryItem::getOwner() const
{
  const char* parentType = parent()->metaObject()->className();

  if (parentType == QString("Inventory"))
  {
    Inventory* inventory = reinterpret_cast<Inventory*>(parent());

    return inventory->getUser();
  }
  else if (parentType == QString("Character"))
    return reinterpret_cast<DynamicObject*>(parent());
  else
    qDebug() << "WARNING InventoryItem has no owner" << parent()->metaObject()->className();
  return nullptr;
}

void InventoryItem::updateScript()
{
  auto itemData = InventoryItemLibrary::get()->getObject(itemType);
  QString scriptName = itemType + ".mjs";

  if (itemData.isObject())
    scriptName = itemData["script"].toString(scriptName);
  setScript(scriptName);
  emit useModesChanged();
}

void InventoryItem::updateSprite()
{
  QString animationName = "any";
  auto itemData = InventoryItemLibrary::get()->getObject(itemType);

  if (itemData.isObject())
    animationName = itemData["sprite"].toString(animationName);
  setAnimation(animationName);
}

void InventoryItem::save(QJsonObject& data) const
{
  data["itemType"] = itemType;
  data["quantity"] = quantity;
  if (virtualItem)
    data["virtual"] = virtualItem;
  data["useMode"] = useMode;
  DynamicObject::save(data);
}

void InventoryItem::load(const QJsonObject& data)
{
  itemType = data["itemType"].toString();
  quantity = data["quantity"].toInt(1);
  virtualItem = data["virtual"].toBool();
  useMode = data["useMode"].toString();
  DynamicObject::load(data);
  emit quantityChanged();
  emit objectNameChanged();
  emit useModesChanged();
  emit useModeChanged();
}
