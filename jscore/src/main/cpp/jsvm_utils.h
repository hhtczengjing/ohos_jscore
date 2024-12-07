//
// Created on 2024/12/6.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef JSVM_DEMO_JSVM_UTILS_H
#define JSVM_DEMO_JSVM_UTILS_H

#include <string>

int CreateJsCore(uint32_t *result);

int ReleaseJsCore(uint32_t coreEnvId);

int EvaluateJS(uint32_t envId, const char *source, std::string &res);

#endif //JSVM_DEMO_JSVM_UTILS_H
