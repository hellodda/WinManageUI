#include "pch.h"
#include "Win32Helper.h"
#include <Microsoft.UI.Xaml.Window.h>

void Win32Helper::DisableMultiInstanceEntry(std::wstring_view const appname, UINT const exitcode)
{
    auto constexpr max_size{ 16 };
    assert(appname.size() < max_size);
   
    auto name{ std::to_array<wchar_t, (MAX_PATH + 2) + max_size>({}) };
    
    auto const length{ ::GetTempPathW(static_cast<DWORD>(name.size()), &name[0]) };
    
    std::memcpy(&name[length], appname.data(), appname.size() * sizeof(wchar_t));
    std::replace(&name[0], &name[length], '\\', '/');
   
    if (!CreateMutexExW(nullptr, &name[0], CREATE_MUTEX_INITIAL_OWNER, NULL)) [[likely]]
    {
        for (auto previous{ FindWindowExW(nullptr, nullptr, nullptr, nullptr) }; previous != nullptr; previous = FindWindowExW(nullptr, previous, nullptr, nullptr)) [[likely]]
        {
            if (GetPropW(previous, appname.data())) [[unlikely]]
            {
                ShowWindow(previous, SW_RESTORE);

                if (SetForegroundWindow(previous))
                    ExitProcess(exitcode);
            }
        }
    }
}

HWND Win32Helper::GetHandleFromWindow(winrt::Microsoft::UI::Xaml::Window const& window)
{
    auto handle{ HWND{} };
    window.try_as<IWindowNative>()->get_WindowHandle(&handle);
    return handle;
}

void Win32Helper::DisableMultiInstanceWindow(winrt::Microsoft::UI::Xaml::Window const& window, std::wstring_view const appname)
{
    auto window_handle{ GetHandleFromWindow(window) };
    SetPropW(window_handle, appname.data(), window_handle);
}
