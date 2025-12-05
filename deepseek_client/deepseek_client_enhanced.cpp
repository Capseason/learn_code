#include <iostream>
#include <string>
#include <curl/curl.h>
#include <memory>
#include <sstream>
#include <fstream>
#include <map>

// 简单的JSON解析器（轻量级实现）
class SimpleJsonParser {
public:
    static std::string extractString(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) {
            return "";
        }
        
        // 找到冒号
        pos = json.find(":", pos);
        if (pos == std::string::npos) {
            return "";
        }
        
        // 跳过空白字符
        pos++;
        while (pos < json.length() && (json[pos] == ' ' || json[pos] == '\t')) {
            pos++;
        }
        
        // 检查是否是字符串值
        if (pos >= json.length() || json[pos] != '"') {
            return "";
        }
        pos++; // 跳过引号
        
        // 提取字符串值（处理转义）
        std::string result;
        bool escaped = false;
        while (pos < json.length()) {
            char c = json[pos];
            if (escaped) {
                if (c == 'n') {
                    result += '\n';
                } else if (c == 't') {
                    result += '\t';
                } else if (c == 'r') {
                    result += '\r';
                } else {
                    result += c;
                }
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                break;
            } else {
                result += c;
            }
            pos++;
        }
        
        return result;
    }
    
    static std::string extractArrayContent(const std::string& json, const std::string& key) {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) {
            return "";
        }
        
        pos = json.find("[", pos);
        if (pos == std::string::npos) {
            return "";
        }
        
        size_t start = pos + 1;
        int depth = 1;
        pos++;
        
        while (pos < json.length() && depth > 0) {
            if (json[pos] == '[') depth++;
            else if (json[pos] == ']') depth--;
            pos++;
        }
        
        if (depth == 0) {
            return json.substr(start, pos - start - 1);
        }
        
        return "";
    }
};

struct ResponseData {
    std::string data;
    
    ResponseData() : data("") {}
};

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
    std::string model;
    double temperature;
    int maxTokens;
    CURL* curl;

    bool initCurl() {
        curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Failed to initialize CURL" << std::endl;
            return false;
        }
        return true;
    }

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

    std::string buildRequestJson(const std::string& prompt) {
        std::ostringstream json;
        json << "{"
             << "\"model\": \"" << model << "\","
             << "\"messages\": ["
             << "{\"role\": \"user\", \"content\": \"" << escapeJson(prompt) << "\"}"
             << "],"
             << "\"temperature\": " << temperature << ","
             << "\"max_tokens\": " << maxTokens
             << "}";
        return json.str();
    }

public:
    DeepSeekClient(const std::string& key, 
                   const std::string& url = "https://api.deepseek.com/v1/chat/completions",
                   const std::string& modelName = "deepseek-chat",
                   double temp = 0.7,
                   int maxToks = 2000)
        : apiKey(key), apiUrl(url), model(modelName), 
          temperature(temp), maxTokens(maxToks), curl(nullptr) {
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

    std::string chat(const std::string& prompt) {
        if (!curl) {
            return "Error: CURL not initialized";
        }

        std::string jsonData = buildRequestJson(prompt);
        
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string authHeader = "Authorization: Bearer " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());

        ResponseData responseData;

        curl_easy_setopt(curl, CURLOPT_URL, apiUrl.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);

        if (res != CURLE_OK) {
            std::string error = "CURL error: ";
            error += curl_easy_strerror(res);
            return error;
        }

        long responseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);

        if (responseCode != 200) {
            return "HTTP Error " + std::to_string(responseCode) + ": " + responseData.data;
        }

        return responseData.data;
    }

    std::string extractMessage(const std::string& jsonResponse) {
        // DeepSeek API响应格式: {"choices": [{"message": {"content": "..."}}]}
        // 首先查找choices数组
        std::string choices = SimpleJsonParser::extractArrayContent(jsonResponse, "choices");
        if (!choices.empty()) {
            // 在choices数组的第一个元素中查找message对象
            std::string message = SimpleJsonParser::extractString(choices, "message");
            if (!message.empty()) {
                // 从message对象中提取content
                std::string content = SimpleJsonParser::extractString(message, "content");
                if (!content.empty()) {
                    return content;
                }
            }
            // 如果message格式不同，直接尝试从choices中提取content
            std::string content = SimpleJsonParser::extractString(choices, "content");
            if (!content.empty()) {
                return content;
            }
        }
        
        // 如果上述方法都失败，尝试在整个响应中查找content
        std::string content = SimpleJsonParser::extractString(jsonResponse, "content");
        if (!content.empty()) {
            return content;
        }
        
        // 如果都找不到，返回原始响应（可能包含错误信息）
        return jsonResponse;
    }
    
    void printFormattedResponse(const std::string& jsonResponse) {
        std::string message = extractMessage(jsonResponse);
        std::cout << "\n=== DeepSeek Response ===" << std::endl;
        std::cout << message << std::endl;
        std::cout << "========================\n" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    std::cout << "=== DeepSeek C++ Client Demo (Enhanced) ===" << std::endl;
    std::cout << std::endl;

    std::string apiKey;
    
    // 优先从命令行参数获取
    if (argc > 1) {
        apiKey = argv[1];
    } else {
        // 从环境变量获取
        const char* envKey = std::getenv("DEEPSEEK_API_KEY");
        if (envKey) {
            apiKey = envKey;
        } else {
            std::cerr << "Error: Please provide DeepSeek API key" << std::endl;
            std::cerr << "Usage: ./deepseek_client_enhanced <api_key>" << std::endl;
            std::cerr << "   or: export DEEPSEEK_API_KEY=<api_key>" << std::endl;
            return 1;
        }
    }

    try {
        DeepSeekClient client(apiKey);

        // 示例1: 中文对话
        std::cout << "Request 1: 中文问候" << std::endl;
        std::string response1 = client.chat("你好，请用中文简单介绍一下DeepSeek");
        client.printFormattedResponse(response1);

        // 示例2: 代码生成
        std::cout << "Request 2: 代码生成" << std::endl;
        std::string response2 = client.chat("用C++实现一个简单的HTTP客户端类，包含GET和POST方法");
        client.printFormattedResponse(response2);

        // 示例3: 数学问题
        std::cout << "Request 3: 数学问题" << std::endl;
        std::string response3 = client.chat("解释一下什么是快速傅里叶变换(FFT)，并给出其时间复杂度");
        client.printFormattedResponse(response3);

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
