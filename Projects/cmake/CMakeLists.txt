# define the project name
set(project_name "matata")
# set the project namee
project("${project_name}")
SET(CMAKE_C_COMPILER "g++")
cmake_minimum_required(VERSION 2.8)
# add opencv package to the project
set(OpenCV_DIR /usr/local/opencv2.4.13/share/OpenCV)
find_package( OpenCV REQUIRED )
MESSAGE(OpenCV version : ${OpenCV_VERSION})
# add opencv include directories to the project
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( matata  main.cpp)
# link libraries
target_link_libraries( matata  ${OpenCV_LIBS})
