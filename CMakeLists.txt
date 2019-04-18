cmake_minimum_required(VERSION 3.1)
project(WaterCoherer)

add_executable(core
  src/main.cpp
  src/NDWICalculator.cpp
  src/WaterDifferencer.cpp
  src/CloudDetection.cpp)

target_include_directories(core PUBLIC include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O3 -lpthread -ltiff -lX11")
