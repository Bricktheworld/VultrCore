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


if(UNIX)
  set(CMAKE_CXX_STANDARD 20)
  find_package(ASSIMP REQUIRED)
  find_package(GTest CONFIG REQUIRED)
  # include_directories(${ASSIMP_INCLUDE_DIRS})
endif(UNIX)

if(WIN32)
  find_library(GLFW3 glfw3 HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(FREETYPE freetyped HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(ASSIMP assimp HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
endif(WIN32)

include_directories(src vendor)

add_library(${This} STATIC ${Sources})

set(TargetIncludeDirs src vendor)

if(UNIX)
  set(Libs GL glfw3 X11 Xxf86vm Xrandr pthread Xi dl freetype ${ASSIMP_LIBRARIES})
  file(GLOB_RECURSE VultrExeSources "src/platform/entry_point/linux_main.cpp")
endif(UNIX)

if(WIN32)
  find_library(GLFW3 glfw3 HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(FREETYPE freetyped HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(ASSIMP assimp HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  set(Libs ${GLFW3} ${FREETYPE} ${ASSIMP} opengl32)
  file(GLOB_RECURSE VultrExeSources "src/platform/entry_point/win32_main.cpp")
endif(WIN32)

add_executable(${Executable} ${VultrExeSources})

target_link_libraries(${This} PUBLIC ${Libs})
target_include_directories(${This} PUBLIC ${TargetIncludeDirs})

target_link_libraries(${Executable} PUBLIC ${Libs} ${This})
target_include_directories(${Executable} PUBLIC ${TargetIncludeDirs})

target_precompile_headers(${This} PUBLIC pch.h)

add_subdirectory(tests)


if(WIN32)
  add_custom_command(
    TARGET Vultr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/vendor/libs/WIN32/assimp-vc142-mtd.dll"
    $<TARGET_FILE_DIR:Vultr>
  )
endif(WIN32)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_CXX_FLAGS "-Wno-reorder -fdeduce-init-list")
