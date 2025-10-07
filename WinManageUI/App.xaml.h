#pragma once

#include "App.xaml.g.h"

#include "Utils/DependencyContainer.h"

namespace winrt::WinManageUI::implementation
{
    struct App : AppT<App>
    {
        App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

        static winrt::Microsoft::UI::Xaml::Window Window() noexcept;
        static Containers::DependencyContainer& Dependencies() noexcept;

    private:

        void RegisterDependencies();

    private:
        static winrt::Microsoft::UI::Xaml::Window m_window;
        static Containers::DependencyContainer m_container;

        winrt::WinManageUI::RootPage m_rootPage{ nullptr };
        winrt::hstring m_appName{ L"WinManageUI" };
    };
}
