#include <napi.h>
#include <windows.h>

Napi::Value SendMessageToWindow(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // check args count
    if (info.Length() < 4) {
        Napi::TypeError::New(env, "Expected four arguments: HWND, UINT, WPARAM, LPARAM").ThrowAsJavaScriptException();
        return env.Null();
    }

    // check args type
    if (!info[0].IsNumber() || !info[1].IsNumber() || !info[2].IsNumber() || !info[3].IsNumber()) {
        Napi::TypeError::New(env, "Expected arguments of type Number").ThrowAsJavaScriptException();
        return env.Null();
    }

    // get args from js
    HWND hwnd = (HWND)(UINT_PTR)info[0].As<Napi::Number>().Int64Value();
    UINT msg = (UINT)info[1].As<Napi::Number>().Uint32Value();
    WPARAM wParam = (WPARAM)info[2].As<Napi::Number>().Int64Value();
    LPARAM lParam = (LPARAM)info[3].As<Napi::Number>().Int64Value();

    // send args to window
    LRESULT result = SendMessage(hwnd, msg, wParam, lParam);

    // return call result
    return Napi::Number::New(env, result);
}