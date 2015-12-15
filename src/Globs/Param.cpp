/*
 * Param2.cpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#include "Param.hpp"
#include "Common.hpp"

#include <iostream>
#include <libconfig.h++>
#include <cstring>
#include <string.h>
#include <cstdlib>
#include <cassert>

#include "ErrorManager.hpp"

using namespace libconfig;
using namespace std;

Param *Param::_singleton = NULL;

Param* Param::getInstance(string paramFilePath)
{
  if(_singleton == NULL && !paramFilePath.compare(""))
    ERROR("No parameter file specified");


  if (_singleton == NULL)
  {
    _singleton =  new Param(paramFilePath);
  }
  return _singleton;
}

void Param::kill()
{
  if (_singleton != NULL)
    {
      delete _singleton;
      _singleton = NULL;
    }
}

Param::Param(string paramFilePath) : _cfg(NULL), _paramFilePath(paramFilePath)
{
  _doBuildParams();
}
Param::~Param(){}

void Param::setInt(string param, int val)
{
  _integers[param] = val;
}

void Param::setBool(string param, bool val)
{
  _booleans[param] = val;
}

void Param::setDouble(string param, double val)
{
  _doubles[param] = val;
}

void Param::setString(string param, string val)
{
  _strings[param] = val;
}

int Param::getInt(string param)
{
  if(_integers.find(param) == _integers.end())
  {
    string errmsg = "Cannot find requested integer parameter \"" + param + "\"";
    ERROR(errmsg);
  }
  return _integers[param];
}

bool Param::getBool(string param)
{
  if(_booleans.find(param) == _booleans.end())
  {
    string errmsg = "Cannot find requested bool parameter \"" + param + "\"";
    ERROR(errmsg);
  }
  return _booleans[param];
}

double Param::getDouble(string param)
{
  if(_doubles.find(param) == _doubles.end())
  {
    string errmsg = "Cannot find requested double parameter \"" + param + "\"";
    ERROR(errmsg);
  }
  return _doubles[param];
}

string Param::getString(string param)
{
  if(_strings.size() == 0 || _strings.find(param) == _strings.end())
  {
    string errmsg = "Cannot find requested string parameter \"" + param + "\"";
    ERROR(errmsg);
  }
  return _strings[param];
}

int Param::_doBuildParams()
{
  _cfg = new Config();

  try
  {
    _cfg->readFile(_paramFilePath.c_str());
  }
  catch(const FileIOException &fioex)
  {
    cout << "I/O error when reading conf file " << _paramFilePath <<
      " ..." << endl;

    return -1;
  }
  catch(const ParseException &pex)
  {
    cout << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    return -1;
  }

  int res = doParse();

  delete _cfg;

  return res;
}

int Param::parseInt(string param, bool optional, bool *parsed)
{
  int res = -1;

  if(optional && parsed == NULL)
    ERROR2("Parsing " + param + ", when optional, *parsed should not be "
           "NULL please provide a valued", __FILE__, __LINE__);

  try
  {
    res = _cfg->lookup(param);
    _integers[param] = res;
  }
  catch(const SettingNotFoundException &nfex)
  {
    if(!optional)
    {
      ERROR("Missing mandatory integer parameter \"" + param + "\" in " + _paramFilePath);
      return -1;
    }
    else
    {
      *parsed = false;
      return -1;
    }
  }

  if(optional)
    *parsed = true;

  return res;
}

bool Param::parseBool(string param, bool optional, bool *parsed)
{
  bool res = true;

  if(optional && parsed == NULL)
    ERROR2("Parsing " + param + ", when optional, *parsed should not be "
           "NULL please provide a valued", __FILE__, __LINE__);

  try
  {
    res = _cfg->lookup(param);
    _booleans[param] = res;
  }
  catch(const SettingNotFoundException &nfex)
  {
    if(!optional)
      ERROR("Missing mandatory boolean parameter \"" + param + "\" in " + _paramFilePath);
    else
    {
      *parsed = false;
      return false;
    }
  }

  if(optional)
    *parsed = true;

  return res;
}

double Param::parseDouble(string param, bool optional, bool *parsed)
{
  double res = -1;

  if(optional && parsed == NULL)
    ERROR2("Parsing " + param + ", when optional, *parsed should not be "
           "NULL please provide a valued", __FILE__, __LINE__);

  try
  {
    res = _cfg->lookup(param);
    _doubles[param] = res;
  }
  catch(const SettingNotFoundException &nfex)
  {
    if(!optional)
      ERROR("Missing mandatory double parameter \"" + param + "\" in " + _paramFilePath);
    else
    {
      *parsed = false;
      return -1;
    }
  }
  catch(SettingTypeException &tex)
  {
    try
    {
      int resInt = _cfg->lookup(param);
      res = (double) resInt;
      _doubles[param] = res;
    }
    catch(const SettingNotFoundException &nfex)
    {
      if(!optional)
	ERROR("Missing mandatory double parameter \"" + param + "\" in " + _paramFilePath);
      else
      {
	*parsed = false;
	return -1;
      }
    }
  }

  if(optional)
    *parsed = true;

  return res;
}

string Param::parseString(string param, bool optional, bool *parsed)
{
  const char *res = "";

  if(optional && parsed == NULL)
    ERROR2("Parsing " + param + ", when optional, *parsed should not be "
           "NULL please provide a valued", __FILE__, __LINE__);

  try
  {
    res = _cfg->lookup(param);
    _strings[param] = res;
  }
  catch(const SettingNotFoundException &nfex)
  {
    if(!optional)
      ERROR("Missing string parameter \"" + param + "\" in " + _paramFilePath);
    else
    {
      *parsed = false;
      return "";
    }
  }

  assert(strcmp(res, ""));

  if(optional)
    *parsed = true;

  return res;
}

void Param::dumpParams()
{
  cout << "Dumping parameters :" << endl;
  cout << "==========" << endl;
  cout << "Integers :" << endl;
  cout << "==========" << endl;

  for( map<string, int>::iterator ii=_integers.begin(); ii!=_integers.end(); ++ii)
  {
     cout << (*ii).first << " = " << (*ii).second << endl;
  }

  cout << "==========" << endl;
  cout << "Booleans :" << endl;
  cout << "==========" << endl;

  for( map<string, bool>::iterator ii=_booleans.begin(); ii!=_booleans.end(); ++ii)
  {
     cout << (*ii).first << " = " << (*ii).second << endl;
  }

  cout << "=========" << endl;
  cout << "Doubles :" << endl;
  cout << "=========" << endl;

  for( map<string, double>::iterator ii=_doubles.begin(); ii!=_doubles.end(); ++ii)
  {
     cout << (*ii).first << " = " << (*ii).second << endl;
  }

  cout << "=========" << endl;
  cout << "Strings :" << endl;
  cout << "=========" << endl;

  for( map<string, string>::iterator ii=_strings.begin(); ii!=_strings.end(); ++ii)
  {
     cout << (*ii).first << " = " << (*ii).second << endl;
  }

}

int Param::parsePpcModels()
{
  vector<string> bases;
  bases.push_back("ppc_models.performance_model.TON");
  bases.push_back("ppc_models.performance_model.TIN");
  bases.push_back("ppc_models.performance_model.IO");
  bases.push_back("ppc_models.performance_model.BERS");
  bases.push_back("ppc_models.power_consumption_model.PTIN");
  bases.push_back("ppc_models.power_consumption_model.PTON");
  bases.push_back("ppc_models.power_consumption_model.PIO");
  bases.push_back("ppc_models.power_consumption_model.PBERS");


  for(vector<string>::const_iterator p = bases.begin(); p != bases.end(); ++p)
  {
    try
    {
      Setting& set = _cfg->lookup(*p);
      if(!set.isList())                   /* const val */
      {
        double res;
        _strings[*p+".mode"] = "const";

        try
        {
         res = _cfg->lookup(*p);
        }
        catch(const SettingTypeException ex)
        {
          int res2 = _cfg->lookup(*p);
          res = res2;
        }
        _doubles[*p+".const_val"] = res;
      }
      else                                    /* list */
      {
        double val_1, val_2;
        Setting& set = _cfg->lookup(*p);
        string mode = set[0];
        _strings[*p+".mode"] = mode;

        try
        {
          val_1 = set[1];
        }
        catch(SettingTypeException ex)
        {
          int val12 = set[1];
          val_1 = val12;
        }

        try
        {
          val_2 = set[2];
        }
        catch(SettingTypeException ex)
        {
          int val22 = set[2];
          val_2 = val22;
        }

        _doubles[*p+".val_1"] = val_1;
        _doubles[*p+".val_2"] = val_2;

      }
    }
    catch(const SettingNotFoundException &nfex)
    {
      ERROR("Missing parameter : " + *p);
    }
  }

  for(vector<string>::const_iterator p = bases.begin(); p != bases.end(); ++p)
    if(!_strings[*p+".mode"].compare("min_max") ||
        !_strings[*p+".mode"].compare("normal") ||
        !_strings[*p+".mode"].compare("exponential"))
      parseInt("ppc_models.random_seed");

  return 0;
}

