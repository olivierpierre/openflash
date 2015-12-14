/**
 * \file ErrorManager.hpp
 * \brief ErrorManager class header
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/12
 *
 * ErrorManager class header.
 *
 */

#ifndef ERROR_MANAGER_HPP
#define ERROR_MANAGER_HPP

#include <iostream>
#include <string>
#include <sstream>  

using namespace std;

/**
 * \class ErrorManager
 * \brief manages error and warnings
 */
class ErrorManager
{
public:

  static ErrorManager* getInstance();
  static void kill();


  void error(string errMsg, string file="", int line=-1);
  void warning(string errMsg);

private:

  ErrorManager();
  virtual ~ErrorManager();
  static ErrorManager *_singleton;

  bool _suppressNextWarning;  /**< Do not take into account the next warning (debug only) */
  bool _suppressNextError;    /**< Do not take into account the next error (debug only) */
};

extern ErrorManager *em;

#endif /* ERROR_MANAGER_HPP */
