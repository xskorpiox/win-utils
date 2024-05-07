#include <napi.h>
//#include "../../node_modules/node-addon-api/napi.h" // for debug only
#include <Windows.h>
#include <vector>
#include <string>
#include <winioctl.h>
#include <iostream>

// Get removable disk letters
Napi::Array GetRemovableDrives(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    std::vector<std::string> drives;
    DWORD driveMask = GetLogicalDrives();

    for (char letter = 'A'; letter <= 'Z'; letter++) {
        if (driveMask & (1 << (letter - 'A'))) {
            std::string driveLetter(1, letter);
            std::string drivePath = driveLetter + ":\\";
            UINT type = GetDriveType(drivePath.c_str());

            if (type == DRIVE_REMOVABLE) {
                drives.push_back(Napi::String::New(env, driveLetter));
            }
        }
    }

    Napi::Array array = Napi::Array::New(env, drives.size());

    for (size_t i = 0; i < drives.size(); i++) {
        array[i] = Napi::String::New(env, drives[i]);
    }

    return array;
}

// FN for set disk attributes
bool SetDiskAttributes(const std::string& driveLetter, DWORD attributes) {
    std::string physicalDrivePath = "\\\\.\\" + driveLetter + ":"; // looks like \\.\C:

    HANDLE hDevice = CreateFile(
            physicalDrivePath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr,
            OPEN_EXISTING,
            0,
            nullptr
    );


    if (hDevice == INVALID_HANDLE_VALUE) {
        DWORD errorCode = GetLastError();  // get err code
        std::cerr << "Failed to open device. Error code: " << errorCode << std::endl;
        return false;
    }

    SET_DISK_ATTRIBUTES diskAttributes = {};
    diskAttributes.Version = sizeof(SET_DISK_ATTRIBUTES);
    diskAttributes.Persist = 0; // attribute should be default after reboot
    diskAttributes.AttributesMask = DISK_ATTRIBUTE_READ_ONLY;

    if (attributes != 0) {
        diskAttributes.Attributes = DISK_ATTRIBUTE_READ_ONLY;
    } else {
        diskAttributes.Attributes = 0;
    }

    DWORD bytesReturned;
    BOOL success = DeviceIoControl(
            hDevice,
            IOCTL_DISK_SET_DISK_ATTRIBUTES,
            &diskAttributes,
            sizeof(diskAttributes),
            nullptr,
            0,
            &bytesReturned,
            nullptr
    );

    CloseHandle(hDevice);

    if (success == 0) {
        DWORD errorCode = GetLastError();  // get err code
        std::cerr << "Failed to set attributes. Error code: " << errorCode << std::endl;
        return success;
    }
    return success;
}

// Function which set attributes for disk with specific letter
// (disk: string, attributes: number)
Napi::Value SetSpecificDiskAttribute(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsString() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected a disk letter and a numeric argument for attributes").ThrowAsJavaScriptException();
        return env.Null();
    }

    std::string driveLetter = info[0].As<Napi::String>().Utf8Value();  // parse disk letter
    DWORD attributes = info[1].As<Napi::Number>().Uint32Value();  // parse readonly flag

    bool success = SetDiskAttributes(driveLetter, attributes);

    return Napi::Boolean::New(env, success);
}