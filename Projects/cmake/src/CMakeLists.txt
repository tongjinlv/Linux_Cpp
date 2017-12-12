# define the project name
set(project_name "Lesson3")
# set the project namee
project("${project_name}")
cmake_minimum_required(VERSION 2.8)
add_subdirectory(src)
# add opencv package to the project
set(OpenCV_DIR /usr/local/opencv2.4.13/share/OpenCV)
find_package( OpenCV REQUIRED )
MESSAGE(OpenCV version : ${OpenCV_VERSION})
# add opencv include directories to the project
include_directories( ${OpenCV_INCLUDE_DIRS} )
ADD_SUBDIRECTORY( src )
add_executable( lesson3 main.cpp)
# link libraries
target_link_libraries( lesson3 ${OpenCV_LIBS})
