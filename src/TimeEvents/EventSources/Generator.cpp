/*
 * Generator.cpp
 *
 *  Created on: 9 avr. 2014
 *      Author: pierre
 */

#include "Generator.hpp"
#include "../../Globs/Common.hpp"
#include "../../Globs/Param.hpp"
#include "../../Globs/ErrorManager.hpp"

Generator *Generator::_singleton = NULL;

Generator* Generator::getInstance ()
{
  if(_singleton == NULL)
  {
    if(!Param::getInstance()->getString("simulator.trace.generator_type").compare("fakegen"))
    {
      _singleton = new FakeGen();
    }
    else
      ERROR("Unknow synthetic generator" + Param::getInstance()->getString("simulator.trace.generator_type"));
  }

  return _singleton;
}

void Generator::kill ()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

Generator::Generator ()
{
  // TODO Auto-generated constructor stub

}

Generator::~Generator ()
{
  // TODO Auto-generated destructor stub
}

Event* FakeGen::getNextEvent ()
{
  return NULL;
}
