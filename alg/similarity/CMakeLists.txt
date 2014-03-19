add_library(sim_update SHARED update.cpp)
target_link_libraries(sim_update ${CMAKE_DL_LIBS})
install(TARGETS sim_update LIBRARY DESTINATION lib)

add_executable(paracel_sim sim_driver.cpp)

target_link_libraries(paracel_sim ${Boost_LIBRARIES} 
  comm
  scheduler)
install(TARGETS paracel_sim RUNTIME DESTINATION bin)
