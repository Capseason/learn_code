# Makefile for DeepSeek C++ Client

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = -lcurl

TARGET = deepseek_client
TARGET_ENHANCED = deepseek_client_enhanced
SOURCE = deepseek_client.cpp
SOURCE_ENHANCED = deepseek_client_enhanced.cpp

# 默认目标
all: $(TARGET) $(TARGET_ENHANCED)

# 编译基础版本
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE) $(LDFLAGS)

# 编译增强版本
$(TARGET_ENHANCED): $(SOURCE_ENHANCED)
	$(CXX) $(CXXFLAGS) -o $(TARGET_ENHANCED) $(SOURCE_ENHANCED) $(LDFLAGS)

# 清理
clean:
	rm -f $(TARGET) $(TARGET_ENHANCED)

# 安装依赖 (Ubuntu/Debian)
install-deps:
	sudo apt-get update
	sudo apt-get install -y libcurl4-openssl-dev

# 安装依赖 (CentOS/RHEL)
install-deps-rhel:
	sudo yum install -y libcurl-devel

.PHONY: all clean install-deps install-deps-rhel
