add_library(wc_update SHARED update.cpp)
target_link_libraries(wc_update ${CMAKE_DL_LIBS})
install(TARGETS wc_update LIBRARY DESTINATION lib)

add_executable(paracel_wc wc_driver.cpp)
set(EXTRA_SO 
  "/usr/lib/libboost_regex.so")

target_link_libraries(paracel_wc ${Boost_LIBRARIES} 
  ${EXTRA_SO}
  comm
  scheduler)
install(TARGETS paracel_wc RUNTIME DESTINATION bin)
