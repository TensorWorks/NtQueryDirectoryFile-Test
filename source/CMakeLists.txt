cmake_minimum_required(VERSION 3.22)
project(ntquerydirectoryfile-test)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build our test executable
add_executable(ntquerydirectoryfile-test ntquerydirectoryfile-test.cpp)
set_property(TARGET ntquerydirectoryfile-test PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
target_compile_definitions(ntquerydirectoryfile-test PUBLIC UNICODE)
target_compile_definitions(ntquerydirectoryfile-test PUBLIC _UNICODE)

# Install the executable to the output directory
install(TARGETS ntquerydirectoryfile-test RUNTIME DESTINATION bin)
