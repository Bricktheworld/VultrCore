cmake_minimum_required(VERSION 3.19.4)

set(This VultrBenchmark)
file(
  GLOB_RECURSE Sources
  "*.cpp"
)

set(BENCHMARK_ENABLE_TESTING OFF)
set(HAVE_GNU_POSIX_REGEX FALSE)
set(HAVE_POSIX_REGEX FALSE)
set(HAVE_STD_REGEX TRUE)
add_subdirectory(vendor/benchmark)

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC Vultr benchmark::benchmark)
include_directories(vendor)

add_test(
  NAME ${This}
  COMMAND ${This}
)
