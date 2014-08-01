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
  set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O2")
  set(CMAKE_CXX_FLAGS_DEBUG "-Wall -std=c++11 -O0 -g -fno-inline")
  set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -O3 -march=native -ffast-math")
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

set(BOOST_ROOT "/usr")
#set(BOOST_ROOT "/mfs/user/wuhong/.local")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.55.0 COMPONENTS filesystem system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmakes")
find_package(Eigen REQUIRED)
include_directories(${Eigen_INCLUDE_DIR})
find_package(GFlags REQUIRED)
include_directories(${GFlags_INCLUDE_DIR})
find_package(MsgpackC REQUIRED)
include_directories(${MsgpackC_INCLUDE_DIR})
find_package(ZeroMQ REQUIRED)
include_directories(${ZermMQ_INCLUDE_DIR})

set(COROUTINE_VAR "${BOOST_ROOT}/lib/libboost_coroutine.so")
link_libraries(${COROUTINE_VAR} ${GFlags_LIBRARIES} ${MsgpackC_LIBRARIES} ${ZeroMQ_LIBRARIES} ${MPI_C_LIBRARIES} ${MPI_CXX_LIBRARIES})
##############################--make-output--###################################
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/local")
############################--install-path--####################################
install(PROGRAMS prun.py DESTINATION "${PROJECT_SOURCE_DIR}/local/bin")
################################################################################
add_subdirectory(src)
add_subdirectory(alg/word_count)
add_subdirectory(alg/logistic_regression)
add_subdirectory(alg/matrix_factorization)
add_subdirectory(alg/similarity)
add_subdirectory(alg/hi)
add_subdirectory(alg/cbr)
add_subdirectory(alg/pagerank)
add_subdirectory(alg/clustering)
add_subdirectory(alg/hostname)
add_subdirectory(alg/max_vertex_value)
################################################################################
