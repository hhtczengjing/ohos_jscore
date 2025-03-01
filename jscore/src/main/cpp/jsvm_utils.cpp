//
// Created on 2024/12/6.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include "jsvm_utils.h"
#include "napi/native_api.h"
#include "ark_runtime/jsvm.h"
#include "ark_runtime/jsvm_types.h"
#include "common.h"

#include <bits/alltypes.h>
#include <deque>
#include <map>
#include <unistd.h>
#include <hilog/log.h>

#include <cstring>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

// 定义map管理每个独立vm环境
static map<int, JSVM_VM*> g_vmMap;
static map<int, JSVM_VMScope> g_vmScopeMap;
static map<int, JSVM_Env*> g_envMap;
static map<int, napi_env> g_napiEnvMap;
static map<int, JSVM_EnvScope> g_envScopeMap;
static map<int, napi_ref> g_callBackMap;
static map<int, JSVM_CallbackStruct*> g_callBackStructMap;
static uint32_t ENVTAG_NUMBER = 0;
static std::mutex envMapLock;
static int g_aa = 0;

class Task {
public:
    virtual ~Task() = default;
    virtual void Run() = 0;
};
static map<int, deque<Task *>> g_taskQueueMap;

// 自定义创建Promise方法用以在JS代码中创建Promise
static JSVM_Value CreatePromise(JSVM_Env env, JSVM_CallbackInfo info) {
    OH_LOG_INFO(LOG_APP, "JSVM API TEST: CreatePromise start");
    int envID = -1;
    // 通过当前env获取envID
    for (auto it = g_envMap.begin(); it != g_envMap.end(); ++it) {
        if (*it->second == env) {
            envID = it->first;
            break;
        }
    }
    if (envID == -1) {
        OH_LOG_ERROR(LOG_APP, "JSVM API TEST: CreatePromise envID faild");
        return nullptr;
    }
    JSVM_Value promise;
    JSVM_Deferred deferred;
    JSVM_CALL(env, OH_JSVM_CreatePromise(env, &deferred, &promise));
    // 设计ReadTask类用以将promise对象的deferred加入执行队列
    class ReadTask : public Task {
    public:
        ReadTask(JSVM_Env env, JSVM_Deferred deferred, int envNum) : env_(env), envID_(envNum), deferred_(deferred) {}
        void Run() override {
            // string str = "TEST RUN OH_JSVM_ResolveDeferred";
            int envID = 0;
            for (auto it = g_envMap.begin(); it != g_envMap.end(); ++it) {
                if (*it->second == env_) {
                    envID = it->first;
                    break;
                }
            }
            OH_LOG_INFO(LOG_APP, "JSVM API TEST: CreatePromise %{public}d", envID);
            JSVM_Value result;
            if (OH_JSVM_CreateInt32(env_, envID, &result) != JSVM_OK) {
                return;
            }
            if (OH_JSVM_ResolveDeferred(env_, deferred_, result) != JSVM_OK) {
                return;
            }
        }

    private:
        JSVM_Env env_;
        int envID_;
        JSVM_Deferred deferred_;
    };
    g_taskQueueMap[envID].push_back(new ReadTask(env, deferred, envID));
    OH_LOG_INFO(LOG_APP, "JSVM API TEST: CreatePromise end");
    return promise;
}

// 用以在native层中调用TS侧传入的Callback函数
static JSVM_Value OnJSResultCallback(JSVM_Env env, JSVM_CallbackInfo info) {
    size_t argc = 3;
    JSVM_Value args[3];
    JSVM_CALL(env, OH_JSVM_GetCbInfo(env, info, &argc, args, NULL, NULL));
    int callId = 0;
    OH_JSVM_GetValueInt32(env, args[0], &callId);
    napi_value callArgs[2] = {nullptr, nullptr};
    size_t size;
    size_t size1;

    OH_JSVM_GetValueStringUtf8(env, args[1], nullptr, 0, &size);
    char Str1[size + 1];
    OH_JSVM_GetValueStringUtf8(env, args[1], Str1, size + 1, &size);
    
    OH_JSVM_GetValueStringUtf8(env, args[2], nullptr, 0, &size1);
    char Str2[size1 + 1];
    OH_JSVM_GetValueStringUtf8(env, args[2], Str2, size1 + 1, &size1);
    
    napi_create_string_utf8(g_napiEnvMap[callId], Str1, size + 1, &callArgs[0]);
    napi_create_string_utf8(g_napiEnvMap[callId], Str2, size1 + 1, &callArgs[1]);
    napi_value callback = nullptr;
    // 通过callId获取在创建当前JSVM环境时传入的TS回调方法
    napi_get_reference_value(g_napiEnvMap[callId], g_callBackMap[callId], &callback);
    napi_value ret;
    // 执行TS回调方法
    napi_call_function(g_napiEnvMap[callId], nullptr, callback, 2, callArgs, &ret);
    char retStr[256];
    napi_get_value_string_utf8(g_napiEnvMap[callId], ret, retStr, 256, &size);
    
    JSVM_Value returnVal;
    OH_JSVM_CreateStringUtf8(env, retStr, JSVM_AUTO_LENGTH, &returnVal);
    return returnVal;
}

