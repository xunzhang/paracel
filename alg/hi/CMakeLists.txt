add_executable(hi say_hi.cpp)

target_link_libraries(hi ${Boost_LIBRARIES}
  comm
  scheduler)
install(TARGETS hi RUNTIME DESTINATION bin)
