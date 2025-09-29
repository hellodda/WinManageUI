#pragma once

#include "MainWindow.g.h"

namespace winrt::WinManageUI::implementation
{
    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
    };
}

namespace winrt::WinManageUI::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
