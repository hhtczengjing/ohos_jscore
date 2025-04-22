//
// Created on 2025/3/13.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef OHOS_NAPI_UTILS_H
#define OHOS_NAPI_UTILS_H

#include "napi/native_api.h"
#include <string>
#include <vector>

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

static napi_value NAPIUndefined(napi_env env) {
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

static napi_value NAPINull(napi_env env) {
    napi_value result;
    napi_get_null(env, &result);
    return result;
}

static napi_value BoolToNValue(napi_env env, bool value) {
    napi_value result;
    napi_value resultBool;
    napi_create_double(env, value, &result);
    napi_coerce_to_bool(env, result, &resultBool);
    return resultBool;
}

static bool NValueToBool(napi_env env, napi_value value) {
    bool result;
    if (napi_get_value_bool(env, value, &result) == napi_ok) {
        return result;
    }
    return false;
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

static napi_value DoubleToNValue(napi_env env, double value) {
    napi_value result;
    napi_create_double(env, value, &result);
    return result;
}

static double NValueToDouble(napi_env env, napi_value value) {
    double result;
    napi_get_value_double(env, value, &result);
    return result;
}

static napi_value Int64ToNValue(napi_env env, int64_t value) {
    napi_value result;
    napi_create_bigint_int64(env, value, &result);
    return result;
}

static int64_t NValueToInt64(napi_env env, napi_value value) {
    int64_t result;
    bool lossless;
    napi_get_value_bigint_int64(env, value, &result, &lossless);
    return result;
}

static napi_value UInt64ToNValue(napi_env env, uint64_t value) {
    napi_value result;
    napi_create_bigint_uint64(env, value, &result);
    return result;
}

static uint64_t NValueToUInt64(napi_env env, napi_value value, bool maybeUndefined = false) {
    if (maybeUndefined && IsNValueUndefined(env, value)) {
        return 0;
    }
    uint64_t result;
    bool lossless;
    napi_get_value_bigint_uint64(env, value, &result, &lossless);
    return result;
}

static std::vector<std::string> NValueToStringArray(napi_env env, napi_value value, bool maybeUndefined = false) {
    std::vector<std::string> keys;
    if (maybeUndefined && IsNValueUndefined(env, value)) {
        return keys;
    }

    uint32_t length = 0;
    if (napi_get_array_length(env, value, &length) != napi_ok || length == 0) {
        return keys;
    }
    keys.reserve(length);

    for (uint32_t index = 0; index < length; index++) {
        napi_value jsKey = nullptr;
        if (napi_get_element(env, value, index, &jsKey) != napi_ok) {
            continue;
        }
        keys.push_back(NValueToString(env, jsKey));
    }
    return keys;
}

static napi_value StringArrayToNValue(napi_env env, const std::vector<std::string> &value) {
    napi_value jsArr = nullptr;
    napi_create_array_with_length(env, value.size(), &jsArr);
    for (size_t index = 0; index < value.size(); index++) {
        auto jsKey = StringToNValue(env, value[index]);
        napi_set_element(env, jsArr, index, jsKey);
    }
    return jsArr;
}

#endif //OHOS_NAPI_UTILS_H
