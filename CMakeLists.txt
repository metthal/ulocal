cmake_minimum_required(VERSION 3.8)
project(ulocal CXX)

option(ULOCAL_EXAMPLES "Build examples" OFF)
option(ULOCAL_TESTS "Build tests" OFF)

find_package(Threads REQUIRED)

include(GNUInstallDirs)

set(CMAKE_CXX_STANDARD 17)
set(INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")

file(GLOB_RECURSE HEADERS "${INCLUDE_DIRECTORY}/*.hpp")

add_library(ulocal INTERFACE)
target_include_directories(ulocal INTERFACE
	"$<BUILD_INTERFACE:${INCLUDE_DIRECTORY}>"
	"$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}>"
)
# Only for CMake 3.16+
#set_target_properties(ulocal PROPERTIES PUBLIC_HEADER "${HEADERS}")
target_link_libraries(ulocal INTERFACE Threads::Threads)

# Only for CMake 3.16+
#install(
#	TARGETS ulocal
#	PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}"
#)
install(
	TARGETS ulocal
	EXPORT ulocal
	DESTINATION ${CMAKE_INSTALL_PREFIX}
)
install(
	DIRECTORY "${INCLUDE_DIRECTORY}/"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(
	EXPORT ulocal
	NAMESPACE ulocal::
	DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/cmake
)

if(ULOCAL_EXAMPLES)
	add_subdirectory(examples)
endif()
if(ULOCAL_TESTS)
	add_subdirectory(tests)
endif()
