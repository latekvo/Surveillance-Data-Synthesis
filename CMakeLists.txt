cmake_minimum_required(VERSION 4.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(app)

find_package(OpenCV REQUIRED)
find_package(raylib REQUIRED)
find_package(onnxruntime REQUIRED)

add_executable(app main.cpp list_parser.cpp preprocess.cpp postprocess.cpp detection.cpp utils.cpp)

target_include_directories(app PRIVATE ${raylib_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(app PRIVATE onnxruntime::onnxruntime ${raylib_LIBRARIES} ${OpenCV_LIBS})

