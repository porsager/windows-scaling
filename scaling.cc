#define _WIN32_WINNT 0x0601
#define WINVER 0x0601

#include <nan.h>
#include <Windows.h>
#include <wingdi.h>
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
    static UINT32 GetScaling() {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths;
        std::vector<DISPLAYCONFIG_MODE_INFO> modes;

        if (!DpiHelper::GetPathsAndModes(paths, modes)) {
            Nan::ThrowError(FormatErrorMessage("Failed to retrieve display configuration paths and modes", GetLastError()).c_str());
        }

        for (const auto& path : paths) {
            if (path.flags & DISPLAYCONFIG_PATH_ACTIVE) {
                auto dpiInfo = DpiHelper::GetDPIScalingInfo(path.sourceInfo.adapterId, path.sourceInfo.id);
                if (dpiInfo.bInitDone) return dpiInfo.current;
                Nan::ThrowError(FormatErrorMessage("Failed to retrieve DPI scaling information", GetLastError()).c_str());
            }
        }

        Nan::ThrowError("No active display path found.");
        return 0;
    }

    static bool SetScaling(UINT32 scaling) {
        std::vector<DISPLAYCONFIG_PATH_INFO> paths;
        std::vector<DISPLAYCONFIG_MODE_INFO> modes;

        if (!DpiHelper::GetPathsAndModes(paths, modes)) {
            Nan::ThrowError(FormatErrorMessage("Failed to retrieve display configuration paths and modes", GetLastError()).c_str());
        }

        for (const auto& path : paths) {
            if (path.flags & DISPLAYCONFIG_PATH_ACTIVE) {
                if (DpiHelper::SetDPIScaling(path.sourceInfo.adapterId, path.sourceInfo.id, scaling)) return true;
                Nan::ThrowError(FormatErrorMessage("Failed to set DPI scaling", GetLastError()).c_str());
            }
        }

        Nan::ThrowError("No active display path found.");
        return false;
    }
};

NAN_METHOD(GetScaling) {
    info.GetReturnValue().Set(Nan::New(DpiWrapper::GetScaling()));
}

NAN_METHOD(SetScaling) {
    if (info.Length() < 1 || !info[0]->IsNumber()) {
        Nan::ThrowTypeError("Scaling value must be a number.");
        return;
    }

    UINT32 scaling = Nan::To<UINT32>(info[0]).FromJust();
    info.GetReturnValue().Set(Nan::New(DpiWrapper::SetScaling(scaling)));
}

NAN_MODULE_INIT(Init) {
    Nan::Set(target, Nan::New("get").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GetScaling)).ToLocalChecked());
    Nan::Set(target, Nan::New("set").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(SetScaling)).ToLocalChecked());
}

NODE_MODULE(scaling, Init)
