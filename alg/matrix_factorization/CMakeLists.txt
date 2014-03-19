add_library(mf_update SHARED update.cpp)
target_link_libraries(mf_update ${CMAKE_DL_LIBS})
install(TARGETS mf_update LIBRARY DESTINATION lib)

add_executable(paracel_mf mf_driver.cpp)

target_link_libraries(paracel_mf 
  ${Boost_LIBRARIES} 
  comm scheduler)

install(TARGETS paracel_mf RUNTIME DESTINATION bin)
