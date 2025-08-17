#include "pch.h"
#include "App.xaml.h"

#include "CppUnitTest.h"
#include <winrt/Microsoft.VisualStudio.TestPlatform.TestExecutor.WinRTCore.h>

using namespace winrt::Microsoft::UI::Xaml;


namespace winrt::UnitTests::implementation
{
    App::App(PWSTR argv)
    {
        m_args = argv;

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
        winrt::Microsoft::VisualStudio::TestPlatform::TestExecutor::WinRTCore::UnitTestClient::CreateDefaultUI();
        winrt::Microsoft::VisualStudio::TestPlatform::TestExecutor::WinRTCore::UnitTestClient::Run(m_args);
    }
}


#ifdef DISABLE_XAML_GENERATED_MAIN

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR argv, int)
{
    winrt::init_apartment(winrt::apartment_type::single_threaded);

    ::winrt::Microsoft::UI::Xaml::Application::Start(
        [argv](auto&&)
        {
            ::winrt::make<::winrt::UnitTests::implementation::App>(argv);
        });

    return 0;
}

#endif