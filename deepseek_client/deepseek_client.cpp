#include <iostream>
#include <string>
#include <curl/curl.h>
#include <memory>
#include <sstream>

// JSON处理结构体（简单实现，也可以使用nlohmann/json库）
struct ResponseData {
    std::string data;
    
    ResponseData() : data("") {}
};

// CURL写回调函数
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    ResponseData* response = static_cast<ResponseData*>(userp);
    response->data.append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

class DeepSeekClient {
private:
    std::string apiKey;
    std::string apiUrl;
    CURL* curl;

    // 初始化CURL
    bool initCurl() {
        curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL" << std::endl;
            return false;
        }
        return true;
    }

    // 构建JSON请求体
    std::string buildRequestJson(const std::string& prompt, const std::string& model = "deepseek-chat") {
        std::ostringstream json;
        json << "{"
             << "\"model\": \"" << model << "\","
             << "\"messages\": ["
             << "{\"role\": \"user\", \"content\": \"" << escapeJson(prompt) << "\"}"
             << "],"
             << "\"temperature\": 0.7,"
             << "\"max_tokens\": 2000"
             << "}";
        return json.str();
    }

    // 简单的JSON转义
    std::string escapeJson(const std::string& str) {
        std::string escaped;
        for (char c : str) {
            if (c == '"') {
                escaped += "\\\"";
            } else if (c == '\\') {
                escaped += "\\\\";
            } else if (c == '\n') {
                escaped += "\\n";
            } else if (c == '\r') {
                escaped += "\\r";
            } else if (c == '\t') {
                escaped += "\\t";
            } else {
                escaped += c;
            }
        }
        return escaped;
    }

public:
    DeepSeekClient(const std::string& key, const std::string& url = "https://api.deepseek.com/v1/chat/completions")
        : apiKey(key), apiUrl(url), curl(nullptr) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        if (!initCurl()) {
            throw std::runtime_error("Failed to initialize CURL");
        }
    }

    ~DeepSeekClient() {
        if (curl) {
            curl_easy_cleanup(curl);
        }
        curl_global_cleanup();
    }

    // 发送聊天请求
    std::string chat(const std::string& prompt, const std::string& model = "deepseek-chat") {
        if (!curl) {
            return "Error: CURL not initialized";
        }

        std::string jsonData = buildRequestJson(prompt, model);
        
        // 设置HTTP头
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());

        // 准备响应数据
        ResponseData responseData;

        // 配置CURL选项
        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        // 执行请求
        CURLcode res = curl_easy_perform(curl);

        // 清理
        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::string error = "CURL error: ";
            error += curl_easy_strerror(res);
            return error;
        }

        // 获取HTTP响应码
        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode != 200) {
            return "HTTP Error " + std::to_string(responseCode) + ": " + responseData.data;
        }

        return responseData.data;
    }

    // 从JSON响应中提取消息内容（简单解析）
    std::string extractMessage(const std::string& jsonResponse) {
        // 查找 "content" 字段
        size_t contentPos = jsonResponse.find("\"content\"");
        if (contentPos == std::string::npos) {
            return jsonResponse; // 如果找不到，返回原始响应
        }

        // 找到content值的位置
        size_t valueStart = jsonResponse.find("\"", contentPos + 9);
        if (valueStart == std::string::npos) {
            return jsonResponse;
        }
        valueStart++; // 跳过引号

        size_t valueEnd = jsonResponse.find("\"", valueStart);
        if (valueEnd == std::string::npos) {
            return jsonResponse;
        }

        std::string content = jsonResponse.substr(valueStart, valueEnd - valueStart);
        
        // 简单的unescape处理
        std::string unescaped;
        for (size_t i = 0; i < content.length(); ++i) {
            if (content[i] == '\\' && i + 1 < content.length()) {
                if (content[i + 1] == 'n') {
                    unescaped += '\n';
                    i++;
                } else if (content[i + 1] == '\\') {
                    unescaped += '\\';
                    i++;
                } else if (content[i + 1] == '"') {
                    unescaped += '"';
                    i++;
                } else {
                    unescaped += content[i];
                }
            } else {
                unescaped += content[i];
            }
        }
        
        return unescaped;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "=== DeepSeek C++ Client Demo ===" << std::endl;
    std::cout << std::endl;

    // 从环境变量或命令行参数获取API密钥
    std::string apiKey;
    if (argc > 1) {
        apiKey = argv[1];
    } else {
        const char* envKey = std::getenv("DEEPSEEK_API_KEY");
        if (envKey) {
            apiKey = envKey;
        } else {
            std::cerr << "Error: Please provide DeepSeek API key as:" << std::endl;
            std::cerr << "  1. Command line argument: ./deepseek_client <api_key>" << std::endl;
            std::cerr << "  2. Environment variable: export DEEPSEEK_API_KEY=<api_key>" << std::endl;
            return 1;
        }
    }

    try {
        // 创建客户端
        DeepSeekClient client(apiKey);

        // 示例1: 简单对话
        std::cout << "Sending request 1: Simple greeting..." << std::endl;
        std::string response1 = client.chat("你好，请用中文介绍一下你自己");
        std::cout << "Raw Response: " << response1 << std::endl;
        std::cout << std::endl;
        
        std::string message1 = client.extractMessage(response1);
        std::cout << "Extracted Message: " << message1 << std::endl;
        std::cout << std::endl;
        std::cout << "---" << std::endl;
        std::cout << std::endl;

        // 示例2: 代码生成
        std::cout << "Sending request 2: Code generation..." << std::endl;
        std::string response2 = client.chat("用C++写一个快速排序函数");
        std::cout << "Raw Response: " << response2 << std::endl;
        std::cout << std::endl;
        
        std::string message2 = client.extractMessage(response2);
        std::cout << "Extracted Message: " << message2 << std::endl;
        std::cout << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
