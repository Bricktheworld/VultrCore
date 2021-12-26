cmake_minimum_required(VERSION 3.19.4)

set(This VultrTests)
file(
		GLOB_RECURSE Sources
		"*.cpp"
)

enable_testing()

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC Vultr gtest)

add_test(
		NAME ${This}
		COMMAND ${This}
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

