cmake_minimum_required(VERSION 4.1)

project(app)

find_package(OpenCV REQUIRED)
find_package(raylib REQUIRED)
find_package(onnxruntime REQUIRED)

add_executable(app main.cpp coco_labels.cpp csv.cpp preprocess.cpp postprocess.cpp detection.cpp utils.cpp)

target_include_directories(app PRIVATE ${raylib_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS})
target_link_libraries(app PRIVATE onnxruntime::onnxruntime ${raylib_LIBRARIES} ${OpenCV_LIBS})

