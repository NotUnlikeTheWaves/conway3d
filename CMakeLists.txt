project("3D Game Of Life")
cmake_minimum_required(VERSION 3.0.0)
cmake_policy(SET CMP0072 OLD)

set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "")
set(FREEGLUT_BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(lib/FreeGLUT/freeglut/freeglut)

add_executable(out main.cpp)
target_include_directories(out PRIVATE lib/FreeGLUT/freeglut/freeglut/include)
target_link_libraries(out freeglut_static GLU)
