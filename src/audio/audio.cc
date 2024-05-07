#include <iostream>
#include <napi.h>
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>

Napi::String GetActiveAudioOutputSync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IPropertyStore* pPropertyStore = NULL;
    LPWSTR pwszDeviceName = NULL;

    // COM initialization
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        Napi::TypeError::New(env, "Failed to initialize COM").ThrowAsJavaScriptException();
        return Napi::String::New(env, "");
    }

    // Creating an object to enumerate audio devices
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        Napi::TypeError::New(env, "Failed to create device enumerator").ThrowAsJavaScriptException();
        CoUninitialize();
        return Napi::String::New(env, "");
    }

    // Get the current active audio device
    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
    if (FAILED(hr)) {
        Napi::TypeError::New(env, "Failed to get default audio endpoint").ThrowAsJavaScriptException();
        pEnumerator->Release();
        CoUninitialize();
        return Napi::String::New(env, "");
    }

    // Getting audio device properties
    hr = pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
    if (FAILED(hr)) {
        Napi::TypeError::New(env, "Failed to open property store").ThrowAsJavaScriptException();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();
        return Napi::String::New(env, "");
    }

    // Getting the audio device name
    PROPERTYKEY keyFriendlyName = PKEY_Device_FriendlyName;
    PROPVARIANT varName;
    PropVariantInit(&varName);
    hr = pPropertyStore->GetValue(keyFriendlyName, &varName);
    if (SUCCEEDED(hr)) {
        pwszDeviceName = varName.pwszVal;
    }

    // clean
    PropVariantClear(&varName);
    pPropertyStore->Release();
    pDevice->Release();
    pEnumerator->Release();
    CoUninitialize();

    if (pwszDeviceName != NULL) {
        return Napi::String::New(env, reinterpret_cast<const char16_t*>(pwszDeviceName));
    } else {
        return Napi::String::New(env, "");
    }
}

// call the callback from first arg with {RESULT_STRING}
// info[0].As<Napi::Function>().Call(env.Global(), { Napi::String::New(env, {RESULT STRING} ) });

void GetActiveAudioOutputAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    // run async operation
    Napi::ThreadSafeFunction tsfn = Napi::ThreadSafeFunction::New(
            env,
            info[0].As<Napi::Function>(), // Callback
            "Win-Utils", // resource name (anything)
            0, // Maximum number of threads that can call this callback function simultaneously
            1 // Minimum number of threads that must be available to call this callback function
    );

    // async execution
    std::thread([tsfn]() mutable {
        HRESULT hr;
        IMMDeviceEnumerator *pEnumerator = NULL;
        IMMDevice *pDevice = NULL;
        IPropertyStore *pPropertyStore = NULL;
        LPWSTR pwszDeviceName = NULL;

        // COM initialization
        hr = CoInitialize(NULL);
        if (FAILED(hr)) {
            tsfn.NonBlockingCall([tsfn](Napi::Env env, Napi::Function jsCallback) {
                jsCallback.Call({ Napi::String::New(env, "") });
                tsfn.Release();
            });
            return;
        }

        // Creating an object to enumerate audio devices
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator),
                              (void **) &pEnumerator);
        if (FAILED(hr)) {
            CoUninitialize();
            tsfn.NonBlockingCall([tsfn](Napi::Env env, Napi::Function jsCallback) {
                jsCallback.Call({ Napi::String::New(env, "") });
                tsfn.Release();
            });
            return;
        }

        // Get the current active audio device
        hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, &pDevice);
        if (FAILED(hr)) {
            pEnumerator->Release();
            CoUninitialize();
            tsfn.NonBlockingCall([tsfn](Napi::Env env, Napi::Function jsCallback) {
                jsCallback.Call({ Napi::String::New(env, "") });
                tsfn.Release();
            });
            return;
        }

        // Getting audio device properties
        hr = pDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        if (FAILED(hr)) {
            pDevice->Release();
            pEnumerator->Release();
            CoUninitialize();
            tsfn.NonBlockingCall([tsfn](Napi::Env env, Napi::Function jsCallback) {
                jsCallback.Call({ Napi::String::New(env, "") });
                tsfn.Release();
            });
            return;
        }

        // Getting the audio device name
        PROPERTYKEY keyFriendlyName = PKEY_Device_FriendlyName;
        PROPVARIANT varName;
        PropVariantInit(&varName);
        hr = pPropertyStore->GetValue(keyFriendlyName, &varName);
        if (SUCCEEDED(hr)) {
            pwszDeviceName = varName.pwszVal;
        }

        // clean
        PropVariantClear(&varName);
        pPropertyStore->Release();
        pDevice->Release();
        pEnumerator->Release();
        CoUninitialize();

        // Callback invocation in the main Node.js thread
        tsfn.NonBlockingCall([pwszDeviceName, tsfn](Napi::Env env, Napi::Function jsCallback) {
            if (pwszDeviceName != NULL) {
                jsCallback.Call({Napi::String::New(env, reinterpret_cast<const char16_t *>(pwszDeviceName))});
            } else {
                jsCallback.Call({Napi::String::New(env, "")});
            }
            tsfn.Release();
            return;
        });

    }).detach();
}