/**
 * To add a parameter simply add a line here according to the parameter name
 * and type. It then will be accessible from everywhere through getInt,
 * getDouble, etc., according to its type.
 */
int Param::doParse()
{
  string string_val;
  bool parsed;

  /* flash layer */
  parseFlashLayerParameters();

  /* trace */
  string_val = parseString("simulator.trace.mode");
  if(!string_val.compare("file"))
  {
    string_val = parseString("simulator.trace.parser");
    string_val = parseString("simulator.trace.path");
  }
  else if(!string_val.compare("generator"))
    ERROR("Generator mode not implemented yet");
  else
    ERROR("Unknown input trace mode " + string_val);

  /* print progress */
	parseBool("simulator.print_progress", true, &parsed);
	if(!parsed)
	  _booleans["simulator.print_progress"] = false;

  /* random seed */
  parseInt("simulator.random_seed");

  /* Functional layer */
  string functional_mode = parseString("functional_model.functional_mode");
  if(!functional_mode.compare("ffs"))
    parseFfsParameters();
  else if(!functional_mode.compare("ftl"))
    parseFtlParameters();
  else
    ERROR("mode " + functional_mode + " unknown");

  /* Outputs */
  parseOutputParameters();


  sanityCheck();

  return 0;
}

int Param::parseFlashLayerParameters ()
{
  int int_val;
  bool bool_val, parsed;
  (void)int_val; (void)bool_val;

  /* structural params */
  int_val = parseInt("flash_layer.blocks_per_plane");
  int_val = parseInt("flash_layer.planes_per_lun");
  int_val = parseInt("flash_layer.luns_per_channel");
  int_val = parseInt("flash_layer.channels");
  int_val = parseInt("flash_layer.pages_per_block");

  int_val = parseInt("flash_layer.page_size_bytes");
  int_val = parseInt("flash_layer.oob_size_bytes");
  int_val = parseInt("flash_layer.channel_width_bits");

  /* flash functional params */
  bool_val = parseBool("functional_model.copyback_even_odd_constraint", true, &parsed);
  if(!parsed)
    _booleans["functional_model.copyback_even_odd_constraint"] = true;

  bool_val = parseBool("functional_model.multiplane_same_block", true, &parsed);
  if(!parsed)
    _booleans["functional_model.multiplane_same_block"] = true;

  /* performance and power con. params */
  parsePpcModels();

  return 0;
}


