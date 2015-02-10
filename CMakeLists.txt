project(ros_wasabi)
cmake_minimum_required(VERSION 2.8)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
aux_source_directory(./src SRC_LIST)

# Add Qt5 support for this project
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
add_executable(${PROJECT_NAME} ${SRC_LIST})
## Configure the target
#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#add_library(WASABIEngine SHARED
#    src/AffectiveState.cc
#    src/cogaAttendee.cc
#    src/cogaEmotionalAttendee.cc
#    src/EmotionContainer.cc
#    src/EmotionConverter.cc
#    src/EmotionDynamics.cc
#    src/PrimaryEmotion.cc
#    src/SecondaryEmotion.cc
#    src/WASABIEngine.cc
#    src/WASABIRunner.cc
#)

# Link Qt to the library
qt5_use_modules(ros_wasabi Core)
