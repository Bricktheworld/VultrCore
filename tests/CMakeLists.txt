cmake_minimum_required(VERSION 3.19.4)

set(This VultrTests)
file(
  GLOB_RECURSE Sources
  "*.cpp"
)

add_executable(${This} ${Sources})
target_link_libraries(${This} PUBLIC Vultr GTest::gtest)

add_test(
  NAME ${This}
  COMMAND ${This}
)
