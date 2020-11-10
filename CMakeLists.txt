cmake_minimum_required(VERSION 3.17)
project(password_validator)
set(CMAKE_CXX_STANDARD 20)
find_package(Boost 1.67.0 REQUIRED)
find_package(fmt REQUIRED)

include_directories(
        PUBLIC ${Boost_INCLUDE_DIRS}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/mitama-cpp-result/include
)

add_executable(password_validator main.cpp)
target_link_libraries(password_validator ${Boost_LIBRARIES} dl fmt::fmt)
