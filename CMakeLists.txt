cmake_minimum_required (VERSION 3.5.1)
project (netwhere)

add_executable(netwhere main.cpp netwhere.cpp)

target_link_libraries(netwhere PRIVATE tins microhttpd)

target_compile_features(netwhere PUBLIC cxx_auto_type cxx_generalized_initializers)


add_executable(netwhere_test netwhere_test.cpp netwhere.cpp)

target_compile_features(netwhere_test PUBLIC cxx_auto_type cxx_generalized_initializers)

target_link_libraries(netwhere_test PRIVATE tins microhttpd boost_unit_test_framework)


add_executable(auto_dynamic_map_test auto_dynamic_map_test.cpp)

target_compile_features(auto_dynamic_map_test PUBLIC cxx_auto_type cxx_generalized_initializers)

target_link_libraries(auto_dynamic_map_test PRIVATE boost_unit_test_framework)


add_executable(flow_test flow_test.cpp)

target_compile_features(flow_test PUBLIC cxx_auto_type cxx_generalized_initializers)

target_link_libraries(flow_test PRIVATE tins boost_unit_test_framework)