int Param::parseFfsParameters ()
{
  /** FFS type */
  string ffs_type = parseString("functional_model.ffs.type");
  if(!ffs_type.compare("JFFS2"))
    parseJffs2Parameters();
  else
    ERROR("FFS " + ffs_type + " unknown");

  // Linux nand driver power con. and time model
  parseDouble("ppc_models.nand_driver.read_time_overhead");
  parseDouble("ppc_models.nand_driver.write_time_overhead");
  parseDouble("ppc_models.nand_driver.erase_time_overhead");
  parseDouble("ppc_models.nand_driver.read_mem_power");
  parseDouble("ppc_models.nand_driver.write_mem_power");
  parseDouble("ppc_models.nand_driver.erase_mem_power");
  parseDouble("ppc_models.nand_driver.read_cpu_power");
  parseDouble("ppc_models.nand_driver.write_cpu_power");
  parseDouble("ppc_models.nand_driver.erase_cpu_power");

  //VFS
  parseInt("functional_model.vfs.page_cache_size_bytes");
  parseDouble("functional_model.vfs.pdflush_frequency");
  parseBool("functional_model.vfs.read_ahead");
  parseBool("functional_model.vfs.read_ahead_sequential_cache_miss_bug");

  parseDouble("functional_model.vfs.base_readpage_cpu_energy_overhead");
  parseDouble("functional_model.vfs.base_readpage_mem_energy_overhead");
  parseDouble("functional_model.vfs.base_writepage_cpu_energy_overhead");
  parseDouble("functional_model.vfs.base_writepage_mem_energy_overhead");
  parseDouble("functional_model.vfs.base_readpage_overhead");

  return 0;
}

