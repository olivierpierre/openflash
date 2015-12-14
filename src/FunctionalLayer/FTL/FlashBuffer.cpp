#include "FlashBuffer.hpp"

#include "FlashBuffers/BPLRU.hpp"

FlashBuffer *FlashBuffer::_singleton = NULL;

FlashBuffer::~FlashBuffer() {}

FlashBuffer* FlashBuffer::getInstance()
{
  if(_singleton == NULL)
  {
    if(!Param::getInstance()->getString("functional_model.ftl.flash_buffer").compare("bplru"))
      _singleton = new Bplru(FlashTranslationLayer::getInstance());
    else
      ERROR("unknown flash buffer type ...");
  }

  return _singleton;
}

void FlashBuffer::kill()
{
	if(_singleton != NULL)
	{
		delete _singleton;
		_singleton = NULL;
	}
}
