add_executable(tinyURL service.cpp)
target_link_libraries(tinyURL ${Boost_LIBRARIES} comm scheduler)
install(TARGETS tinyURL RUNTIME DESTINATION bin)
