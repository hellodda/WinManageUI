#pragma once

#include "App.xaml.g.h"

namespace winrt::WinManageUI::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:
        winrt::Microsoft::UI::Xaml::Window m_window{ nullptr };
        winrt::WinManageUI::RootPage m_rootPage{ nullptr };
        winrt::hstring m_appName{ L"WinManageUI" };
    };
}
