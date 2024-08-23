#define _WIN32_WINNT 0x0601
#define WINVER 0x0601

#include <nan.h>
#include <Windows.h>
#include <vector>
#include <string>
#include "DpiHelper.h"

std::string FormatErrorMessage(const std::string& message, DWORD errorCode) {
    char* buffer = nullptr;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL, errorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);
    std::string errorMsg = message + " (Error code: " + std::to_string(errorCode) + "): " + buffer;
    LocalFree(buffer);
    return errorMsg;
}

class DpiWrapper {
public:
    static UINT32 GetScaling(UINT32 displayIndex) {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths;
        std::vector<DISPLAYCONFIG_MODE_INFO> modes;

        if (!DpiHelper::GetPathsAndModes(paths, modes)) {
            Nan::ThrowError(FormatErrorMessage("Failed to retrieve display configuration paths and modes", GetLastError()).c_str());
        }

        if (displayIndex >= paths.size()) {
            Nan::ThrowError("Display index out of bounds.");
            return 0;
        }

        LUID adapterID = paths[displayIndex].sourceInfo.adapterId;
        UINT32 sourceID = paths[displayIndex].sourceInfo.id;

        auto dpiInfo = DpiHelper::GetDPIScalingInfo(adapterID, sourceID);
        if (dpiInfo.bInitDone) {
            return dpiInfo.current;
        } else {
            Nan::ThrowError(FormatErrorMessage("Failed to retrieve DPI scaling information", GetLastError()).c_str());
        }

        return 0;
    }

    static bool SetScaling(UINT32 displayIndex, UINT32 scaling) {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths;
        std::vector<DISPLAYCONFIG_MODE_INFO> modes;

        if (!DpiHelper::GetPathsAndModes(paths, modes)) {
            Nan::ThrowError(FormatErrorMessage("Failed to retrieve display configuration paths and modes", GetLastError()).c_str());
        }

        if (displayIndex >= paths.size()) {
            Nan::ThrowError("Display index out of bounds.");
            return false;
        }

        LUID adapterID = paths[displayIndex].sourceInfo.adapterId;
        UINT32 sourceID = paths[displayIndex].sourceInfo.id;

        return DpiHelper::SetDPIScaling(adapterID, sourceID, scaling);
    }
};

NAN_METHOD(GetScaling) {
    if (info.Length() < 1 || !info[0]->IsNumber()) {
        Nan::ThrowTypeError("Display index must be a number.");
        return;
    }

    UINT32 displayIndex = Nan::To<UINT32>(info[0]).FromJust();
    info.GetReturnValue().Set(Nan::New(DpiWrapper::GetScaling(displayIndex)));
}

NAN_METHOD(SetScaling) {
    if (info.Length() < 2 || !info[0]->IsNumber() || !info[1]->IsNumber()) {
        Nan::ThrowTypeError("Display index and scaling value must be numbers.");
        return;
    }

    UINT32 displayIndex = Nan::To<UINT32>(info[0]).FromJust();
    UINT32 scaling = Nan::To<UINT32>(info[1]).FromJust();
    info.GetReturnValue().Set(Nan::New(DpiWrapper::SetScaling(displayIndex, scaling)));
}

NAN_MODULE_INIT(Init) {
    Nan::Set(target, Nan::New("get").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GetScaling)).ToLocalChecked());
    Nan::Set(target, Nan::New("set").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(SetScaling)).ToLocalChecked());
}

NODE_MODULE(scaling, Init)
