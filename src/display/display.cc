#define UNICODE
#define _UNICODE

#include <napi.h>
#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <cstdarg>
#include <cstdarg>
#include "common.cc"

#pragma comment(lib, "User32.lib")

// main
Napi::Value GetDisplayInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // set UTF-8
    SetConsoleOutputCP(CP_UTF8);

    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        Napi::TypeError::New(env, "Failed to get buffer sizes").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(
            QDC_ONLY_ACTIVE_PATHS | QDC_INCLUDE_HMD, // QDC_ONLY_ACTIVE_PATHS | QDC_DATABASE_CURRENT | QDC_ALL_PATHS
            &pathCount,
            paths.data(),
            &modeCount,
            modes.data(),
            nullptr) != ERROR_SUCCESS) {
        Napi::TypeError::New(env, "Failed to query display config").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Array result = Napi::Array::New(env);

    int index = 0;
    for (const auto& path : paths) {
        Napi::Object monitorInfo = Napi::Object::New(env);

        // is monitor active
        bool isActive = (path.flags & DISPLAYCONFIG_PATH_ACTIVE) != 0;
        monitorInfo.Set("isActive", Napi::Boolean::New(env, isActive));

        // connection type
        DISPLAYCONFIG_VIDEO_OUTPUT_TECHNOLOGY outputTechnology = path.targetInfo.outputTechnology;
        monitorInfo.Set("outputTechnology", Napi::Number::New(env, static_cast<int>(outputTechnology)));

        // scaling
        DISPLAYCONFIG_SCALING scaling = path.targetInfo.scaling;
        monitorInfo.Set("scaling", Napi::Number::New(env, static_cast<int>(scaling)));

        // display rotation
        DISPLAYCONFIG_ROTATION rotation = path.targetInfo.rotation;
        monitorInfo.Set("rotation", Napi::Number::New(env, static_cast<int>(rotation)));

        // isStereo (flag)
#ifdef DISPLAYCONFIG_PATH_STEREO
        bool isStereo = (path.flags & DISPLAYCONFIG_PATH_STEREO) != 0;
        monitorInfo.Set("isStereo", Napi::Boolean::New(env, isStereo));
#else
        monitorInfo.Set("isStereo", Napi::Boolean::New(env, false));
#endif

        // get monitor info
        DISPLAYCONFIG_TARGET_DEVICE_NAME targetDeviceName = {};
        targetDeviceName.header.adapterId = path.targetInfo.adapterId;
        targetDeviceName.header.id = path.targetInfo.id;
        targetDeviceName.header.size = sizeof(targetDeviceName);
        targetDeviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;

        MONITORINFOEXW monitorInfoEx = {};

        if (DisplayConfigGetDeviceInfo(&targetDeviceName.header) == ERROR_SUCCESS) {

            // get HMONITOR from monitor configuration
            POINTL point = { 0, 0 };
            for (const auto& mode : modes) {
                if (mode.id == path.sourceInfo.id &&
                    mode.adapterId.LowPart == path.sourceInfo.adapterId.LowPart &&
                    mode.adapterId.HighPart == path.sourceInfo.adapterId.HighPart &&
                    mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {

                    point.x = mode.sourceMode.position.x;
                    point.y = mode.sourceMode.position.y;
                    break;
                }
            }

            HMONITOR hMonitor = MonitorFromPoint(*(POINT*)&point, MONITOR_DEFAULTTONEAREST);
            monitorInfoEx.cbSize = sizeof(MONITORINFOEXW);
            if (GetMonitorInfoW(hMonitor, &monitorInfoEx)) {
                // generate Id like chromium engine (m114) version
                int64_t monitorId = GenerateDisplayId(monitorInfoEx);
                monitorInfo.Set("id", Napi::Number::New(env, static_cast<double>(monitorId)));

                // monitor coordinates
                monitorInfo.Set("positionX", Napi::Number::New(env, monitorInfoEx.rcMonitor.left));
                monitorInfo.Set("positionY", Napi::Number::New(env, monitorInfoEx.rcMonitor.top));
                monitorInfo.Set("width", Napi::Number::New(env, monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left));
                monitorInfo.Set("height", Napi::Number::New(env, monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top));

                // work area
                monitorInfo.Set("workAreaX", Napi::Number::New(env, monitorInfoEx.rcWork.left));
                monitorInfo.Set("workAreaY", Napi::Number::New(env, monitorInfoEx.rcWork.top));
                monitorInfo.Set("workAreaWidth", Napi::Number::New(env, monitorInfoEx.rcWork.right - monitorInfoEx.rcWork.left));
                monitorInfo.Set("workAreaHeight", Napi::Number::New(env, monitorInfoEx.rcWork.bottom - monitorInfoEx.rcWork.top));

                // is primary
                bool isPrimary = (monitorInfoEx.dwFlags & MONITORINFOF_PRIMARY) != 0;
                monitorInfo.Set("isPrimary", Napi::Boolean::New(env, isPrimary));
            }
        }

        // DISPLAYCONFIG_TARGET_DEVICE_NAME info
        monitorInfo.Set("outputTechnology", Napi::Number::New(env, static_cast<int>(targetDeviceName.outputTechnology)));
        monitorInfo.Set("edidManufactureId", Napi::Number::New(env, targetDeviceName.edidManufactureId));
        monitorInfo.Set("edidProductCodeId", Napi::Number::New(env, targetDeviceName.edidProductCodeId));
        monitorInfo.Set("connectorInstance", Napi::Number::New(env, targetDeviceName.connectorInstance));
        monitorInfo.Set("monitorFriendlyDeviceName", Napi::String::New(env, WideToUTF8(targetDeviceName.monitorFriendlyDeviceName)));
        monitorInfo.Set("monitorDevicePath", Napi::String::New(env, WideToUTF8(targetDeviceName.monitorDevicePath)));
        monitorInfo.Set("szDevice", Napi::String::New(env, WideToUTF8(monitorInfoEx.szDevice)));
        // is extended
        bool isExtended = isActive && !monitorInfo.Get("isPrimary").As<Napi::Boolean>().Value();
        monitorInfo.Set("isExtended", Napi::Boolean::New(env, isExtended));

        // if no monitorInfoEx, set id as 0
        if (monitorInfo.Has("id") == false) {
            monitorInfo.Set("id", Napi::Number::New(env, 0));
        }

        // refresh rate
        double refreshRate = static_cast<double>(path.targetInfo.refreshRate.Numerator) /
                             static_cast<double>(path.targetInfo.refreshRate.Denominator);
        monitorInfo.Set("refreshRate", Napi::Number::New(env, refreshRate));

        // additional fields
        monitorInfo.Set("sourceId", Napi::Number::New(env, path.sourceInfo.id));
        monitorInfo.Set("targetId", Napi::Number::New(env, path.targetInfo.id));
        monitorInfo.Set("adapterIdHigh", Napi::Number::New(env, static_cast<double>(path.targetInfo.adapterId.HighPart)));
        monitorInfo.Set("adapterIdLow", Napi::Number::New(env, static_cast<double>(path.targetInfo.adapterId.LowPart)));

        // resolution and position (if not exists in monitorInfoEx)
        if (!monitorInfo.Has("width")) {
            UINT32 width = 0, height = 0;
            INT32 positionX = 0, positionY = 0;

            // get resolution and position from source
            for (const auto& mode : modes) {
                if (mode.id == path.sourceInfo.id &&
                    mode.adapterId.LowPart == path.sourceInfo.adapterId.LowPart &&
                    mode.adapterId.HighPart == path.sourceInfo.adapterId.HighPart &&
                    mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE) {

                    width = mode.sourceMode.width;
                    height = mode.sourceMode.height;
                    positionX = mode.sourceMode.position.x;
                    positionY = mode.sourceMode.position.y;
                    break;
                }
            }

            monitorInfo.Set("width", Napi::Number::New(env, width));
            monitorInfo.Set("height", Napi::Number::New(env, height));
            monitorInfo.Set("positionX", Napi::Number::New(env, positionX));
            monitorInfo.Set("positionY", Napi::Number::New(env, positionY));
        }

        result.Set(index++, monitorInfo);
    }

    return result;
}