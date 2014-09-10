add_library(comm STATIC comm.cpp)
install(TARGETS comm ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

add_library(scheduler STATIC scheduler.cpp)
install(TARGETS scheduler ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

add_library(default STATIC default.cpp)
install(TARGETS default ARCHIVE DESTINATION lib LIBRARY DESTINATION lib)

set(EXTRA_SO 
    "/usr/lib/libboost_regex.so")


add_executable(start_server start_server.cpp)
target_link_libraries(start_server ${Boost_LIBRARIES} ${EXTRA_SO} ${CMAKE_DL_LIBS})
install(TARGETS start_server RUNTIME DESTINATION bin)

add_executable(paracelrun_cpp_proxy paracelrun_cpp_proxy.cpp)
target_link_libraries(paracelrun_cpp_proxy ${Boost_LIBRARIES} ${EXTRA_SO} ${CMAKE_DL_LIBS})
install(TARGETS paracelrun_cpp_proxy RUNTIME DESTINATION bin)
