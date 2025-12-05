cmake_minimum_required(VERSION 3.10)
project(DeepSeekClient)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找CURL库
find_package(CURL REQUIRED)

# 包含目录
include_directories(${CURL_INCLUDE_DIRS})

# 基础版本
add_executable(deepseek_client deepseek_client.cpp)
target_link_libraries(deepseek_client ${CURL_LIBRARIES})

# 增强版本
add_executable(deepseek_client_enhanced deepseek_client_enhanced.cpp)
target_link_libraries(deepseek_client_enhanced ${CURL_LIBRARIES})

# 编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(deepseek_client PRIVATE -Wall -Wextra)
endif()
