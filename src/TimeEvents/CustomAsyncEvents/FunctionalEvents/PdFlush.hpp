#ifndef PDFLUSH_HPP
#define PDFLUSH_HPP

#include "../../Event.hpp"

class PdFlush : public AsyncEvent
{
public:
  PdFlush(double frequency);
  PdFlush *clone() const { return new PdFlush(*this); };

private:
  int processEvent();
};



#endif /* PDFLUSH_HPP */
