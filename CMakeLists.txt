cmake_minimum_required(VERSION 3.19.4)

set(This Vultr)
set(Executable VultrExe)

project(${This} VERSION 0.1.0)


enable_testing()

file(
  GLOB_RECURSE Sources
  "vendor/*.cpp"
  "src/sources.cpp"
)

add_subdirectory(vendor/googletest)

if(WIN32)
  add_compile_options("/std:c++latest")
  add_compile_options("/Zc:preprocessor")
endif(WIN32)

set(CMAKE_CXX_STANDARD 20)
add_compile_options("-Wno-reorder" "-Wno-volatile" "-Wno-deprecated-enum-enum-conversion")

include_directories(src vendor)

add_library(${This} STATIC ${Sources})

set(TargetIncludeDirs src vendor)

if(UNIX)
  set(LINUX_PRECOMPILED_BINARIES ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/LINUX)
  set(Libs vulkan GL X11 Xxf86vm Xrandr pthread Xi dl freetype ${LINUX_PRECOMPILED_BINARIES}/libassimp.so ${LINUX_PRECOMPILED_BINARIES}/libglfw3.a)
  link_directories(${CMAKE_BINARY_DIR}/res)
  file(GLOB_RECURSE VultrExeSources "src/platform/entry_point/linux_main.cpp")

  add_executable(${Executable} ${VultrExeSources})
endif(UNIX)

if(WIN32)
  find_library(GLFW3 glfw3 HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(FREETYPE freetyped HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(ASSIMP assimp HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)

  set(Libs ${GLFW3} ${FREETYPE} ${ASSIMP} opengl32)

  file(GLOB_RECURSE VultrExeSources "src/platform/entry_point/win32_main.cpp")
  add_executable(${Executable} WIN32 ${VultrExeSources})
endif(WIN32)

target_link_libraries(${This} PUBLIC ${Libs})
target_include_directories(${This} PUBLIC ${TargetIncludeDirs})

target_link_libraries(${Executable} PUBLIC ${Libs} ${This})
target_include_directories(${Executable} PUBLIC ${TargetIncludeDirs})

target_precompile_headers(${This} PUBLIC pch.h)

add_executable(ShaderGenerator "src/tools/shader_generator/sources.cpp")
target_link_libraries(ShaderGenerator PUBLIC ${Libs} ${This})
target_include_directories(ShaderGenerator PUBLIC ${TargetIncludeDirs})

add_subdirectory(tests)

add_subdirectory(benchmark)


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

#    set(CMAKE_CXX_FLAGS "")
