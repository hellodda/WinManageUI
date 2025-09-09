#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include <Helpers/Win32Helper.h>
#include <Helpers/SettingsHelper.h>

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

namespace winrt::WinManageUI::implementation
{
    App::App()
    {
        InitializeComponent();

        Win32Helper::DisableMultiInstanceEntry(m_appName, 1);

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
    }

    void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
    {
        m_window = winrt::Microsoft::UI::Xaml::Window{};
        m_rootPage = winrt::WinManageUI::RootPage{};

        m_window.Content(m_rootPage);
        
        auto appWindow{ m_window.AppWindow() };
        //appWindow.SetIcon(L"AAA");

        m_window.ExtendsContentIntoTitleBar(true);
        m_window.Activated([&self = *this](winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs const&)
        {
            Win32Helper::DisableMultiInstanceWindow(sender.try_as<Microsoft::UI::Xaml::Window>(), self.m_appName);
        });

        m_window.Activate();
    }
}
