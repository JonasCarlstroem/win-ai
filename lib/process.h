#pragma once

#include <Windows.h>
#include <string>
#include <iostream>

class process {
public:
    static bool launch(const std::wstring& cmd) {
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        
        BOOL success = CreateProcessW(
            nullptr,
            const_cast<wchar_t*>(cmd.c_str()),
            nullptr,
            nullptr,
            FALSE,
            0,
            nullptr,
            nullptr,
            &si,
            &pi
        );

        if (!success) {
            std::wcerr << L"[process] Failed to start: " << cmd << L"\n";
            return false;
        }

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return true;
    }
};