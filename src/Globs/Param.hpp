/*
 * Param.hpp
 *
 *  Created on: 3 sept. 2013
 *      Author: pierre
 */

#ifndef PARAM_HPP
#define PARAM_HPP

#include <map>
#include <vector>
#include <string>

#include <libconfig.h++>

using namespace libconfig;
using namespace std;

class ParamDesc;

class Param
{
public:

  static vector<ParamDesc> getParamList();

  static Param * getInstance(string paramFilePath="");
  static void kill();

  int getInt(string param);
  bool getBool(string param);
  double getDouble(string param);
  string getString(string param);

  bool isParamPresent(string param);

  void dumpParams();

private:
  Param(string paramFilePath);
  ~Param();

  int _doBuildParams();
  int doParse();
  int parsePpcModels();

  int parseFlashLayerParameters();
  int parseFfsParameters();
  int parseFtlParameters();
  int parseJffs2Parameters();
  int parsePagemapParameters();
  int parseBplruParameters();
  int parseOutputParameters();
  int parseArchiveParameters();

  void setInt(string param, int val);
  void setBool(string param, bool val);
  void setDouble(string param, double val);
  void setString(string param, string val);

  int parseInt(string param, bool optional=false, bool *parsed=NULL);
  bool parseBool(string param, bool optional=false, bool *parsed=NULL);
  double parseDouble(string param, bool optional=false, bool *parsed=NULL);
  string parseString(string param, bool optional=false, bool *parsed=NULL);

  void sanityCheck();

  map<string, int> _integers;
  map<string, bool> _booleans;
  map<string, double> _doubles;
  map<string, string> _strings;

  Config *_cfg;
  string _paramFilePath;

  static Param *_singleton;
};

extern Param *param;

/**
 * Type of a parameter
 */
typedef enum
{
  TYPE_STRING,
  TYPE_BOOL,
  TYPE_INT,
  TYPE_DOUBLE,
  TYPE_DISTRIB,
  TYPE_FILE
} param_desc_type_t;

/**
 * Description and type for one parameter
 */
class ParamDesc
{
public:
  ParamDesc(string name, string description, param_desc_type_t type, bool optional) :
    name(name), description(description), type(type), optional(optional) {};
  string name;
  string description;
  param_desc_type_t type;
  bool optional;
};

#endif /* PARAM_HPP */
