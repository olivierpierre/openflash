# as we add some custom modules to find gsl and libconfig ...
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../cmake/Modules/")

# Libconfig
find_package(LibConfig++ REQUIRED)
include_directories(${LIBCONFIG++_INCLUDE_DIR})
set(LIBS ${LIBS} ${LIBCONFIG++_LIBRARY})

# GSL
find_package(GSL REQUIRED)
include_directories(${GSL_INCLUDE_DIR})
set(LIBS ${LIBS} ${GSL_LIBRARIES})
