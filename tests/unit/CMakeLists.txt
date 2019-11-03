set(SOURCES
	ulocal_tests.cpp
	test_http_request_parser.cpp
	test_string_stream.cpp
	test_utils.cpp
)

add_executable(ulocal_unit_tests gmock-gtest-all.cc ${SOURCES})
target_include_directories(ulocal_unit_tests PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(ulocal_unit_tests ulocal Threads::Threads)
