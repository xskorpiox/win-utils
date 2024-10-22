#define UNICODE
#define _UNICODE

// min version = Windows 10
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10
#endif

#include <napi.h>
#include <windows.h>
#include <ShellScalingAPI.h> // after windows.h
#include <vector>
#include <string>
#include <iostream>
#include <cstdarg>
#include "common.cc"

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shcore.lib") // For scalling

// main
Napi::Value GetDisplayInfo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // set UTF-8
    SetConsoleOutputCP(CP_UTF8);

    DPI_AWARENESS_CONTEXT previousContext = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    UINT32 pathCount = 0, modeCount = 0;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) {
        Napi::TypeError::New(env, "Failed to get buffer sizes").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::vector<DISPLAYCONFIG_PATH_INFO> paths(pathCount);
    std::vector<DISPLAYCONFIG_MODE_INFO> modes(modeCount);

    if (QueryDisplayConfig(
            QDC_ONLY_ACTIVE_PATHS | QDC_INCLUDE_HMD,
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

        bool isInternal = (outputTechnology == DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL);
        monitorInfo.Set("isInternal", Napi::Boolean::New(env, isInternal));

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
        HMONITOR hMonitor = NULL;

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

            hMonitor = MonitorFromPoint(*(POINT*)&point, MONITOR_DEFAULTTONEAREST);
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

        DPI_AWARENESS_CONTEXT previousContext = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        // get scaling
        UINT dpiX = 96, dpiY = 96; // default values
        if (hMonitor) {
            HRESULT hr = GetDpiForMonitor(hMonitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);
            std::cout << "hr: " << hr << std::endl;
            if (SUCCEEDED(hr)) {
                // calculate scape percent
                double scalingFactor = dpiX / 96.0 * 100; // 96 DPI === 100%
                monitorInfo.Set("scalingPercentage", Napi::Number::New(env, scalingFactor));
            } else {
                monitorInfo.Set("scalingPercentage", Napi::Number::New(env, 100.0));
            }
        } else {
            monitorInfo.Set("scalingPercentage", Napi::Number::New(env, 100.0));
        }

        result.Set(index++, monitorInfo);
    }

    SetThreadDpiAwarenessContext(previousContext);

    return result;
}
