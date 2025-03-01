#include "JSVMException.h"
#include "napi/native_api.h"
#include "jsvm_utils.h"
#include <cstdint>

// assuming env is defined
#define NAPI_CALL_RET(call, return_value)                                                                              \
    do {                                                                                                               \
        napi_status status = (call);                                                                                   \
        if (status != napi_ok) {                                                                                       \
            const napi_extended_error_info *error_info = nullptr;                                                      \
            napi_get_last_error_info(env, &error_info);                                                                \
            bool is_pending;                                                                                           \
            napi_is_exception_pending(env, &is_pending);                                                               \
            if (!is_pending) {                                                                                         \
                auto message = error_info->error_message ? error_info->error_message : "null";                         \
                napi_throw_error(env, nullptr, message);                                                               \
                return return_value;                                                                                   \
            }                                                                                                          \
        }                                                                                                              \
    } while (0)

#define NAPI_CALL(call) NAPI_CALL_RET(call, nullptr)

bool IsNValueUndefined(napi_env env, napi_value value) {
    napi_valuetype type;
    if (napi_typeof(env, value, &type) == napi_ok && type == napi_undefined) {
        return true;
    }
    return false;
}

static std::string NValueToString(napi_env env, napi_value value, bool maybeUndefined = false) {
    if (maybeUndefined && IsNValueUndefined(env, value)) {
        return "";
    }

    size_t size;
    NAPI_CALL_RET(napi_get_value_string_utf8(env, value, nullptr, 0, &size), "");
    std::string result(size, '\0');
    NAPI_CALL_RET(napi_get_value_string_utf8(env, value, (char *) result.data(), size + 1, nullptr), "");
    return result;
}

static napi_value StringToNValue(napi_env env, const std::string &value) {
    napi_value result;
    napi_create_string_utf8(env, value.data(), value.size(), &result);
    return result;
}

static napi_value Int32ToNValue(napi_env env, int32_t value) {
    napi_value result;
    napi_create_int32(env, value, &result);
    return result;
}

static int32_t NValueToInt32(napi_env env, napi_value value) {
    int32_t result;
    napi_get_value_int32(env, value, &result);
    return result;
}

static napi_value UInt32ToNValue(napi_env env, uint32_t value) {
    napi_value result;
    napi_create_uint32(env, value, &result);
    return result;
}

static uint32_t NValueToUInt32(napi_env env, napi_value value) {
    uint32_t result;
    napi_get_value_uint32(env, value, &result);
    return result;
}

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
