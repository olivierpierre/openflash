cmake_minimum_required(VERSION 2.6)

# project name
project(OpenFlash)

include("${CMAKE_SOURCE_DIR}/../cmake/find_libs.cmake")

# sources files for the simulator
# TODO when the number of source files is relatively stable, put all direclty here
# because glob_recurse is not recommended
file(GLOB_RECURSE simulator_sources FlashLayer/*.cpp FunctionalLayer/*.cpp Globs/*.cpp TimeEvents/*.cpp Outputs/*.cpp)

# main program
add_executable(openflash Main.cpp ${simulator_sources})
target_link_libraries(openflash ${LIBS})

# jffs2 write buffer test
#add_executable(Jffs2WriteBufferTest Tests/Jffs2WriteBufferTest.cpp ${simulator_sources})
#target_link_libraries(Jffs2WriteBufferTest ${LIBS})

# jffs2 frag comparison test
#add_executable(Jffs2FragCompTest Tests/Jffs2FragCompTest.cpp ${simulator_sources})
#target_link_libraries(Jffs2FragCompTest ${LIBS})

# jffs2 frag tests
#add_executable(Jffs2FragTests Tests/Jffs2FragTests.cpp ${simulator_sources})
#target_link_libraries(Jffs2FragTests ${LIBS})

# jffs2 tests
#add_executable(Jffs2Tests Tests/Jffs2Tests.cpp ${simulator_sources})
#target_link_libraries(Jffs2Tests ${LIBS})

# vfs tests
#add_executable(VfsTests Tests/VfsTests.cpp ${simulator_sources})
#target_link_libraries(VfsTests ${LIBS})

# event processor tests
#add_executable(EventProcessorTests Tests/EventProcessorTests.cpp ${simulator_sources})
#target_link_libraries(EventProcessorTests ${LIBS})

# trace parser tests
#add_executable(TraceParserTests Tests/TraceParserTests.cpp ${simulator_sources})
#target_link_libraries(TraceParserTests ${LIBS})

# Jffs2Block tests
#add_executable(Jffs2BlockTests Tests/Jffs2BlockTests.cpp ${simulator_sources})
#target_link_libraries(Jffs2BlockTests ${LIBS})

# pdflush tests
#add_executable(PdFlushTests Tests/PdFlushTests.cpp ${simulator_sources})
#target_link_libraries(PdFlushTests ${LIBS})

#set(CMAKE_CXX_FLAGS "-Wall -Werror -pedantic -g -O0")
#set(CMAKE_CXX_FLAGS "-Wall -Werror -O3")


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror -pedantic -g -O0")

set(CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror -pedantic -g -pg -O0")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -O3")

install (TARGETS openflash DESTINATION bin)

# uninstall target
configure_file(
    "../cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)

