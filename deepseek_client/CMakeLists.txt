cmake_minimum_required(VERSION 3.10)
project(DeepSeekClient)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找CURL库
find_package(CURL QUIET)

if(CURL_FOUND)
    # 使用find_package找到的CURL
    include_directories(${CURL_INCLUDE_DIRS})
    set(CURL_LIB ${CURL_LIBRARIES})
else()
    # 备选方案：直接查找curl库
    find_library(CURL_LIB curl)
    if(NOT CURL_LIB)
        message(FATAL_ERROR "CURL library not found. Please install libcurl4-openssl-dev")
    endif()
endif()

# 基础版本
add_executable(deepseek_client deepseek_client.cpp)
target_link_libraries(deepseek_client ${CURL_LIB})

# 增强版本
add_executable(deepseek_client_enhanced deepseek_client_enhanced.cpp)
target_link_libraries(deepseek_client_enhanced ${CURL_LIB})

# 编译选项
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(deepseek_client PRIVATE -Wall -Wextra)
    target_compile_options(deepseek_client_enhanced PRIVATE -Wall -Wextra)
endif()
