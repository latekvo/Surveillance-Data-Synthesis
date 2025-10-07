cmake_minimum_required(VERSION 4.1)
project(app)
find_package(OpenCV REQUIRED)
find_package(raylib REQUIRED)
add_executable(app main.cpp)
target_include_directories(app PRIVATE ${raylib_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(app PRIVATE ${raylib_LIBS} ${OpenCV_LIBS})

