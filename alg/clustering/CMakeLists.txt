add_library(clustering_update SHARED update.cpp)
target_link_libraries(clustering_update ${CMAKE_DL_LIBS})
install(TARGETS clustering_update LIBRARY DESTINATION lib)

add_library(clustering_filter SHARED filter.cpp)
target_link_libraries(clustering_update ${CMAKE_DL_LIBS})
install(TARGETS clustering_filter LIBRARY DESTINATION lib)

add_executable(kmeans kmeans_driver.cpp)
set(EXTRA_SO
    "/usr/lib/libboost_regex.so")
add_executable(spectral_clustering sc_driver.cpp)
set(EXTRA_SO,
    "/usr/lib/libboost_regex.so")


target_link_libraries(kmeans ${Boost_LIBRARIES}
  ${EXTRA_SO}
  comm
  scheduler)
target_link_libraries(spectral_clustering ${Boost_LIBRARIES}
  ${EXTRA_SO}
  comm
  scheduler)

install(TARGETS kmeans RUNTIME DESTINATION bin)
install(TARGETS spectral_clustering RUNTIME DESTINATION bin)
