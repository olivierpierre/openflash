/**
 * \file Address.cpp
 * \brief Address class implementation
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/02
 *
 * Address class implementation.
 *
 */


#include "Address.hpp"

#include <iostream>
#include <sstream>

#include "../Globs/Common.hpp"


using namespace std;

/**
 * \fn Address::Address(	uint32_t channel, 
							uint32_t chip,
							uint32_t die,
							uint32_t plane,
							uint32_t block,
							uint32_t page);
 * \brief Constructor. 
 */
Address::Address(	uint32_t channel, 
							uint32_t lun,
							uint32_t plane,
							uint32_t block,
							uint32_t page)
{
	_channel = channel;
	_lun = lun;
	_plane = plane;
	_block = block;
	_page = page;
}

/**
 * \fn Address::Address(const Address &a)
 * \brief Copy contructor
 */
Address::Address(const Address &a)
{
  _channel = a._channel;
  _lun = a._lun;
  _plane = a._plane;
  _block = a._block;
  _page = a._page;
}

/**
 * \fn Address::~Address()
 * \brief Destructor
 */
Address::~Address(){}

/**
 * \fn int Address::getPage()
 * \brief Return the page index in its block for the address object
 */
int Address::getPage()
{
	return _page;
}

/**
 * \fn int Address::getBlock()
 * \brief Return the block index in its plane for the address object
 */
int Address::getBlock()
{
	return _block;
}

/**
 * \fn int Address::getPlane()
 * \brief Return the plane index in its die for the address object
 */
int Address::getPlane()
{
	return _plane;
}

/**
 * \fn int Address::getLun()
 * \brief Return the lun index in its chip for the address object
 */
int Address::getLun()
{
  return _lun;
}

/**
 * \fn int Address::getChannel()
 * \brief Return the channel index in the flash layer for the address
 * object
 */
int Address::getChannel()
{
	return _channel;
}

/**
 * \fn ostream& operator<<(ostream& os, const Address& a)
 * \brief Print info about the address on a stream
 * \param os stream
 * \param a address
 **/
ostream& operator<<(ostream& os, const Address& a)
{
    os << "(" << a._channel << "," << a._lun << "," <<
      a._plane << "," << a._block << "," << a._page << ")";
    return os;
}

/**
 * \fn string Address::str()
 * \brief Return a string describing the address object
 */
string Address::str()
{
  stringstream ss;
  ss << (*this);
  return ss.str();
}

/**
 * \fn bool operator== (Address &a1, Address &a2)
 * \brief Comparison between 2 addresses
 */
bool operator== (Address &a1, Address &a2)
{
    return (a1._page == a2._page &&
            a1._block == a2._block &&
            a1._plane == a2._plane &&
            a1._lun == a2._lun &&
            a1._channel == a2._channel);
}

/**
 * \fn bool operator!= (Address &a1, Address &a2)
 * \brief Comparison between 2 addresses
 */
bool operator!= (Address &a1, Address &a2)
{
    return !(a1 == a2);
}

/**
 * \fn void Address::setPage(int page)
 * \brief set the page member of the address to the corresponding value
 */
void Address::setPage(int page)
{
  _page = page;
}

/**
 * \fn void Address::setBlock(int block)
 * \brief set the block member of the address to the corresponding value
 */
void Address::setBlock(int block)
{
  _block = block;
}

/**
 * \fn void Address::setPlane(int plane)
 * \brief set the plane member of the address to the corresponding value
 */
void Address::setPlane(int plane)
{
  _plane = plane;
}

/**
 * \fn void Address::setLun(int lun)
 * \brief set the LUN member of the address to the corresponding value
 */
void Address::setLun(int lun)
{
  _lun = lun;
}

/**
 * \fn void Address::setChannel(int channel)
 * \brief set the channel member of the address to the corresponding value
 */
void Address::setChannel(int channel)
{
  _channel = channel;
}

bool Address::sameBlock(Address a)
{
  return samePlane(a) && (a.getBlock() == _block);
}

bool Address::samePlane(Address a)
{
  return sameLun(a) && (a.getPlane() == _plane);
}

bool Address::sameLun(Address a)
{
  return sameChannel(a) && (a.getLun() == _lun);
}

bool Address::sameChannel(Address a)
{
  return a.getChannel() == _channel;
}
