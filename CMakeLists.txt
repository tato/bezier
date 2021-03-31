cmake_minimum_required (VERSION 3.16.3)
project (bezier)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw-3.3.3)

find_package(OpenGL REQUIRED)

add_executable (bezier src/main.cpp)
target_include_directories(bezier PRIVATE src)
target_include_directories(bezier PRIVATE lib/glfw-3.3.3/include)
target_link_libraries(bezier glfw)
target_link_libraries(bezier OpenGL::GL)