int Param::parseFtlParameters ()
{
  string ftl_type = parseString("functional_model.ftl.type");
  string buffer_type = parseString("functional_model.ftl.flash_buffer");

  /* parse FTL params */
  if(!ftl_type.compare("pagemap"))
    parsePagemapParameters();
  else
    ERROR("FTL " + ftl_type + " unknown");

  /* parse buffer params */
  if(!buffer_type.compare("bplru"))
    parseBplruParameters();
  else if(buffer_type.compare("none"))
    ERROR("Flash buffer " + buffer_type + " unknown");

  return 0;
}

int Param::parseJffs2Parameters()
{
  parseBool("functional_model.ffs.jffs2.check_after_erase");
  parseDouble("functional_model.ffs.jffs2.base_readpage_timing_overhead");
  parseDouble("functional_model.ffs.jffs2.base_write_end_timing_overhead");
  parseDouble("functional_model.ffs.jffs2.base_write_begin_timing_overhead");

  bool gcd_on, parsed;
  gcd_on = parseBool("functional_model.ffs.jffs2.bg_thread", true, &parsed);
  if(parsed && gcd_on)
  {
    parseDouble("functional_model.ffs.jffs2.bg_thread_inter_arrival_exponential_rate");
    parseDouble("functional_model.ffs.jffs2.bg_thread_inter_arrival_exponential_base");
  }

  parseBool("functional_model.ffs.jffs2.read_check", true, &parsed);
  if(!parsed)
    _booleans["functional_model.ffs.jffs2.read_check"] = false;

  return 0;
}

int Param::parsePagemapParameters()
{
  parseInt("functional_model.ftl.pagemap.gc_threshold");
  return 0;
}

int Param::parseBplruParameters()
{
  parseInt("functional_model.ftl.bplru.size_blocks");

  return 0;
}

int Param::parseOutputParameters ()
{
  bool parsed;

  parseString("outputs.wear_leveling", true, &parsed);
  parseString("outputs.jffs2", true, &parsed);
  parseString("outputs.mtd", true, &parsed);
  parseString("outputs.vfs", true, &parsed);

  return 0;
}

/**
 * Launched just after the param object is built to do various checks
 */
