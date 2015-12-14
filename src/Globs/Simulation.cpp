/*
 * Simulation.cpp
 *
 *  Created on: 8 avr. 2014
 *      Author: pierre
 */

#include "Simulation.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

Simulation * Simulation::_singleton = NULL;

Simulation::Simulation() : _simulation_mode(FFS_MODE), _trace_mode(FILE_MODE), _parameters(NULL), _error_manager(NULL),
    _flash_layer(NULL), _event_processor(NULL),
    _time_manager(NULL), _output_pool(NULL), _random_distributions(NULL),
    _nand_driver(NULL), _flash_file_system(NULL),
    _virtual_file_system(NULL), _page_cache(NULL),
    _ftl_system(NULL), _trace_parser(NULL), _generator(NULL) {}

Simulation * Simulation::getInstance()
{
  if(_singleton == NULL)
    _singleton = new Simulation();

  return _singleton;
}

void Simulation::kill()
{
  if(_singleton != NULL)
  {
    delete _singleton;
    _singleton = NULL;
  }
}

void Simulation::init(string paramFilePath)
{
  /* check if config file exists */
  struct stat buffer;
  if(!(stat (paramFilePath.c_str(), &buffer) == 0))
  {
      cerr << "Error can't find configuration file " << paramFilePath;
      exit(-1);
  }

  /* pre-init error manager and init parameters from param file */
  _error_manager = ErrorManager::getInstance();
  _parameters = Param::getInstance(paramFilePath);

  /* init general simulation objects */
  _event_processor = EventProcessor::getInstance();
  _time_manager = TimeMgr::getInstance();
  _flash_layer = FlashSystem::getInstance();
  _random_distributions = RanDist::getInstance();

  string sim_mode = _parameters->getString("functional_model.functional_mode");
  if(!sim_mode.compare("ffs"))
  {
    /* ffs related objects */
    _page_cache = PageCache::getInstance();
    _flash_file_system = FlashFileSystem::getInstance();
    _virtual_file_system = VirtualFileSystem::getInstance();
    _nand_driver = NandDriver::getInstance();

    _simulation_mode = FFS_MODE;
  }
  else if(!sim_mode.compare("ftl"))
  {
    /* ftl related objects */
    _ftl_system = FtlSystem::getInstance();

    _simulation_mode = FTL_MODE;
  }
  else
    ERROR("Unknown mode " + sim_mode);

  string trace_mode = Param::getInstance()->getString("simulator.trace.mode");
  if(!trace_mode.compare("file"))
  {
    _trace_mode = FILE_MODE;
    _trace_parser = TraceParser::getInstance();
  }
  else if(!trace_mode.compare("generator"))
  {
    _trace_mode = GENERATOR_MODE;
    _generator = Generator::getInstance();
  }

  /* init the outputs in the end because they
   * may want to request infos from other
   * simulator submodules so we want them to
   * be initialized first
   */
  _output_pool = OutputPool::getInstance();
}

int Simulation::run (string paramFile)
{
  Event *e;

  init(paramFile);


  if(_trace_mode == FILE_MODE)
    while((e = _trace_parser->getNextEvent()) != NULL)
    {
      _event_processor->insertWorkloadEvent(*e);
      delete e;
    }
  else
    while((e = _generator->getNextEvent()) != NULL)
    {
      _event_processor->insertWorkloadEvent(*e);
      delete e;
    }

  return 0;
}

Simulation::~Simulation()
{
  _event_processor->kill();
  _time_manager->kill();
  _output_pool->kill();
  _random_distributions->kill();

  if(_simulation_mode == FFS_MODE)
  {
    /* ffs related objects */
    _flash_file_system->kill();
    _virtual_file_system->kill();
    _nand_driver->kill();
  }
  else
  {
    /* ftl related objects */
    _ftl_system->kill();
  }

  if(_trace_mode == FILE_MODE)
    _trace_parser->kill();
  else
    _generator->kill();

  _flash_layer->kill();

  _parameters->kill();
  _error_manager->kill();
}

int Simulation::output ()
{
  return _output_pool->output();
  // archive ?
}
