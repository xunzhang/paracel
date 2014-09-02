add_executable(hi say_hi.cpp)

set(EXTRA_SO
    "/usr/lib/libboost_regex.so")

target_link_libraries(hi ${Boost_LIBRARIES}
  ${EXTRA_SO}
  comm
  scheduler)
install(TARGETS hi RUNTIME DESTINATION bin)
