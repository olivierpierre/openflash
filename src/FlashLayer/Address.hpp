/**
 * \file Address.hpp
 * \brief Address class header
 * \author Pierre Olivier <pierre.olivier@univ-brest.fr>
 * \version 0.1
 * \date 2013/04/08
 *
 * Address class header.
 *
 */

#ifndef ADDRESS_HPP
#define ADDRESS_HPP

#include <string>

#include "../Globs/Common.hpp"

using namespace std;

class Address
{
public:
  Address(uint32_t channel,
	  uint32_t lun,
	  uint32_t plane,
	  uint32_t block,
	  uint32_t page);
  Address(const Address &a);
  virtual ~Address(void);

  /* Getters / setters : */
  int getPage();
  int getBlock();
  int getPlane();
  int getLun();
  int getChannel();
  void setPage(int page);
  void setBlock(int block);
  void setPlane(int plane);
  void setLun(int lun);
  void setChannel(int channel);

  bool sameBlock(Address a);
  bool samePlane(Address a);
  bool sameLun(Address a);
  bool sameChannel(Address a);

  string str();
    
  friend bool operator== (Address &a1, Address &a2);  /**< Equality */
  friend bool operator!= (Address &a1, Address &a2);  /**< Inequality */
  int operator[] (const int nIndex);

private:
  uint16_t _page;					/**< \brief Page index in the block */
  uint16_t _block;				/**< \brief Block index in the plane */
  uint16_t _plane : 8;		/**< \brief Plane index in the die */
  uint16_t _lun : 8;      /**< \brief LUN index in the channel */
  uint16_t _channel : 8;	/**< \brief Channel index in the flash layer */

  friend ostream& operator<<(ostream& os, const Address& a);  /**< print on stream */
};

#endif /* ADDRESS_HPP */
