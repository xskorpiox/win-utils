#include <napi.h>
#include <windows.h>

// find window by title
Napi::Value GetWindowHWNDbyTitle(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // check arg
    if (info.Length() < 1 || !info[0].IsString()) {
        Napi::TypeError::New(env, "String expected").ThrowAsJavaScriptException();
        return env.Null();
    }

    // get title from 0 arg
    std::string windowTitle = info[0].As<Napi::String>();

    // find by title
    HWND hwnd = FindWindowA(NULL, windowTitle.c_str());

    // check is hwnd exists
    if (hwnd == NULL) {
        return Napi::Number::New(env, 0); // if not return 0
    }

    // return HWND
    return Napi::Number::New(env, reinterpret_cast<uintptr_t>(hwnd));
}