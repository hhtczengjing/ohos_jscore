//
// Created on 2025/3/1.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "JSVMException.h"
#include <sstream>

// 构造函数实现
JSVMException::JSVMException(int code, const std::string& msg) : errorCode(code), errorMessage(msg) {}

int JSVMException::getErrorCode() const noexcept {
    return errorCode;
}

const std::string& JSVMException::getErrorMessage() const noexcept {
    return errorMessage;
}

const char* JSVMException::what() const noexcept {
    std::ostringstream oss;
    oss << "[" << errorCode << "]" << errorMessage;
    static std::string result =  oss.str();
    return result.c_str();
}