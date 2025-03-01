//
// Created on 2025/3/1.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef OHOS_JSCORE_JSVMEXCEPTION_H
#define OHOS_JSCORE_JSVMEXCEPTION_H

#include <exception>
#include <string>

class JSVMException : public std::exception {
private:
    int errorCode;
    std::string errorMessage;

public:
    JSVMException(int code, const std::string& msg);
    
    int getErrorCode() const noexcept;
    
    const std::string& getErrorMessage() const noexcept;
    
    const char* what() const noexcept override;
};

#endif //OHOS_JSCORE_JSVMEXCEPTION_H
