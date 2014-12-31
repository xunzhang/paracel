add_executable(lasso_serial lasso_serial.cc)
target_link_libraries(lasso_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lasso_serial RUNTIME DESTINATION bin)
