##############################--CMakeLists.txt--################################
cmake_minimum_required(VERSION 2.8.9)
##############################--g++4.7-plus--###################################
if(EXISTS "gentoo_rules.cmake")
  find_program(CXX NAMES g++-4.8 g++-4.7.2 g++-4.7.1 g++-4.7 g++)
  execute_process(COMMAND sed -i 
	"s|CMAKE_CXX_COMPILER [^ ]*|CMAKE_CXX_COMPILER ${CXX}|g" 
	"gentoo_rules.cmake")
  find_program(CC NAMES gcc-4.8 gcc-4.7.2 gcc-4.7.1 gcc-4.7 gcc)
  execute_process(COMMAND sed -i 
	"s|CMAKE_C_COMPILER [^ ]*|CMAKE_C_COMPILER ${CC}|g"
	"gentoo_rules.cmake")
else()
  find_program(CMAKE_CXX_COMPILER NAMES g++-4.8 g++-4.7.2 g++-4.7.1 g++-4.7 g++)
  find_program(CMAKE_C_COMPILER NAMES gcc-4.8 gcc-4.7.2 gcc-4.7.1 gcc-4.7 gcc)
endif()
################################################################################
project(paracel CXX C)
##############################--flags--#########################################
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "-Wall -std=c++11")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -std=c++11 -O0 -g -fno-inline")
  set(CMAKE_CXX_FLAGS_RELEASE "-Wall -std=c++11 -O3 -march-native -ffast-math")
endif()

if(CMAKE_BUILD_TYPE MATCHES RELEASE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")
endif()

include_directories("${PROJECT_SOURCE_DIR}/include")
##############################--extern-libraries--##############################
find_package(MPI REQUIRED)
include_directories(${MPI_CXX_INCLUDE_PATH})
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} ${MPI_CXX_COMPILER_FLAGS})
set(CMAKE_LINK_FLAGS ${CMAKE_LINK_FLAGS} ${MPI_CXX_LINK_FLAGS})

find_package(Boost 1.52.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmakes")
find_package(Eigen REQUIRED)
include_directories(${Eigen_INCLUDE_DIR})
find_package(MsgpackC REQUIRED)
include_directories(${MsgpackC_INCLUDE_DIR})
find_package(ZeroMQ REQUIRED)
include_directories(${ZermMQ_INCLUDE_DIR})

link_libraries(${ZeroMQ_LIBRARIES} ${MsgpackC_LIBRARIES} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
##############################--make-output--###################################
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PEOJECT_BINARY_DIR}/lib")
set(CMAKE_INSTALL_PREFIX "${PEOJECT_SOURCE_DIR}/install")
############################--install-path--####################################
install(PROGRAMS prun.sh DESTINATION ${PEOJECT_SOURCE_DIR}/local)
################################################################################
add_subdirectory(src)
################################################################################
