# DeepSeek C++ Client Demo

这是一个使用C++实现的DeepSeek API客户端示例程序。

## 功能特性

- 支持调用DeepSeek Chat API
- 使用libcurl进行HTTP请求
- 简单的JSON处理
- 支持从环境变量或命令行参数读取API密钥
- 提供基础版本和增强版本两个实现

## 版本说明

### 基础版本 (deepseek_client.cpp)
- 简单直接的实现
- 基本的JSON解析
- 适合快速上手

### 增强版本 (deepseek_client_enhanced.cpp)
- 改进的JSON解析器
- 更好的响应格式化输出
- 更多示例请求
- 更完善的错误处理

## 依赖要求

- C++17或更高版本
- libcurl开发库

### Ubuntu/Debian系统安装依赖

```bash
sudo apt-get update
sudo apt-get install -y libcurl4-openssl-dev build-essential
```

### CentOS/RHEL系统安装依赖

```bash
sudo yum install -y libcurl-devel gcc-c++
```

## 编译方法

### 方法1: 使用Makefile（推荐）

```bash
cd deepseek_client
make
```

这会编译两个版本：
- `deepseek_client` - 基础版本
- `deepseek_client_enhanced` - 增强版本

### 方法2: 使用CMake

```bash
cd deepseek_client
mkdir build
cd build
cmake ..
make
```

### 方法3: 直接使用g++

基础版本：
```bash
cd deepseek_client
g++ -std=c++17 -o deepseek_client deepseek_client.cpp -lcurl
```

增强版本：
```bash
cd deepseek_client
g++ -std=c++17 -o deepseek_client_enhanced deepseek_client_enhanced.cpp -lcurl
```

## 使用方法

### 方式1: 使用命令行参数

基础版本：
```bash
./deepseek_client YOUR_API_KEY
```

增强版本：
```bash
./deepseek_client_enhanced YOUR_API_KEY
```

### 方式2: 使用环境变量

```bash
export DEEPSEEK_API_KEY=YOUR_API_KEY
./deepseek_client
# 或
./deepseek_client_enhanced
```

## 获取API密钥

1. 访问 [DeepSeek官网](https://www.deepseek.com/)
2. 注册账号并登录
3. 在控制台中创建API密钥
4. 将API密钥保存到环境变量或作为命令行参数传入

## 示例输出

### 基础版本
程序会发送两个示例请求：
1. 中文问候和自我介绍
2. 代码生成请求（快速排序函数）

### 增强版本
程序会发送三个示例请求：
1. 中文问候和DeepSeek介绍
2. 代码生成请求（HTTP客户端类）
3. 数学问题（快速傅里叶变换）

## 代码说明

- `DeepSeekClient` 类：封装了与DeepSeek API的交互
- `chat()` 方法：发送聊天请求并返回JSON响应
- `extractMessage()` 方法：从JSON响应中提取消息内容（简单实现）

## 注意事项

1. 本示例使用简单的字符串处理来解析JSON，生产环境建议使用专业的JSON库（如nlohmann/json）
2. 确保API密钥的安全性，不要将密钥提交到版本控制系统
3. 程序需要网络连接才能访问DeepSeek API

## 扩展建议

- 添加更完善的JSON解析（使用nlohmann/json库）
- 支持流式响应（SSE）
- 添加错误处理和重试机制
- 支持多轮对话上下文
- 添加配置文件支持

## 许可证

本示例代码仅供学习和参考使用。
