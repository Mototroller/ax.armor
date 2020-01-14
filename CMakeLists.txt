cmake_minimum_required(VERSION 2.8)
project(AX_ARMOR_TEST)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(AX_WARNINGS "-Wall -pedantic -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Winit-self -Wmissing-include-dirs -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-promo -Wstrict-overflow=1 -Wundef -Wno-unused")


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++${CMAKE_CXX_STANDARD} ${AX_WARNINGS}")

set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O1 -g")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -march=native")

include_directories(ax.armor/include)
add_executable(tests tests/tests.cpp)

target_link_libraries(tests ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
