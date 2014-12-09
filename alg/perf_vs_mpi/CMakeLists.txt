add_library(perf_update SHARED update.cpp)
target_link_libraries(perf_update ${CMAKE_DL_LIBS})
install(TARGETS perf_update LIBRARY DESTINATION lib)

add_executable(paracel_perf main.cpp)
target_link_libraries(paracel_perf ${Boost_LIBRARIES}
  comm
  scheduler)
install(TARGETS paracel_perf RUNTIME DESTINATION bin)
