add_library(pr_update SHARED update.cpp)
target_link_libraries(pr_update ${CMAKE_DL_LIBS})
install(TARGETS pr_update LIBRARY DESTINATION lib)

add_executable(pr main.cpp)
set(EXTRA_SO 
  "/usr/lib/libboost_regex.so")

target_link_libraries(pr ${Boost_LIBRARIES} 
  ${EXTRA_SO}
  comm
  scheduler)
install(TARGETS pr RUNTIME DESTINATION bin)
