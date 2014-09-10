add_library(cos_sim_sparse_update SHARED update.cpp)
target_link_libraries(cos_sim_sparse_update ${CMAKE_DL_LIBS})
install(TARGETS cos_sim_sparse_update LIBRARY DESTINATION lib)

add_executable(cos_sim_sparse cos_sim_sparse.cpp)

target_link_libraries(cos_sim_sparse ${Boost_LIBRARIES} comm scheduler)
install(TARGETS cos_sim_sparse RUNTIME DESTINATION bin)
