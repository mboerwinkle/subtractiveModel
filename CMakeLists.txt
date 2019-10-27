cmake_minimum_required(VERSION 2.8)
project( subtractiveModel )
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
include_directories(
        ${PROJECT_SOURCE_DIR}/src
)
file(GLOB all_src ${PROJECT_SOURCE_DIR}/src/*.cpp)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( subtractiveModel ${all_src})
target_link_libraries( subtractiveModel ${OpenCV_LIBS} pthread)
