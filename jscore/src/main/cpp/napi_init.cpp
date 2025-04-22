#include "JSVMException.h"
#include "napi/native_api.h"
#include "jsvm_utils.h"
#include <cstdint>
#include "napi_utils.h"

static napi_value napi_CreateJsCore(napi_env env, napi_callback_info info) {
    uint32_t coreID;
    CreateJsCore(&coreID);
    return UInt32ToNValue(env, coreID);
}

static napi_value napi_ReleaseJsCore(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    NAPI_CALL(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    uint32_t coreEnvId = NValueToUInt32(env, args[0]);
    
    int32_t flag = ReleaseJsCore(coreEnvId);
    return Int32ToNValue(env, flag);
}

static napi_value napi_EvaluateJS(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2] = {nullptr};
    NAPI_CALL(napi_get_cb_info(env, info, &argc, args, nullptr, nullptr));
    
    uint32_t coreEnvId = NValueToUInt32(env, args[0]);
    std::string source = NValueToString(env, args[1]);
    
    std::string res;
    try {
        EvaluateJS(coreEnvId, source.c_str(), res);
        return StringToNValue(env,res);
    } catch (const JSVMException& e) {
        napi_throw_error(env, nullptr, e.what());
    } catch (const std::exception& e) {
        napi_throw_error(env, nullptr, e.what());
    } catch (...) {
        napi_throw_error(env, nullptr, "unkown error");
    }
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
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

extern "C" __attribute__((constructor)) void RegisterJscoreModule(void) {
    napi_module_register(&demoModule);
}
