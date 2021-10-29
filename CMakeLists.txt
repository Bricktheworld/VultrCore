cmake_minimum_required(VERSION 3.19.4)
project(Vultr VERSION 0.1.0)

enable_testing()

file(
  GLOB_RECURSE Vultr_SRC
  "vendor/*.cpp"
  "src/main.cpp"
)


if(UNIX)
  set(CMAKE_CXX_STANDARD 20)
  find_package(ASSIMP REQUIRED)
  find_package(GTest CONFIG REQUIRED)
  include_directories(${ASSIMP_INCLUDE_DIRS})
endif(UNIX)

if(WIN32)
  if(MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("/std:c++latest" _cpp_latest_flag_supported)
    if(_cpp_latest_flag_supported)
      add_compile_options("/std:c++latest")
    endif()
  endif()
endif(WIN32)


if(UNIX)
  # add_library(Vultr SHARED ${Vultr_SRC})
  add_executable(Vultr ${Vultr_SRC})
  target_link_libraries(Vultr PUBLIC GL glfw3 X11 Xxf86vm Xrandr pthread Xi dl freetype ${ASSIMP_LIBRARIES})
  target_precompile_headers(Vultr PUBLIC pch.h)
  target_include_directories(Vultr PUBLIC src vendor)
  # add_subdirectory(tests)
endif(UNIX)

if(WIN32)
  find_library(GLFW3 glfw3 HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(FREETYPE freetyped HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  find_library(ASSIMP assimp HINTS ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libs/WIN32)
  # add_executable(Vultr ${Vultr_SRC})
  add_library(Vultr STATIC ${Vultr_SRC})
  target_precompile_headers(Vultr PUBLIC pch.h)
  target_link_libraries(Vultr ${GLFW3} ${FREETYPE} ${ASSIMP} opengl32)
  add_custom_command(
    TARGET Vultr POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/vendor/libs/WIN32/assimp-vc142-mtd.dll"
    $<TARGET_FILE_DIR:Vultr>
  )

endif(WIN32)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DENABLE_DEBUG_MACRO")
set(CMAKE_CXX_FLAGS "-Wno-reorder -Wreorder-init-list")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
