#include <iostream>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <unistd.h>

#include "ErrorManager.hpp"
#include "../Globs/Common.hpp"
#include "Simulation.hpp"


using namespace std;

ErrorManager *ErrorManager::_singleton = NULL;

/**
 * \fn ErrorManager::ErrorManager(bool stopOnWarning, bool stopOnError)
 * \brief Error Manager constructor
 * \param stopOnWarning Should we stop the simulation when a warning
 * occurs ?
 */
ErrorManager::ErrorManager()
{
  _suppressNextError = _suppressNextWarning = false;
}

/**
 * \fn ErrorManager::~ErrorManager()
 * \brief Destructor
 */
ErrorManager::~ErrorManager(){}

/**
 * \fn void ErrorManager::warning(string errMsg)
 * \brief Warning message, do not stop the simulation
 */
void ErrorManager::warning(string errMsg)
{
  if(_suppressNextWarning)
  {
    _suppressNextWarning = false;
    return;
  } 

  cerr << "\033[1;31m!! Warning : " << errMsg << "\033[0m" << endl;
} 

/**
 * \fn void ErrorManager::error(string errMsg, string file, int line)
 * \brief (Serious) error message, exit
 */
void ErrorManager::error(string errMsg, string file, int line)
{
  if(_suppressNextError)
  {
    _suppressNextError = false;
    return;
  }
    
  if(file == "" && line == -1)
    cerr << "\033[1;31m!! Error : " << errMsg << "\033[0m" << endl;
  else
    cerr << "\033[1;31m!! Error [" << file << ":" << nts(line) << "] : " << errMsg << "\033[0m" << endl;

#ifndef NDEBUG
  assert(0);
#else
  Simulation::getInstance()->kill();
  exit(EXIT_FAILURE);
#endif /* NDEBUG */
}

/**
 * \fn ErrorManager * ErrorManager::getInstance (bool stopOnWarning = false)
 * \brief Return a pointer on the single(ton) error manager
 */
ErrorManager * ErrorManager::getInstance ()
{
  if (_singleton == NULL)
    _singleton =  new ErrorManager();

  return _singleton;
}

/**
 * \fn void ErrorManager::kill ()
 * \brief free the error manger object
 */
void ErrorManager::kill ()
{
  if (_singleton != NULL)
    {
      delete _singleton;
      _singleton = NULL;
    }
}
