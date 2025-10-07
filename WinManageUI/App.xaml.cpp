#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"

#include <Helpers/Win32Helper.h>
#include <Helpers/SettingsHelper.h>

#include "Utils/Logging.h"

using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;

namespace winrt::WinManageUI::implementation
{
    winrt::Microsoft::UI::Xaml::Window App::m_window{ nullptr };
    Containers::DependencyContainer App::m_container{ nullptr };

    App::App()
    {
        InitializeComponent();
        RegisterDependencies();

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

    Microsoft::UI::Xaml::Window App::Window() noexcept
    {
        return m_window;
    }

    Containers::DependencyContainer& App::Dependencies() noexcept
    {
        return m_container;
    }

    void App::RegisterDependencies()
    {
        m_container.RegisterInstance<winrt::WinMgmt::WmiDataContext>(Lifetime::Singleton ,"Default");
        m_container.RegisterInstance<winrt::WinMgmt::WmiDataContext>(Lifetime::Transient, "ForNT");
    }

    void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
    {
        OPEN_CONSOLE

        m_window = winrt::Microsoft::UI::Xaml::Window{};
        m_rootPage = winrt::WinManageUI::RootPage{};

        m_window.Content(m_rootPage);
        
        auto context = m_container.Resolve<winrt::WinMgmt::WmiDataContext>();
        auto context_2 = m_container.Resolve<winrt::WinMgmt::WmiDataContext>("test");

        auto appWindow{ m_window.AppWindow() };

        m_window.ExtendsContentIntoTitleBar(true);
        m_window.Activated([&self = *this](winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::WindowActivatedEventArgs const&)
        {
            Win32Helper::DisableMultiInstanceWindow(sender.try_as<Microsoft::UI::Xaml::Window>(), self.m_appName);
        });

        LOG_INFO("Test")
  
        m_window.Activate();
    }
}
