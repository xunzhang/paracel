add_library(kmeans_update SHARED update.cpp)
target_link_libraries(kmeans_update ${CMAKE_DL_LIBS})
install(TARGETS kmeans_update LIBRARY DESTINATION lib)

add_executable(kmeans driver.cpp)
set(EXTRA_SO
    "/usr/lib/libboost_regex.so")

target_link_libraries(kmeans ${Boost_LIBRARIES}
  ${EXTRA_SO}
  comm
  scheduler)
install(TARGETS kmeans RUNTIME DESTINATION bin)
