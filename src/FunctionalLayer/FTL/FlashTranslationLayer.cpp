#include "FlashTranslationLayer.hpp"
#include "FTLs/Pagemap.hpp"
#include "FTLs/Fast.hpp"
#include "FTLs/Dftl.hpp"
#include "FTLs/Hftl.hpp"
#include "FTLs/Last.hpp"

FlashTranslationLayer *FlashTranslationLayer::_singleton = NULL;

FlashTranslationLayer* FlashTranslationLayer::getInstance ()
{
  if(_singleton == NULL)
  {
    if(!Param::getInstance()->getString("functional_model.ftl.type").compare("pagemap"))
      _singleton = new Pagemap();
    else if(!Param::getInstance()->getString("functional_model.ftl.type").compare("fast"))
      _singleton = new Fast();
    else if(!Param::getInstance()->getString("functional_model.ftl.type").compare("last"))
      _singleton = new Fast();
    else if(!Param::getInstance()->getString("functional_model.ftl.type").compare("dftl"))
      _singleton = new Fast();
    else if(!Param::getInstance()->getString("functional_model.ftl.type").compare("hftl"))
      _singleton = new Fast();
    else
      ERROR("unknown ftl type ...");
  }

  return _singleton;
}

void FlashTranslationLayer::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

FlashTranslationLayer::~FlashTranslationLayer ()
{
}

PpcValF2 FlashTranslationLayer::flashLegacyRead (Address address)
{
  PpcValF2 res = {0, 0};

  LegacyRead lr(address);

  res.time = lr.getTimeTaken();
  res.power_consumption = lr.getEnergyConsumed();

  return res;
}

PpcValF2 FlashTranslationLayer::flashLegacyWrite (Address address)
{
  PpcValF2 res = {0, 0};

  LegacyWrite lw(address);

  res.time = lw.getTimeTaken();
  res.power_consumption = lw.getEnergyConsumed();

  return res;
}

PpcValF2 FlashTranslationLayer::flashLegacyErase (Address address)
{
  PpcValF2 res = {0, 0};

  LegacyErase le(address);

  res.time = le.getTimeTaken();
  res.power_consumption = le.getEnergyConsumed();

  return res;
}
