cmake_minimum_required (VERSION 3.5.1)
project (netwhere)

option(BUILD_TESTS
  "Build tests" OFF)

option(BUILD_DOC
  "Build documentation" ON)

if (BUILD_TESTS)
  enable_testing()

  add_executable(netwhere_test netwhere_test.cpp netwhere.cpp flow_collector.cpp)
  target_compile_features(netwhere_test PUBLIC cxx_auto_type cxx_generalized_initializers)
  target_link_libraries(netwhere_test PRIVATE tins microhttpd boost_unit_test_framework)
  add_test(netwhere_test ./netwhere_test)

  add_executable(object_set_test object_set_test.cpp)
  target_compile_features(object_set_test PUBLIC cxx_auto_type cxx_generalized_initializers)
  target_link_libraries(object_set_test PRIVATE boost_unit_test_framework)
  add_test(object_set_test ./object_set_test)

  add_executable(flow_test flow_test.cpp flow_collector.cpp)
  target_compile_features(flow_test PUBLIC cxx_auto_type cxx_generalized_initializers)
  target_link_libraries(flow_test PRIVATE tins boost_unit_test_framework)
  add_test(flow_test ./flow_test)
endif (BUILD_TESTS)

add_executable(netwhere main.cpp netwhere.cpp flow_collector.cpp)
target_link_libraries(netwhere PRIVATE tins microhttpd)
target_compile_features(netwhere PUBLIC cxx_auto_type cxx_generalized_initializers)
install(TARGETS netwhere DESTINATION bin)

if (ENABLE_LOGGING)
  target_compile_definitions(netwhere PRIVATE ENABLE_LOGGING)
endif (ENABLE_LOGGING)

find_package(Doxygen)
if (DOXYGEN_FOUND)
  set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile)
  set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/docs)

  message("Doxygen build started")

  add_custom_target( doc_doxygen ALL
    COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM )

else (DOXYGEN_FOUND)
  message("Doxygen need to be installed to generate documentation")
endif (DOXYGEN_FOUND)