void Param::sanityCheck ()
{
  /* we dont want a ffs trace on ftl mode and conversely */
  if(	!getString("functional_model.functional_mode").compare("ftl") &&
	!getString("simulator.trace.parser").compare("ffs_csv"))
    ERROR("ffs trace file format in ftl mode ...");

  if(	!getString("functional_model.functional_mode").compare("ffs") &&
  	!getString("simulator.trace.parser").compare("ftl_ascii"))
      ERROR("ftl trace file format in ffs mode ...");

  /* outputs : no ffs & mtd outputs on ftl mode */
  if(!getString("functional_model.functional_mode").compare("ftl"))
  {
    if(isParamPresent("outputs.jffs2"))
    {
      WARNING("Jffs2 output should not be used in FTL mode");
      _strings.erase("outputs.jffs2");
    }

    if(isParamPresent("outputs.mtd"))
    {
      WARNING("MTD output should not be used in FTL mode");
      _strings.erase("outputs.mtd");
    }
  }

  /* when choosing jffs2 output we want the FFS to be jffs2 */
  if(isParamPresent("outputs.jffs2"))
  {
    if(getString("functional_model.ffs.type").compare("JFFS2"))
    {
      WARNING("Choosed jffs2 output but FFS is not jffs2");
      _strings.erase("outputs.jffs2");
    }
  }
}

/**
 * Return true if the parameter has been parsed, false otherwise
 */
bool Param::isParamPresent (string param)
{
  if(	_integers.find(param) == _integers.end() &&
	_booleans.find(param) == _booleans.end() &&
	_strings.find(param) == _strings.end() &&
	_doubles.find(param) == _doubles.end())
	  return false;
  return true;
}

/**
 * TODO ajouer log stdout
 */
