#include <napi.h>
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdarg>

// String formatting (analog base::StringPrintf)
std::string StringPrintf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    // copy va_list for reuse
    va_list args_copy;
    va_copy(args_copy, args);

    // set buffer size
    int size = vsnprintf(nullptr, 0, format, args_copy);
    va_end(args_copy);

    if (size < 0) {
        va_end(args);
        return std::string();
    }

    // create string with required size
    std::string result(size, '\0');

    // format string
    vsnprintf(&result[0], size + 1, format, args);

    va_end(args);

    return result;
}

// SuperFastHash function
uint32_t SuperFastHash(const char* data, int len) {
    if (len <= 0 || data == nullptr) {
        return 0;
    }

    uint32_t hash = static_cast<uint32_t>(len);
    uint32_t tmp;
    int rem = len & 3;
    len >>= 2;

    const uint16_t* data16 = reinterpret_cast<const uint16_t*>(data);

    // main loop
    for (; len > 0; len--) {
        hash += data16[0];
        tmp = (data16[1] << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        data16 += 2;
        hash += hash >> 11;
    }

    // handle other bytes
    const uint8_t* data8 = reinterpret_cast<const uint8_t*>(data16);

    switch (rem) {
        case 3:
            hash += data8[0] + (data8[1] << 8);
            hash ^= hash << 16;
            hash ^= data8[2] << 18;
            hash += hash >> 11;
            break;
        case 2:
            hash += data8[0] + (data8[1] << 8);
            hash ^= hash << 11;
            hash += hash >> 17;
            break;
        case 1:
            hash += data8[0];
            hash ^= hash << 10;
            hash += hash >> 1;
            break;
        default:
            break;
    }

    // finally shuffle bits
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 4;
    hash += hash >> 17;
    hash ^= hash << 25;
    hash += hash >> 6;

    return hash;
}

// PersistentHash using SuperFastHash
uint32_t PersistentHash(const std::string& data) {
    return SuperFastHash(data.data(), static_cast<int>(data.size()));
}

// convert wide-string to UTF-8 string
std::string WideToUTF8(const wchar_t* wstr) {
    if (!wstr) return std::string();

    int size_needed = WideCharToMultiByte(
            CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) return std::string();

    std::string strTo(size_needed - 1, 0);
    WideCharToMultiByte(
            CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, nullptr, nullptr);
    return strTo;
}

// Function which generate ID for display Chromium like (114 version)
int64_t GenerateDisplayId(const MONITORINFOEXW& monitorInfo) {
    // Use szDevice из MONITORINFOEXW for ID gen
    const wchar_t* deviceName = monitorInfo.szDevice;
    std::string utf8DeviceName = WideToUTF8(deviceName);
    uint32_t hash = PersistentHash(utf8DeviceName);

    // debug
    // std::cout << "Device Name: " << utf8DeviceName << ", Hash ID: " << hash << std::endl;

    return static_cast<int64_t>(hash);
}