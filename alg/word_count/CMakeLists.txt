add_library(wc_update SHARED update.cpp)
target_link_libraries(wc_update ${CMAKE_DL_LIBS})
install(TARGETS wc_update LIBRARY DESTINATION lib)

add_executable(wc wc_driver.cpp)
set(EXTRA_SO 
  "/usr/lib/libboost_regex.so")

target_link_libraries(wc ${Boost_LIBRARIES} 
  ${EXTRA_SO}
  comm
  scheduler)
install(TARGETS wc RUNTIME DESTINATION bin)
