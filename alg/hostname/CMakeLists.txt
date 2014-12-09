add_library(hostname_update SHARED update.cpp)
target_link_libraries(hostname_update ${CMAKE_DL_LIBS})
install(TARGETS hostname_update LIBRARY DESTINATION lib)

add_executable(hostname hostname.cpp)

target_link_libraries(hostname ${Boost_LIBRARIES} comm scheduler)
install(TARGETS hostname RUNTIME DESTINATION bin)
