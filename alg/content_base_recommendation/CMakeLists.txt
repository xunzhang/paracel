add_library(cbr_update SHARED update.cpp)
target_link_libraries(cbr_update ${CMAKE_DL_LIBS})
install(TARGETS cbr_update LIBRARY DESTINATION lib)

add_executable(cbr cbr_driver.cpp)

target_link_libraries(cbr
  ${Boost_LIBRARIES}
  comm scheduler)

install(TARGETS cbr RUNTIME DESTINATION bin)
