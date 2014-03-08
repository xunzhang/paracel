add_library(comm SHARED comm.cpp)
install(TARGETS comm LIBRARY DESTINATION lib)

add_library(scheduler SHARED scheduler.cpp)
install(TARGETS scheduler LIBRARY DESTINATION lib)

add_library(default SHARED default.cpp)
install(TARGETS default LIBRARY DESTINATION lib)

add_executable(start_server start_server.cpp)
install(TARGETS start_server RUNTIME DESTINATION bin)

add_executable(paracelrun_cpp_proxy paracelrun_cpp_proxy.cpp)
install(TARGETS paracelrun_cpp_proxy RUNTIME DESTINATION bin)