// 自定义AssertEqual方法
static JSVM_Value AssertEqual(JSVM_Env env, JSVM_CallbackInfo info) {
    size_t argc = 2;
    JSVM_Value args[2];
    JSVM_CALL(env, OH_JSVM_GetCbInfo(env, info, &argc, args, NULL, NULL));

    bool isStrictEquals = false;
    JSVM_CALL(env, OH_JSVM_StrictEquals(env, args[0], args[1], &isStrictEquals));

    if (isStrictEquals) {
        OH_LOG_INFO(LOG_APP, "JSVM API TEST RESULT: PASS");
    } else {
        OH_LOG_INFO(LOG_APP, "JSVM API TEST RESULT: FAILED");
    }
    return nullptr;
}

static int fromOHStringValue(JSVM_Env &env, JSVM_Value &value, std::string &result) {
    size_t size;
    CHECK_RET(OH_JSVM_GetValueStringUtf8(env, value, nullptr, 0, &size));
    char resultStr[size + 1];
    CHECK_RET(OH_JSVM_GetValueStringUtf8(env, value, resultStr, size + 1, &size));
    result = resultStr;
    return 0;
}

// 提供创建JSVM运行环境的对外接口并返回对应唯一ID
int CreateJsCore(uint32_t *result) {
    OH_LOG_INFO(LOG_APP, "JSVM CreateJsCore START");
    g_taskQueueMap[ENVTAG_NUMBER] = deque<Task *>{};

    if (g_aa == 0) {
        JSVM_InitOptions init_options;
        memset(&init_options, 0, sizeof(init_options));
        CHECK(OH_JSVM_Init(&init_options));
        g_aa++;
    }
    std::lock_guard<std::mutex> lock_guard(envMapLock);

    // 虚拟机实例
    g_vmMap[ENVTAG_NUMBER] = new JSVM_VM;
    JSVM_CreateVMOptions options;
    JSVM_VMScope vmScope;
    memset(&options, 0, sizeof(options));
    CHECK(OH_JSVM_CreateVM(&options, g_vmMap[ENVTAG_NUMBER]));
    CHECK(OH_JSVM_OpenVMScope(*g_vmMap[ENVTAG_NUMBER], &vmScope));

    // 新环境
    g_envMap[ENVTAG_NUMBER] = new JSVM_Env;
    g_callBackStructMap[ENVTAG_NUMBER] = new JSVM_CallbackStruct[3];

    // 注册用户提供的本地函数的回调函数指针和数据，通过JSVM-API暴露给js
    for (int i = 0; i < 3; i++) {
        g_callBackStructMap[ENVTAG_NUMBER][i].data = nullptr;
    }
    g_callBackStructMap[ENVTAG_NUMBER][0].callback = OnJSResultCallback;
    g_callBackStructMap[ENVTAG_NUMBER][1].callback = AssertEqual;
    g_callBackStructMap[ENVTAG_NUMBER][2].callback = CreatePromise;
    JSVM_PropertyDescriptor descriptors[] = {
        {"onJSResultCallback", NULL, &g_callBackStructMap[ENVTAG_NUMBER][0], NULL, NULL, NULL, JSVM_DEFAULT},
        {"assertEqual", NULL, &g_callBackStructMap[ENVTAG_NUMBER][1], NULL, NULL, NULL, JSVM_DEFAULT},
        {"createPromise", NULL, &g_callBackStructMap[ENVTAG_NUMBER][2], NULL, NULL, NULL, JSVM_DEFAULT},
    };
    CHECK(OH_JSVM_CreateEnv(*g_vmMap[ENVTAG_NUMBER], sizeof(descriptors) / sizeof(descriptors[0]), descriptors, g_envMap[ENVTAG_NUMBER]));
    CHECK(OH_JSVM_CloseVMScope(*g_vmMap[ENVTAG_NUMBER], vmScope));

    OH_LOG_INFO(LOG_APP, "JSVM CreateJsCore END");
    *result = ENVTAG_NUMBER;
    ENVTAG_NUMBER++;
    return 0;
}

