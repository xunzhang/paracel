add_executable(lasso_serial lasso_serial.cc)
target_link_libraries(lasso_serial ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lasso_serial RUNTIME DESTINATION bin)

add_executable(lasso_serial_old lasso_serial_old.cc)
target_link_libraries(lasso_serial_old ${Boost_LIBRARIES} comm scheduler)
install(TARGETS lasso_serial_old RUNTIME DESTINATION bin)
