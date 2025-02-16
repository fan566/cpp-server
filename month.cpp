#include <iostream>
#include <climits>  // 用于 INT_MAX 和 INT_MIN
using namespace std;

int myAtoi(const char* str) {
    if (str == nullptr) {
        return 0;
    }
    
    // 跳过前导空白字符
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    // 处理正负号
    bool isNegative = false;
    if (*str == '-' || *str == '+') {
        isNegative = (*str == '-');
        str++;
    }
    
    // 转换数字字符
    long long result = 0;  // 使用long long避免溢出
    while (*str >= '0' && *str <= '9') {
        // 计算新的结果
        result = result * 10 + (*str - '0');
        
        // 检查溢出
        if (!isNegative && result > INT_MAX) {
            return INT_MAX;
        }
        if (isNegative && -result < INT_MIN) {
            return INT_MIN;
        }
        
        str++;
    }
    
    return isNegative ? -result : result;
}

int main() {
    // 测试用例
    const char* testCases[] = {
        "42",           // 普通正数
        "   -42",      // 带空格的负数
        "4193 with words",  // 带非数字字符
        "words and 987",    // 开头非数字
        "-91283472332",     // 溢出
        "+1",           // 带加号
        "   +0 123",    // 带前导零
        "2147483647",   // INT_MAX
        "-2147483648",  // INT_MIN
        ""             // 空字符串
    };
    
    for (const char* test : testCases) {
        cout << "输入字符串: \"" << test << "\"" << endl;
        cout << "转换结果: " << myAtoi(test) << endl << endl;
    }
    
    return 0;
}