// 对外提供释放JSVM环境接口，通过envId释放对应环境
int ReleaseJsCore(uint32_t coreEnvId) {
    OH_LOG_INFO(LOG_APP, "JSVM ReleaseJsCore START");
    CHECK_VAL(g_envMap.count(coreEnvId) != 0 && g_envMap[coreEnvId] != nullptr);

    std::lock_guard<std::mutex> lock_guard(envMapLock);

    CHECK(OH_JSVM_DestroyEnv(*g_envMap[coreEnvId]));
    g_envMap[coreEnvId] = nullptr;
    g_envMap.erase(coreEnvId);
    CHECK(OH_JSVM_DestroyVM(*g_vmMap[coreEnvId]));
    g_vmMap[coreEnvId] = nullptr;
    g_vmMap.erase(coreEnvId);
    delete[] g_callBackStructMap[coreEnvId];
    g_callBackStructMap[coreEnvId] = nullptr;
    g_callBackStructMap.erase(coreEnvId);
    g_taskQueueMap.erase(coreEnvId);

    OH_LOG_INFO(LOG_APP, "JSVM ReleaseJsCore END");
    return 0;
}

static std::mutex mutexLock;
// 对外提供执行JS代码接口，通过coreID在对应的JSVN环境中执行JS代码
int EvaluateJS(uint32_t envId, const char *source, std::string &res) {
    OH_LOG_INFO(LOG_APP, "JSVM EvaluateJS START");

    CHECK_VAL(g_envMap.count(envId) != 0 && g_envMap[envId] != nullptr);

    JSVM_Env env = *g_envMap[envId];
    JSVM_VM vm = *g_vmMap[envId];
    JSVM_VMScope vmScope;
    JSVM_EnvScope envScope;
    JSVM_HandleScope handleScope;
    JSVM_Value result;

    std::lock_guard<std::mutex> lock_guard(mutexLock);
    {
        // 创建JSVM环境
        CHECK_RET(OH_JSVM_OpenVMScope(vm, &vmScope));
        CHECK_RET(OH_JSVM_OpenEnvScope(*g_envMap[envId], &envScope));
        CHECK_RET(OH_JSVM_OpenHandleScope(*g_envMap[envId], &handleScope));

        // 通过script调用测试函数
        JSVM_Script script;
        JSVM_Value jsSrc;
        CHECK_RET(OH_JSVM_CreateStringUtf8(env, source, JSVM_AUTO_LENGTH, &jsSrc));
        CHECK_THROW_RET(OH_JSVM_CompileScript(env, jsSrc, nullptr, 0, true, nullptr, &script));
        CHECK_THROW_RET(OH_JSVM_RunScript(env, script, &result));

        JSVM_ValueType type;
        CHECK_RET(OH_JSVM_Typeof(env, result, &type));
        OH_LOG_INFO(LOG_APP, "JSVM API TEST type: %{public}d", type);
        // Execute tasks in the current env event queue
        while (!g_taskQueueMap[envId].empty()) {
            auto task = g_taskQueueMap[envId].front();
            g_taskQueueMap[envId].pop_front();
            task->Run();
            delete task;
        }

        if (type == JSVM_STRING) {
            CHECK_VAL(fromOHStringValue(env, result, res) != -1);
        } else if (type == JSVM_BOOLEAN) {
            bool ret = false;
            CHECK_RET(OH_JSVM_GetValueBool(env, result, &ret));
            ret ? res = "true" : res = "false";
        } else if (type == JSVM_NUMBER) {
            int32_t num;
            CHECK_RET(OH_JSVM_GetValueInt32(env, result, &num));
            res = std::to_string(num);
        } else if (type == JSVM_OBJECT) {
            JSVM_Value objResult;
            CHECK_RET(OH_JSVM_JsonStringify(env, result, &objResult));
            CHECK_VAL(fromOHStringValue(env, objResult, res) != -1);
        }
    }
    {
        bool aal = false;
        CHECK_RET(OH_JSVM_PumpMessageLoop(*g_vmMap[envId], &aal));
        CHECK_RET(OH_JSVM_PerformMicrotaskCheckpoint(*g_vmMap[envId]));
        CHECK_RET(OH_JSVM_CloseHandleScope(*g_envMap[envId], handleScope));
        CHECK_RET(OH_JSVM_CloseEnvScope(*g_envMap[envId], envScope));
        CHECK_RET(OH_JSVM_CloseVMScope(*g_vmMap[envId], vmScope));
    }
    OH_LOG_INFO(LOG_APP, "JSVM EvaluateJS END");
    return 0;
}