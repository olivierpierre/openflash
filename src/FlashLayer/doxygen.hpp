/*! \mainpage A Flash based system Simulator for Performance and Energy Computation - FSPEC Technical Reference
 *
 * \section intro_sec Introduction
 *
 * This is the FSPEC technical documentation. FSPEC stands for 'a Flash
 * Simulator for Performance and Energy Computation'.
 * 
 * FSPEC aims to simulate the whole storage hierarchy of NAND flash
 * based storage systems, from simple embedded single chip (Linux based)
 *  storage systems, up to complex, multi chips / multi channels 
 * solid-state drives systems . By hierarchy we mean the control layer 
 * and the flash layer:
 * 
 * * The control layer is the layer abstracting the flash complexity to
 * the applicative space. It is for example in a SSD the FTL 
 * (Flash Translation Layer) which implements address mapping schemes,
 * allocation schemes, wear leveling and garbage collection algorithms.
 * For a Linux-based embedded NAND storage system, the control layer 
 * can be seen as the reunion of the VFS, FFS, and NAND driver software
 * layers implemented in the operating system. In particular, the FFS
 * implements the same functions as the FTL for Flash based devices.
 * 
 * * The flash layer itself is the reunion of the flash hardware 
 * components managed by the control layer. For a Linux based embedded
 * NAND storage system it is usually a single chip soildered on the 
 * embedded system board. For a SSD it is a set of flash chips and I/O
 * buses named channels.
 * 
 * In the current state of developpement, FSPEC implements only the 
 * Flash layer.  
 *
 * \section starting_up_sec Starting up
 * 
 * \subsection userguide_subsec User guide
 * 
 * The first thing to know is that there is a user guide for the FSPEC
 * flash subsystem layer. Before using the flash subsystem API provided
 * by FSPEC, it is very important to understand how we view a flash 
 * based subsystem, and how we implement it in FSPEC. So The the user 
 * guide should be the first documentation reading.
 * 
 * The latest version of the flash subsystem layer user guide is 
 * provided with FSPEC sources in the doc/ directory. This guide is also
 * present at the following address:
 * 
 * http://stockage.univ-brest.fr/~polivier/FSPEC/UserGuide.pdf
 * 
 * \subsection using_the_api_subsec Using the API
 * 
 * Instantiating a flash system object and sending commands to it is
 * done with the use of the FlashSystem class.
 * 
 * Addressing something (a page, block, planee, etc.) in the flash
 * system is done trough the use of the Address class.
 * 
 * Sending flash commands to the simulated flash system during the
 * simulation can be done in two ways:
 * * Using the member functions of the simulated FlashSystem, such as
 * FlashSystem::readPage(), FlashSystem::eraseBlock(), 
 * FlashSystem::dieInterleavedRead(), etc. ;
 * * Using the FlashSystem::receiveCommand() function. This function 
 * takes a FlashCommand object in parameter, which is a generic type
 * used to describe all the flash commands supported by FSPEC. It is a
 * virtual type, not meant to be instantiated itself. Pl
 */
