# CMake最低版本
cmake_minimum_required(VERSION 3.10)

# 项目名 语言
project(image_renderer C CXX)

# C++版本
set(CMAKE_CXX_STANDARD 20)

# 子模块
add_subdirectory(src/config)
add_subdirectory(src/consul)
add_subdirectory(resources/protos)

# 可执行程序的输出目录
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
execute_process(
  COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_SOURCE_DIR}/bin
)

# 生成文件路径
set(GENERATED_OUT_PATH ${PROJECT_SOURCE_DIR}/generated)

# 查找OpenCV
find_package(OpenCV REQUIRED)

# include
include_directories(${GENERATED_OUT_PATH})
include_directories(${PROJECT_SOURCE_DIR}/src)
include_directories(${OpenCV_INCLUDE_DIRS})

set(SOURCES
    main.cpp
    src/utils/random_utils.h
    src/utils/random_utils.cpp
    /workspace/src/grpc/clients/image_harmony/image_harmony_client.h
    /workspace/src/grpc/clients/image_harmony/image_harmony_client.cpp
    /workspace/src/grpc/clients/target_detection/target_detection.h
    /workspace/src/grpc/clients/target_detection/target_detection.cpp
    /workspace/src/grpc/servers/service_coordinator/service_coordinator_server.h
    /workspace/src/grpc/servers/service_coordinator/service_coordinator_server.cpp
    /workspace/src/grpc/servers/grpc_server_builder.h
    /workspace/src/grpc/servers/grpc_server_builder.cpp
    /workspace/src/grpc/servers/grpc_server.h
    /workspace/src/grpc/servers/grpc_server.cpp
    /workspace/src/task_manager/task_manager.h
    /workspace/src/task_manager/task_manager.cpp
    )

add_executable(${PROJECT_NAME} ${SOURCES})

# 子模块
target_link_libraries(${PROJECT_NAME} 
  config
  consul-cpp
  image_harmony_data_grpc_proto
  service_coordinator_data_grpc_proto
  )

# OpenCV
target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
