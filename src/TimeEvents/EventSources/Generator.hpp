/*
 * Generator.hpp
 *
 *  Created on: 9 avr. 2014
 *      Author: pierre
 */

#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include "../Event.hpp"

class Generator
{
public:
  static Generator *getInstance();
  static void kill();
  virtual ~Generator ();

  virtual Event *getNextEvent()=0;

protected:

  Generator ();

private:

  static Generator *_singleton;
};

class FakeGen : public Generator
{
public:
  FakeGen() : Generator() {};
  Event *getNextEvent();

private:
};



#endif /* GENERATOR_HPP */
