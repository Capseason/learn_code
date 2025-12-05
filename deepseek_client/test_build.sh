#!/bin/bash

# DeepSeek Client 编译测试脚本

echo "=== DeepSeek C++ Client 编译测试 ==="
echo ""

# 检查依赖
echo "检查依赖..."
if ! command -v g++ &> /dev/null; then
    echo "错误: 未找到 g++ 编译器"
    exit 1
fi

if ! pkg-config --exists libcurl; then
    echo "警告: 未找到 libcurl 开发库"
    echo "请安装: sudo apt-get install libcurl4-openssl-dev (Ubuntu/Debian)"
    echo "     或: sudo yum install libcurl-devel (CentOS/RHEL)"
    exit 1
fi

echo "依赖检查通过"
echo ""

# 测试编译基础版本
echo "编译基础版本..."
g++ -std=c++17 -o deepseek_client deepseek_client.cpp -lcurl 2>&1
if [ $? -eq 0 ]; then
    echo "✓ 基础版本编译成功"
    rm -f deepseek_client
else
    echo "✗ 基础版本编译失败"
    exit 1
fi

# 测试编译增强版本
echo "编译增强版本..."
g++ -std=c++17 -o deepseek_client_enhanced deepseek_client_enhanced.cpp -lcurl 2>&1
if [ $? -eq 0 ]; then
    echo "✓ 增强版本编译成功"
    rm -f deepseek_client_enhanced
else
    echo "✗ 增强版本编译失败"
    exit 1
fi

echo ""
echo "=== 所有编译测试通过 ==="
echo ""
echo "要实际编译，请运行:"
echo "  make"
echo "或"
echo "  mkdir build && cd build && cmake .. && make"
