add_executable(dtr driver.cpp)
target_link_libraries(dtr ${Boost_LIBRARIES} comm scheduler)
install(TARGETS dtr RUNTIME DESTINATION bin)

add_executable(dtr_serial driver_serial.cpp)
target_link_libraries(dtr_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS dtr_serial RUNTIME DESTINATION bin)
