//
// Created by user on 5/7/2024.
//
#include <napi.h>
//#include "../node_modules/node-addon-api/napi.h" // for debug only
#include "usb/usb.cc"
#include "audio/audio.cc"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // audio
    exports.Set(Napi::String::New(env, "getActiveAudioOutputSync"), Napi::Function::New(env, GetActiveAudioOutputSync));
    exports.Set(Napi::String::New(env, "getActiveAudioOutputAsync"), Napi::Function::New(env, GetActiveAudioOutputAsync));

    // usb
    exports.Set("getRemovableDrives", Napi::Function::New(env, GetRemovableDrives));
    exports.Set("setDiskAttribute", Napi::Function::New(env, SetSpecificDiskAttribute));

    return exports;
}

NODE_API_MODULE(addon, Init)