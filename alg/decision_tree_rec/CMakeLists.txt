add_executable(dtr driver.cpp)
target_link_libraries(dtr ${Boost_LIBRARIES} comm scheduler)
install(TARGETS dtr RUNTIME DESTINATION bin)
