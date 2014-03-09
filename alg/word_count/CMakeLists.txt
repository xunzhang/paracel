add_library(wc_update SHARED update.cpp)
target_link_libraries(wc_update ${CMAKE_DL_LIBS})
install(TARGETS wc_update LIBRARY DESTINATION lib)

add_executable(wc wc_driver.cpp)
set(EXTRA_SO 
  "/mfs/user/wuhong/.local/lib/libboost_coroutine.so"
  "/usr/lib/libicuuc.so"
  "/usr/lib/libicui18n.so"
  "/usr/lib/libicudata.so"
  "/usr/lib/libboost_regex.so"
  "/usr/lib/libboost_regex.so")
set(PARACEL_LIB ${LIBRARY_OUTPUT_PATH})
target_link_libraries(wc ${Boost_LIBRARIES} ${EXTRA_SO} ${PARACEL_LIB})
install(TARGETS wc RUNTIME DESTINATION bin)
