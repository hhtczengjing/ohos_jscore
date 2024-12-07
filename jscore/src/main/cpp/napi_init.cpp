#include "napi/native_api.h"
#include "jsvm_utils.h"

static std::string napiValueToString(napi_env env, napi_value nValue) {
    size_t buffLen = 0;
    napi_get_value_string_utf8(env, nValue, nullptr, 0, &buffLen);
    char buffer[buffLen + 1];
    napi_get_value_string_utf8(env, nValue, buffer, buffLen + 1, &buffLen);
    return buffer;
}

static napi_value napi_CreateJsCore(napi_env env, napi_callback_info info) {
    uint32_t coreID;
    CreateJsCore(&coreID);
    
    napi_value result;
    napi_create_int32(env, coreID, &result);
    return result;
}

static napi_value napi_ReleaseJsCore(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    int32_t coreEnvId;
    napi_get_value_int32(env, args[0], &coreEnvId);
    
    int flag = ReleaseJsCore(coreEnvId);
    
    napi_value result;
    napi_create_int32(env, flag, &result);
    return result;
}

static napi_value napi_EvaluateJS(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    
    int32_t coreEnvId;
    napi_get_value_int32(env, args[0], &coreEnvId);
    
    std::string source = napiValueToString(env, args[1]);
    std::string res;
    EvaluateJS(coreEnvId, source.c_str(), res);
    
    napi_value result = nullptr;
    napi_create_string_utf8(env, res.c_str(), res.length(), &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"createJsCore", nullptr, napi_CreateJsCore, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"releaseJsCore", nullptr, napi_ReleaseJsCore, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"evaluateJS", nullptr, napi_EvaluateJS, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "jscore",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterJscoreModule(void)
{
    napi_module_register(&demoModule);
}