vector<ParamDesc> Param::getParamList()
{
  vector<ParamDesc> res;
#define ADD_PARAM(name, desc, type, optional)	res.push_back(ParamDesc(name, desc, type, optional))

  /* Add a line for each new param here */
  ADD_PARAM("ppc_models.random_seed", "Random seed used in random number generations in a simulation", TYPE_INT, true);
  ADD_PARAM("flash_layer.pages_per_block", "Number of flash pages per block", TYPE_INT, false);
  ADD_PARAM("flash_layer.blocks_per_plane", "Number of flash block per plane", TYPE_INT, false);
  ADD_PARAM("flash_layer.planes_per_lun", "Number of flash planes per LUN", TYPE_INT, false);
  ADD_PARAM("flash_layer.luns_per_channel", "Number of flash LUNs per channel", TYPE_INT, false);
  ADD_PARAM("flash_layer.channels", "Total number of channels", TYPE_INT, false);
  ADD_PARAM("flash_layer.page_size_bytes", "Size of one flash page in bytes (user data)", TYPE_INT, false);
  ADD_PARAM("flash_layer.oob_size_bytes", "Size of the OOB area in bytes", TYPE_INT, false);
  ADD_PARAM("flash_layer.channel_width_bits", "Witdh of the I/O bus(es) (channels) in bits", TYPE_INT, false);
  ADD_PARAM("functional_model.vfs.page_cache_size_bytes", "FFS mode : Size of the Linux page cache in bytes", TYPE_INT, true);
  ADD_PARAM("functional_model.ftl.pagemap.gc_threshold", "FTL mode, Pagemap FTL : threshold in blocks under which PM gc is launched", TYPE_INT, true);
  ADD_PARAM("functional_model.ftl.bplru.size_blocks", "FTL mode, BPLRU flash buffer : Size of BPLRU in number of flash blocks", TYPE_INT, true);
  ADD_PARAM("ppc_models.nand_driver.read_time_overhead", "FFS mode : Overhead in us added to a flash page read operation by the MTD nand driver", TYPE_DOUBLE, true);
  ADD_PARAM("ppc_models.nand_driver.write_time_overhead", "FFS mode : Overhead in us added to a flash page write operation by the MTD nand driver", TYPE_DOUBLE, true);
  ADD_PARAM("ppc_models.nand_driver.erase_time_overhead", "FFS mode : Overhead in us added to a flash block erase operation by the MTD nand driver", TYPE_DOUBLE, true);
  ADD_PARAM("ppc_models.nand_driver.read_pc_overhead", "Overhead in J (TODO) added to a flash page read operation by the MTD nand driver ", TYPE_DOUBLE, true);
  ADD_PARAM("ppc_models.nand_driver.write_pc_overhead", "Overhead in J (TODO) added to a flash page write operation by the MTD nand driver ", TYPE_DOUBLE, true);
  ADD_PARAM("ppc_models.nand_driver.erase_pc_overhead", "Overhead in J (TODO) added to a flash block erase operation by the MTD nand driver ", TYPE_DOUBLE, true);
  ADD_PARAM("functional_model.vfs.pdflush_frequency", "FFS mode : Frequency of execution for the pdflush deamon", TYPE_DOUBLE, true);
  ADD_PARAM("simulator.trace.mode", "(TODO) simulator workload input mode (trace / TODO)", TYPE_STRING, false);
  ADD_PARAM("simulator.trace.parser", "Trace parser (FFS : ffs_csv, FTL : ftl_ascii)", TYPE_STRING, true);
  ADD_PARAM("simulator.trace.path", "Path for the workload containing the trace file", TYPE_FILE, true);
  ADD_PARAM("functional_model.functional_mode", "Functional mode : ftl or ffs", TYPE_STRING, false);
  ADD_PARAM("functional_model.ffs.type", "FFS mode : FFS type (jffs2)", TYPE_STRING, true);
  ADD_PARAM("functional_model.ftl.type", "FTL mode : FTL type (pagemap)", TYPE_STRING, true);
  ADD_PARAM("functional_model.ftl.flash_buffer", "FTL mode : flash buffer type (none/bplru", TYPE_STRING, true);
  ADD_PARAM("functional_model.copyback_even_odd_constraint", "Apply even/odd constraint on copy back NAND operations", TYPE_BOOL, true);
  ADD_PARAM("functional_model.multiplane_same_block", "Apply the same block restrictions on NAND multiplane operation", TYPE_BOOL, true);
  ADD_PARAM("functional_model.vfs.read_ahead", "FFS mode : enable the Linux page cache read-ahead algorithm", TYPE_BOOL, true);
  ADD_PARAM("functional_model.vfs.read_ahead_sequential_cache_miss_bug", "FFS mode : enable the pre Linux 3.13 bad read-ahead behavior", TYPE_BOOL, true);
  ADD_PARAM("functional_model.ffs.jffs2.check_after_erase", "FFS mode, FFS JFFS2 : Enable the checks JFFS2 performs on a newly erased blocks (read the entire block)", TYPE_BOOL, true);
  ADD_PARAM("ppc_models.performance_model.TON", "Transfer-out-of-NAND timing model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("ppc_models.performance_model.TIN", "Transfer-in-NAND timing model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("ppc_models.performance_model.IO", "One cycle Input / Output on the flash IO bus timing model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("ppc_models.performance_model.BERS", "Block erase timing model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("ppc_models.performance_model.PTON", "Transfer-out-of-NAND power model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("ppc_models.performance_model.PTIN", "Transfer-in-NAND power model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("ppc_models.performance_model.PIO", "One cycle Input / Output on the flash IO bus power model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("ppc_models.performance_model.PBERS", "Block erase power model TODO", TYPE_DISTRIB, false);
  ADD_PARAM("outputs.wear_leveling", "Set the output for the wear leveling stat result file, STDOUT for std output", TYPE_FILE, true);
  ADD_PARAM("outputs.jffs2", "Set the output for the jffs2 result file, STDOUT for stdout", TYPE_FILE, true);
  ADD_PARAM("outputs.mtd", "Set the output for the mtd result file, STDOUT for stdout", TYPE_FILE, true);
  ADD_PARAM("simulator.random_seed", "Random seed used by the simulator", TYPE_DOUBLE, true);
  ADD_PARAM("functional_model.ffs.jffs2.base_readpage_timing_overhead", "FFS overhead for JFFS2 readpage function", TYPE_DOUBLE, true);
  ADD_PARAM("functional_model.ffs.jffs2.base_write_end_timing_overhead", "Timing overhead added to each jffs2_write_end call", TYPE_DOUBLE, true);
  ADD_PARAM("functional_model.ffs.jffs2.base_write_begin_timing_overhead", "Timing overhead added to each jffs2_write_begin call", TYPE_DOUBLE, true);

  return res;
}
