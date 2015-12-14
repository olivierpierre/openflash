/*
 * Simulation.hpp
 *
 *  Created on: 8 avr. 2014
 *      Author: pierre
 */

#ifndef SIMULATION_HPP
#define SIMULATION_HPP

#include <string>

#include "Param.hpp"
#include "ErrorManager.hpp"
#include "RanDist.hpp"
#include "../FlashLayer/FlashLayer.hpp"
#include "../FunctionalLayer/FunctionalLayer.hpp"
#include "../TimeEvents/TimeEvents.hpp"

class OutputPool;

using namespace std;

extern ErrorManager *em;

typedef enum
{
  FFS_MODE,
  FTL_MODE
} simulation_mode_t;

typedef enum
{
  FILE_MODE,
  GENERATOR_MODE
} trace_mode_t;

class Simulation
{
public:
  static Simulation * getInstance();
  static void kill();
  int run(string paramFile);
  int output();


private:
  void init(string paramFile);

  simulation_mode_t _simulation_mode;
  trace_mode_t _trace_mode;

  // General simulation objects
  Param *_parameters;
  ErrorManager *_error_manager;
  FlashSystem *_flash_layer;
  EventProcessor *_event_processor;
  TimeMgr *_time_manager;
  OutputPool *_output_pool;
  RanDist *_random_distributions;

  // FFS specifics
  NandDriver *_nand_driver;
  FlashFileSystem *_flash_file_system;
  VirtualFileSystem *_virtual_file_system;
  PageCache *_page_cache;

  //FTL specifics
  FtlSystem *_ftl_system;

  //Event sources
  TraceParser *_trace_parser;
  Generator *_generator;

  // singleton stuff
  static Simulation *_singleton;
  Simulation ();
  virtual ~Simulation ();

friend class WearLevelingOutput;
friend class OutputPool;


};

#include "../Outputs/Output.hpp"

#endif /* SIMULATION_HPP */
